#include "DeviceConfigurator.h"

void DeviceConfigurator::validateConfig(const char *name, const String &value, const uint8 min, const uint8 max) {
    Serial.print(name);
    Serial.print("=");
    Serial.print(value);

    if (value.length() > max || value.length() < min) {
        Serial.printf("%s - is %d bytes - Should be >= %d and <= %d bytes.\n", value.c_str(), value.length(), min, max);
        Serial.printf("Erasing bad config  (%s) file ...\n", configFileName);
        SPIFFS.remove(configFileName);
        Serial.println("...And reset.");
        ESP.reset();
    } else {
        Serial.println(" - is OK");
    }


}

String DeviceConfigurator::readConfigValueFromFile(const char *name) {
    char lineBuffer[128];
    File configFile = SPIFFS.open(configFileName, "r");
    String returnVal = "";
    //read the apiKey
    while (configFile.available()) {
        int l = configFile.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer));
        lineBuffer[l] = 0;
        String line = lineBuffer;
        if (line.indexOf(name) > -1) {
            returnVal = line.substring(line.lastIndexOf('=') + 1);
            break;
        }
    }
    configFile.close();
    return returnVal;

}

DeviceConfigurator::DeviceConfigurator() {
    Serial.println("Somebody called the no-arg constructor - this is bad.");
    configFileName = "BAD_INIT";
}

DeviceConfigurator::DeviceConfigurator(const char *configFileName) {
    this->configFileName = configFileName;
}

DeviceConfigurator::~DeviceConfigurator() {
    //nothing to destroy;
}

IOTConfig DeviceConfigurator::getConfig() {
    Serial.println("Starting FS");
    SPIFFS.begin();

    Serial.printf("Checking for config file %s\n", configFileName);
    if (!SPIFFS.exists(configFileName)) {
        initWebPortalForConfigCapture();
    } else {
        String apiKey = readConfigValueFromFile(SINRIC_APIKEY_PARAM);
        String deviceID = readConfigValueFromFile(SINRIC_DEVID_PARAM);
        String ledPin = readConfigValueFromFile(LED_PIN_PARAM);
        String inputPin = readConfigValueFromFile(INPUT_PIN_PARAM);
        String triggerPin = readConfigValueFromFile(TRIGGER_PIN_PARAM);
        String triggerLevel = readConfigValueFromFile(TRIGGER_LEVEL_PARAM);

        validateConfig(SINRIC_APIKEY_PARAM, apiKey, 37, 37);
        validateConfig(SINRIC_DEVID_PARAM, deviceID, 25, 25);
        validateConfig(LED_PIN_PARAM, ledPin, 3, 3);
        validateConfig(INPUT_PIN_PARAM, inputPin, 3, 3);
        validateConfig(TRIGGER_PIN_PARAM, triggerPin, 3, 3);
        validateConfig(TRIGGER_LEVEL_PARAM, triggerLevel, 4, 5);

        struct IOTConfig config = IOTConfig();
        config.apiKey = apiKey;
        config.deviceID = deviceID;
        config.ledPin = getPinFromString(ledPin);
        config.inputPin = getPinFromString(inputPin);
        config.triggerPin = getPinFromString(triggerPin);
        config.onLevel = getLevelFromString(triggerLevel);
        config.offLevel = !getLevelFromString(triggerLevel);

        SPIFFS.end();
        return config;
    }
    //this is unreachable code - initPortalForCapture() above ends in infinite loop;
    struct IOTConfig cfg = IOTConfig();
    return cfg;
}

    uint8_t DeviceConfigurator::getPinFromString(const String &pinRep) {
        Serial.printf("Converting string representation (%s) into pin value\n", pinRep.c_str());
        if (pinRep.startsWith("D0"))
            return D0;
        if (pinRep.startsWith("D1"))
            return D1;
        if (pinRep.startsWith("D2"))
            return D2;
        if (pinRep.startsWith("D3"))
            return D3;
        if (pinRep.startsWith("D4"))
            return D4;
        if (pinRep.startsWith("D5"))
            return D5;
        if (pinRep.startsWith("D6"))
            return D6;
        if (pinRep.startsWith("D7"))
            return D7;
        if (pinRep.startsWith("D8"))
            return D8;

        Serial.println("No Match - returning LED_BUILTIN");
        return LED_BUILTIN;
    }

    boolean DeviceConfigurator::getLevelFromString(const String &lvlRep) {
        if (lvlRep.startsWith("HIGH"))
            return HIGH;
        else
            return LOW;
    }



