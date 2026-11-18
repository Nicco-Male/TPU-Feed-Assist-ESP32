#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include "config.h"

// Provisional bridge. NO actual motor or physical filament feedback.
// Button sensing is a separate future hardware stage, not implemented.
static constexpr char VERSION[] = "0.1.0-prototype";
static constexpr uint32_t MQTT_RETRY_MS = 5000;
static constexpr uint32_t WIFI_RETRY_MS = 12000;
static constexpr long MQTT_MAX_CLOCK_SKEW_S = 10;
static constexpr size_t RECENT_IDS = 12;
static constexpr uint32_t LOCAL_DEBOUNCE_MS = 40;
// Inputs are ONLY from a separately validated isolated open-collector front end.
// NEVER attach a Bambu PCB switch line directly to these GPIOs.
struct LocalInput { const char* name; uint8_t gpio; bool lastRaw; bool stable; uint32_t changedAt; };
LocalInput localInputs[] = {
  {"feed", GPIO_LOCAL_FEED, false, false, 0},
  {"unload", GPIO_LOCAL_UNLOAD, false, false, 0},
  {"force", GPIO_LOCAL_FORCE, false, false, 0}
};
static_assert(PULSE_MS >= 50 && PULSE_MS <= 300, "PULSE_MS must be 50-300ms");

WiFiClient networkClient;
WiFiClientSecure secureClient;
PubSubClient mqtt;
WebServer http(HTTP_PORT);
String topicPrefix, topicAvailability, topicStatus, topicEvent, topicCommand;

struct Button { const char* name; uint8_t gpio; };
static const Button BUTTONS[] = {
  {"feed", GPIO_FEED}, {"unload", GPIO_UNLOAD}, {"force", GPIO_FORCE}
};
const Button* activeButton = nullptr;
String activeRequestId, activeSource;
esp_timer_handle_t pulseTimer = nullptr;
volatile bool pulseExpired = false;
uint32_t lastPulseCompletedAt = 0;
bool hasCompletedPulse = false;
uint32_t lastWifiAttempt = 0;
uint32_t lastMqttAttempt = 0;
String recentIds[RECENT_IDS];
size_t recentPosition = 0;
String lastResult = "boot";
String lastButton = "none";
String lastRequestId = "";

void pulseTimerCallback(void*) {
  // Timer task forcibly opens the relay even if network processing stalls.
  const Button* b = activeButton;
  if (b) gpio_set_level(static_cast<gpio_num_t>(b->gpio), 0);
  pulseExpired = true;
}

bool isUnloadArmed() {
  return ENABLE_UNLOAD_BUTTON && digitalRead(GPIO_UNLOAD_ARM) == LOW;
}

