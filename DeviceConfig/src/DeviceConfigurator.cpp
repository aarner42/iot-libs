#include "DeviceConfigurator.h"

void validateConfig(const char *name, String value, const uint8 expected) {
    Serial.print(name);
    Serial.print("=");
    Serial.print(value);

    if (value.length() != expected) {
        Serial.print(" - is");
        Serial.print(value.length());
        Serial.print(" bytes. Should be");
        Serial.print(expected);
        Serial.println(" bytes.");
        Serial.println("Erasing bad config file ...");
        SPIFFS.remove("/sinric-config.txt");
        Serial.println("...And reset.");
        ESP.reset();
    } else {
        Serial.println(" - is OK");
    }


}

String readConfigValueFromFile(const char *name) {
    char lineBuffer[128];
    File configFile = SPIFFS.open("/sinric-config.txt", "r");
    String  returnVal = "";
    //read the apiKey
    while (configFile.available()) {
        int l = configFile.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer));
        lineBuffer[l] = 0;
        String  line = lineBuffer;
        if (line.indexOf(name) > -1) {
            returnVal = line.substring(line.lastIndexOf('=')+1);
            break;
        }
    }
    configFile.close();
    return returnVal;

}

void initWebPortalForConfigCapture() {
    AsyncWebServer server(80);
    Serial.println("config file not present - starting web server for configuration purposes.");
    server.reset();
    Serial.println("Server reset complete...");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "You'll want to submit /config?apiKey=[yourKey]&deviceID=[yourDeviceId]");
    });
    Serial.println("Setup / handler");

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("apiKey") && request->hasParam("deviceID")) {
            String apiKey = request->getParam("apiKey")->value();
            String deviceID = request->getParam("deviceID")->value();

            File configFile = SPIFFS.open("/sinric-config.txt", "w");
            if (!configFile) {
                Serial.println("File won't open - Can't complete...");
                request->send(500, "text/plain", "The flash FS is fucked...Try again.");
            } else {
                int written = configFile.print("apiKey=");
                written += configFile.println(apiKey);
                written += configFile.print("deviceID=");
                written += configFile.println(deviceID);
                configFile.flush();
                configFile.close();
                if (written < 50) {
                    Serial.println("Didn't write enough data.  Can't complete");
                } else {
                    Serial.println("Wrote config values to flash");
                    Serial.print("apiKey=");
                    Serial.println(apiKey);
                    Serial.print("deviceID=");
                    Serial.println(deviceID);
                    Serial.println("Resetting...");
                    ESP.reset();
                }
            }
        } else {
            request->send(400, "text/plain", "You fucked that up.  Try again, but this time with the required params.");
        }
    });
    Serial.println("Setup /config handler");

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "That'll be a 404.  Try again.");
    });
    Serial.println("Setup 404 handler");

    server.begin();
    Serial.println("Server initialized");
    while (true) {
        yield();
    }
}