#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    void DeviceConfigurator::initWebPortalForConfigCapture() {
        AsyncWebServer server(80);
        Serial.println("config file not present - starting web server for configuration purposes.");
        server.reset();
        Serial.println("Server reset complete...");
        String fileName = this->configFileName;
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain",
                          "You'll want to submit /config?"
                          SINRIC_APIKEY_PARAM "=[yourKey]&"
                          SINRIC_DEVID_PARAM"=[yourDeviceId]&"
                          LED_PIN_PARAM "=[switchIlluminationPin]&"
                          INPUT_PIN_PARAM "=[manualInputPin]&"
                          TRIGGER_PIN_PARAM "=[hiVoltTriggerPin]&"
                          TRIGGER_LEVEL_PARAM "=[LOW-RELAY or HIGH-TRIAC]");
        });
        Serial.println("Setup / handler");

        server.on("/config", HTTP_GET, [fileName](AsyncWebServerRequest *request) {
            if (request->hasParam(SINRIC_APIKEY_PARAM) &&
                request->hasParam(SINRIC_DEVID_PARAM) &&
                request->hasParam(LED_PIN_PARAM) &&
                request->hasParam(INPUT_PIN_PARAM) &&
                request->hasParam(TRIGGER_PIN_PARAM) &&
                request->hasParam(TRIGGER_LEVEL_PARAM)) {

                String apiKey = request->getParam(SINRIC_APIKEY_PARAM)->value();
                String deviceID = request->getParam(SINRIC_DEVID_PARAM)->value();
                String ledPin = request->getParam(LED_PIN_PARAM)->value();
                String inputPin = request->getParam(INPUT_PIN_PARAM)->value();
                String triggerPin = request->getParam(TRIGGER_PIN_PARAM)->value();
                String triggerLevel = request->getParam(TRIGGER_LEVEL_PARAM)->value();
                File configFile = SPIFFS.open(fileName, "w");
                if (!configFile) {
                    Serial.println("File won't open - Can't complete...");
                    request->send(500, "text/plain", "The flash FS is bollocks...Try again.");
                } else {
                    int written = configFile.print(SINRIC_APIKEY_PARAM "=");
                    written += configFile.println(apiKey);
                    written += configFile.print(SINRIC_DEVID_PARAM "=");
                    written += configFile.println(deviceID);
                    written += configFile.print(LED_PIN_PARAM "=");
                    written += configFile.println(ledPin);
                    written += configFile.print(INPUT_PIN_PARAM "=");
                    written += configFile.println(inputPin);
                    written += configFile.print(TRIGGER_PIN_PARAM "=");
                    written += configFile.println(triggerPin);
                    written += configFile.print(TRIGGER_LEVEL_PARAM "=");
                    written += configFile.println(triggerLevel);
                    configFile.flush();
                    configFile.close();
                    if (written < 75) {
                        Serial.print("Didn't write enough data to flash - only ");
                        Serial.print(written);
                        Serial.println(" bytes - can't continue");
                    } else {
                        Serial.println("Wrote config values to flash");
                        Serial.printf("apiKey=%s\ndeviceId=%s\nledPin=%s\ninputPin=%s\ntriggerPin=%s\nhiVoltOnLevel=%s\n",
                                apiKey.c_str(), deviceID.c_str(), ledPin.c_str(), inputPin.c_str(), triggerPin.c_str(), triggerLevel.c_str());
                        Serial.println("Resetting...");
                        ESP.reset();
                    }
                }
            } else {
                request->send(400, "text/plain",
                              "You f***ed that up.  Try again, but this time with the required params.");
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
#pragma clang diagnostic pop

