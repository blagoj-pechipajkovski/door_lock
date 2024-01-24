#include <Arduino.h>
class DoorRelay
{
  private:
    bool _unlocked_state;
    int _pin;
    
    unsigned long _unlock_time_ms = 2345;
    
    unsigned long _unlock_start = 0;
  public:
    
    void unlock()
    {
        _unlock_start = millis();
        digitalWrite(_pin, _unlocked_state);
    }
    
    void lock()
    {
        digitalWrite(_pin, !_unlocked_state);
    }
    
    void begin(int pin, bool unlocked_state=HIGH)
    {
        _pin = pin;
        pinMode(_pin, OUTPUT);
        _unlocked_state = unlocked_state;
        
        lock();
    }
    
    void set_unlock_time_ms(unsigned long new_ms)
    {
        _unlock_time_ms = min(new_ms, 20000ul); // limit max unlocked time
    }
    
    bool is_unlocked()
    {
        return digitalRead(_pin) == _unlocked_state;
    }
    
    void update()
    {
        if ((millis() > _unlock_start + _unlock_time_ms) && is_unlocked())
            lock();
    }
};

#define DOOR_PIN 22
DoorRelay vratata;


#include <DiagnosticLED.h>
DiagnosticLED status_led;
int pattern_idle[2]     = { 1, 1 };
int pattern_updating[2] = { 2, 1 };

#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <Update.h>

DNSServer dnsServer;
AsyncWebServer server(80);


#include "esp_wifi.h"



#include "html_files.h" // generates handle_index(AsyncWebServerRequest *request)
class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request){
        Serial.println(request->url());
        if (request->url() == "/")
            return true;
        if (request->url() == "/chat")
            return true;
        if (request->url().startsWith("/generate_204"))
            return true;
        if (request->url() == "/canonical.html")
            return true;
        if (request->url() == "/success.txt")
            return true;
        
        return false;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        handle_index(request);
    }
};


#include <Preferences.h>
Preferences preferences;

#include <WebVars.h>




const char* default_ap_name = "Vrata unlock";
const char* default_pass = "";
WebVarStr<50> AP_name = WebVarStr<50>("AP_name", default_ap_name,    PUBLIC, PROTECTED, USE_PREFERENCES);
WebVarStr<50> AP_pass = WebVarStr<50>("AP_pass", default_pass, PROTECTED, PROTECTED, USE_PREFERENCES);

WebVarStr<50> admin_pass = WebVarStr<50>("admin_pass", default_pass, PROTECTED, PROTECTED, USE_PREFERENCES);

WebVarInt unlock_time = WebVarInt("unlock_time", 5000, PUBLIC, PROTECTED, USE_PREFERENCES);


// Factory reset
#define RESET_PIN 16
#define RESET_HOLD_TIME 4444

bool do_restart = false;
void setup() {
    // Start the debugger
    Serial.begin(115200);
    
    // Preferences // used in WebVars
    preferences.begin("pref_vars");
    // WebVars
    WebVars::set_load_preferences(&preferences);
    

    // AP
    WiFi.softAP(AP_name, AP_pass);
    
    // Captive portal
    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
    
    WebVars::begin(&server);
    WebVars::set_password(admin_pass);
    
    // html_convert
    html_files(&server);
    
    // Vrata
    server.on("/unlock", HTTP_GET, [](AsyncWebServerRequest *request){
        vratata.unlock();
        request->redirect("/");
    });
    
    server.on("/exit", HTTP_GET, [](AsyncWebServerRequest *request){
        //for int i=0; i<WiFi.softAPgetStationNum(); i++) {}
        request->send(200, "text/plain", "Exiting");
        esp_wifi_deauth_sta(0);
    });
    
    // ESP
    server.onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/plain", "Not found");
    });
    
    // Update
    server.on("/handle_update", HTTP_POST,
        [](AsyncWebServerRequest *request) { },
        [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (WebVars::has_valid_pass(request)) {
                if (!index){
                    status_led.activePattern(pattern_updating);
                    Serial.println("Update");
                    if (!Update.begin()) {
                        Update.printError(Serial);
                    }
                }

                if (Update.write(data, len) != len) {
                    Update.printError(Serial);
                }
                else {
                  Serial.printf("Progress: %d%%\n", (Update.progress()*100)/Update.size());
                }

                if (final) {
                    if (!Update.end(true)) {
                        Update.printError(Serial);
                        request->send(401, "text/plain", "Update failed");
                    }
                    else {
                        request->send(401, "text/plain", "Update complete");
                        Serial.println("Update complete");
                        Serial.flush();
                        delay(123);
                        do_restart = true;
                    }
                }
            }
            else {
                if (final) // if we don't wait for final the ESP crashes
                    request->send(401, "text/plain", "Wrong password");
            }
        }
    );
    
    // Restart
    server.on("/restart", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            if (WebVars::has_valid_pass(request)) {
                do_restart = true;
                request->send(200, "text/plain", "Restarting");
            }
            else {
                request->send(401, "text/plain", "Wrong password");
            }
        }
    );
    
    server.begin();
    
    // Vrata
    vratata.begin(DOOR_PIN);
    vratata.set_unlock_time_ms(unlock_time);
    unlock_time.on_change( []() {vratata.set_unlock_time_ms(unlock_time);} );
    
    // Factory reset
    pinMode(RESET_PIN, INPUT_PULLUP);
    
    // LED
    status_led.begin(LED_BUILTIN, HIGH);
    status_led.activePattern(pattern_idle);
}

void loop() {

    do {
        if (digitalRead(RESET_PIN) == LOW) {
            
            unsigned long reset_start = millis();
            while ( (digitalRead(RESET_PIN) == LOW) && (millis() < reset_start + RESET_HOLD_TIME) ) { delay(1); }
            if (millis() < reset_start + RESET_HOLD_TIME) break;
            
            AP_name.str_to_val(default_ap_name);
            AP_pass.str_to_val(default_pass);
            admin_pass.str_to_val(default_pass);
            
            do_restart = true;
        }
    } while (false);
    
    // Captive portal
    dnsServer.processNextRequest();
    
    // Vrata
    vratata.update();
    
    // Status LED
    if (vratata.is_unlocked()) {
        // hijack LED_BUILTIN from status_led
        digitalWrite(LED_BUILTIN, millis() % 333 < 166);
    }
    else {
        status_led.update();
    }
    
    if (do_restart && !vratata.is_unlocked()) {
        delay(1000);
        ESP.restart();
    }
}