bool validRequestId(const String& v) {
  if (v.isEmpty() || v.length() > 48) return false;
  for (size_t i=0; i<v.length(); ++i) {
    const char c=v[i];
    if (!((c>='0' && c<='9') || (c>='A' && c<='Z') ||
          (c>='a' && c<='z') || c=='_' || c=='-')) return false;
  }
  return true;
}
bool hasSeenId(const String& id) {
  for (const String& s : recentIds) if (s == id) return true;
  return false;
}
const Button* findButton(const String& name) {
  for (const Button& b : BUTTONS) if (name == b.name) return &b;
  return nullptr;
}
bool clockReady() { return time(nullptr) > 1700000000; }
String toJson(const JsonDocument& doc) {
  String s; serializeJson(doc, s); return s;
}
String statusJson() {
  JsonDocument doc;
  doc["schema"] = 1;
  doc["device_id"] = DEVICE_ID;
  doc["version"] = VERSION;
  doc["online"] = true;
  doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["mqtt_connected"] = mqtt.connected();
  doc["ntp_ready"] = clockReady();
  doc["rssi_dbm"] = WiFi.status()==WL_CONNECTED ? WiFi.RSSI() : 0;
  doc["uptime_ms"] = millis();
  doc["pulse_active"] = activeButton != nullptr;
  doc["active_button"] = activeButton ? activeButton->name : "none";
  doc["unload_armed"] = isUnloadArmed();
  doc["automatic_unload_supported"] = false;
  doc["multimaterial_switching_supported"] = false;
  doc["local_button_sensing_available"] = false; // unverified physical front end
  doc["local_button_monitoring_enabled"] = ENABLE_LOCAL_BUTTON_MONITORING;
  doc["motor_feedback_available"] = false;
  doc["last_button"] = lastButton;
  doc["last_request_id"] = lastRequestId;
  doc["last_result"] = lastResult;
  return toJson(doc);
}
void publishStatus() {
  if (mqtt.connected()) mqtt.publish(topicStatus.c_str(), statusJson().c_str(), true);
}
void publishEvent(const String& id, const String& button, const char* result,
                  const char* source) {
  if (!mqtt.connected()) return;
  JsonDocument doc;
  doc["schema"] = 1;
  doc["device_id"] = DEVICE_ID;
  doc["request_id"] = id;
  doc["button"] = button;
  doc["result"] = result;
  doc["source"] = source;
  doc["ts"] = clockReady() ? time(nullptr) : 0;
  mqtt.publish(topicEvent.c_str(), toJson(doc).c_str(), false);
}
// Observe debounced local button states; this code NEVER actuates the feeder.
// A remote PhotoMOS closure looks like a button press to an across-key sensor;
// suppress sensing during and briefly after any remote pulse.
void monitorLocalButtons() {
  if (!ENABLE_LOCAL_BUTTON_MONITORING) return;
  const uint32_t now = millis();
  if (activeButton || (hasCompletedPulse && uint32_t(now-lastPulseCompletedAt) < 350)) {
    // Keep input history aligned while suppressing these electrically ambiguous edges.
    for (LocalInput& key : localInputs) {
      key.lastRaw = digitalRead(key.gpio) == LOW;
      key.stable = key.lastRaw;
      key.changedAt = now;
    }
    return;
  }
  for (LocalInput& key : localInputs) {
    const bool raw = digitalRead(key.gpio) == LOW;
    if (raw != key.lastRaw) { key.lastRaw = raw; key.changedAt = now; }
    if (raw != key.stable && uint32_t(now-key.changedAt) >= LOCAL_DEBOUNCE_MS) {
      key.stable = raw;
      publishEvent("", key.name, raw ? "local_press_observed_unverified" :
                         "local_release_observed_unverified", "local_gpio");
      publishStatus();
    }
  }
}

const char* startPulse(const String& buttonName, const String& id,
                       const char* source) {
  const Button* button = findButton(buttonName);
  if (!button) return "unknown_button";
  if (!validRequestId(id)) return "invalid_request_id";
  if (hasSeenId(id)) return "duplicate_request_id";
  if (activeButton) return "busy";
  if (hasCompletedPulse &&
      uint32_t(millis()-lastPulseCompletedAt) < COMMAND_COOLDOWN_MS) return "cooldown";
  if (buttonName == "unload" && !isUnloadArmed()) return "unload_not_armed";
  // LOW was established before pinMode in setup.
  activeButton = button;
  pulseExpired = false;
  digitalWrite(button->gpio, HIGH);
  if (esp_timer_start_once(pulseTimer, uint64_t(PULSE_MS)*1000ULL) != ESP_OK) {
    digitalWrite(button->gpio, LOW);
    activeButton = nullptr;
    return "timer_failure";
  }
  activeRequestId = id;
  activeSource = source;
  recentIds[recentPosition] = id;
  recentPosition = (recentPosition+1)%RECENT_IDS;
  lastButton = buttonName;
  lastRequestId = id;
  lastResult = "pulse_started";
  publishEvent(id, buttonName, "pulse_started", source);
  publishStatus();
  return nullptr;
}
void updatePulse() {
  if (!activeButton || !pulseExpired) return;
  pulseExpired = false;
  const String button = activeButton->name;
  activeButton = nullptr;
  lastPulseCompletedAt = millis();
  hasCompletedPulse = true;
  lastResult = "pulse_complete_unverified";
  publishEvent(activeRequestId, button, "pulse_complete_unverified",
               activeSource.c_str());
  publishStatus();
  activeRequestId = "";
  activeSource = "";
}
bool authorized() {
  if (http.authenticate(HTTP_USER, HTTP_PASSWORD)) return true;
  const String supplied=http.header("X-API-Key");
  return supplied.length()>=16 && supplied==HTTP_API_KEY;
}
bool requireAuth() {
  if (authorized()) return true;
  http.requestAuthentication(BASIC_AUTH, "TPU Feed Assist", "Authentication required");
  return false;
}
void respondJson(int code, const String& s) {
  http.sendHeader("Cache-Control", "no-store");
  http.send(code, "application/json", s);
}
void respondError(int code, const char* reason) {
  JsonDocument d; d["ok"]=false; d["error"]=reason; respondJson(code,toJson(d));
}

static const char WEB_PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>TPU Feed Assist ESP32</title><style>
body{font:16px system-ui,sans-serif;max-width:560px;margin:2rem auto;padding:0 1rem;
background:#141820;color:#eee}
button{font:inherit;padding:1rem;margin:.3rem .4rem .3rem 0;border:0;border-radius:.6rem;cursor:pointer}
button:disabled{opacity:.4;cursor:not-allowed}
pre{white-space:pre-wrap;background:#252c38;padding:1rem;border-radius:.6rem}
small{color:#bbb}
</style><h1>TPU Feed Assist — prototype</h1>
<p>Isolated button pulses. NOT an AMS. Physical local button sensing is NOT implemented.</p>
<button onclick="press('feed')">FEED</button>
<button onclick="press('force')">FORCE</button>
<button id="unload" onclick="press('unload')">UNLOAD</button>
<p><small>UNLOAD is blocked until enabled in firmware AND physically armed;
manual PTFE/toolhead procedure remains necessary.</small></p>
<pre id="status">Loading...</pre><p id="message"></p>
<script>
async function refresh(){
 try{
  const r=await fetch('/api/v1/status',{cache:'no-store'});
  const j=await r.json();
  document.querySelector('#status').textContent=JSON.stringify(j,null,2);
  document.querySelector('#unload').disabled=!j.unload_armed||j.pulse_active;
 }catch(e){document.querySelector('#status').textContent=String(e)}
}
async function press(button){
 if(button==='unload'&&!confirm('Is the printer IDLE and the PTFE/filament path safe?'))return;
 const request_id='web-'+Date.now()+'-'+Math.floor(Math.random()*1e6);
 const r=await fetch('/api/v1/press',{method:'POST',
    headers:{'Content-Type':'application/json'},
    body:JSON.stringify({button,request_id})});
 const j=await r.json();document.querySelector('#message').textContent=JSON.stringify(j);refresh();
}
setInterval(refresh,3000);refresh();
</script></html>)HTML";

void configureHttp() {
  const char* headers[]={"X-API-Key"};
  http.collectHeaders(headers,1);
  http.on("/",HTTP_GET,[](){
    if(!requireAuth())return;
    http.sendHeader("Cache-Control","no-store");
    http.send_P(200,"text/html",WEB_PAGE);
  });
  http.on("/api/v1/status",HTTP_GET,[](){
    if(!requireAuth())return;
    respondJson(200,statusJson());
  });
  http.on("/api/v1/press",HTTP_POST,[](){
    if(!requireAuth())return;
    if(!http.hasArg("plain")||http.arg("plain").length()>512){
      respondError(400,"invalid_json_body");return;
    }
    JsonDocument req;
    if(deserializeJson(req,http.arg("plain"))){
      respondError(400,"invalid_json");return;
    }
    const String button=req["button"].as<String>();
    const String id=req["request_id"].as<String>();
    const char* err=startPulse(button,id,"http");
    if(err) {
      const String s=err;
      respondError((s=="busy"||s=="cooldown"||s=="unload_not_armed"||
                    s=="duplicate_request_id")?409:400,err);
      return;
    }
    JsonDocument reply;
    reply["ok"]=true;
    reply["request_id"]=id;
    reply["result"]="pulse_started";
    reply["note"]="No feedback from Bambu motor, physical button or filament";
    respondJson(202,toJson(reply));
  });
  http.onNotFound([](){if(requireAuth())respondError(404,"not_found");});
  http.begin();
}
void onMqttMessage(char* topic,byte* payload,unsigned int length) {
  if(topicCommand!=String(topic)||length==0||length>512)return;
  JsonDocument req;
  if(deserializeJson(req,payload,length))return;
  const String button=req["button"].as<String>();
  const String id=req["request_id"].as<String>();
  const long ts=req["ts"].as<long>();
  if(!validRequestId(id))return;
  if(!clockReady() || labs(long(time(nullptr))-ts)>MQTT_MAX_CLOCK_SKEW_S){
    publishEvent(id,button,"rejected_stale_or_clock_unsynced","mqtt");return;
  }
  const char* err=startPulse(button,id,"mqtt");
  if(err) publishEvent(id,button,err,"mqtt");
}
void connectMqtt() {
  if(WiFi.status()!=WL_CONNECTED||mqtt.connected())return;
  if(uint32_t(millis()-lastMqttAttempt)<MQTT_RETRY_MS)return;
  lastMqttAttempt=millis();
  const String clientId=String("tpu-feed-")+DEVICE_ID;
  if(!mqtt.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD,
                   topicAvailability.c_str(),1,true,"offline"))return;
  mqtt.publish(topicAvailability.c_str(),"online",true);
  mqtt.subscribe(topicCommand.c_str(),1);
  publishStatus();
}
void setup() {
  Serial.begin(115200);
  for(const Button& b:BUTTONS){
    digitalWrite(b.gpio,LOW);
    pinMode(b.gpio,OUTPUT);
  }
  pinMode(GPIO_UNLOAD_ARM,INPUT_PULLUP);
  if (ENABLE_LOCAL_BUTTON_MONITORING) {
    for (LocalInput& key : localInputs) {
      pinMode(key.gpio, INPUT_PULLUP); // for isolated open-collector output ONLY
      key.lastRaw = digitalRead(key.gpio) == LOW;
      key.stable = key.lastRaw;
      key.changedAt = millis();
    }
  }
  const esp_timer_create_args_t timerArgs={
    .callback=&pulseTimerCallback,
    .arg=nullptr,
    .dispatch_method=ESP_TIMER_TASK,
    .name="feeder_pulse"
  };
  if(esp_timer_create(&timerArgs,&pulseTimer)!=ESP_OK){
    Serial.println("FATAL: Cannot initialize contact release timer");
    while(true){delay(1000);}
  }
  topicPrefix=String("tpu-feed-assist/")+DEVICE_ID;
  topicAvailability=topicPrefix+"/availability";
  topicStatus=topicPrefix+"/status";
  topicEvent=topicPrefix+"/event";
  topicCommand=topicPrefix+"/cmd/press";
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  lastWifiAttempt=millis();
  configTime(0,0,NTP_SERVER);
  if(MQTT_TLS){
    if(strlen(MQTT_CA_CERT)==0){
      Serial.println("ERROR: TLS requires MQTT_CA_CERT; MQTT disabled");
    }else{
      secureClient.setCACert(MQTT_CA_CERT);
      mqtt.setClient(secureClient);
    }
  }else{
    mqtt.setClient(networkClient);
  }
  mqtt.setServer(MQTT_HOST,MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
  mqtt.setBufferSize(1024);
  configureHttp();
  Serial.printf("TPU Feed Assist %s / %s - button pulses ONLY\n",VERSION,DEVICE_ID);
}
void loop() {
  updatePulse();
  monitorLocalButtons();
  http.handleClient();
  updatePulse();
  if(WiFi.status()!=WL_CONNECTED &&
     uint32_t(millis()-lastWifiAttempt)>=WIFI_RETRY_MS){
    lastWifiAttempt=millis();
    WiFi.reconnect();
  }
  if(!MQTT_TLS||strlen(MQTT_CA_CERT)!=0){
    connectMqtt();
    if(mqtt.connected())mqtt.loop();
  }
  updatePulse();
  delay(2);
}
