#ifndef DeviceConfigurator_H
#define DeviceConfigurator_H

#define SINRIC_DEVID_PARAM      "deviceID"
#define SINRIC_APIKEY_PARAM     "apiKey"
#define LED_PIN_PARAM           "ledPin"
#define INPUT_PIN_PARAM         "inputPin"
#define TRIGGER_PIN_PARAM       "triggerPin"
#define TRIGGER_LEVEL_PARAM     "hiVoltOnLevel"

#include <Arduino.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>

struct IOTConfig {
    String apiKey;          //sinric APIKey
    String deviceID;        //sinric deviceID
    uint8_t ledPin;         //GPIO pin used for switch illumination
    uint8_t triggerPin;     //GPIO pin used to trigger hi-voltage side
    uint8_t inputPin;       //GPIO pin used for manual input (assumes pull-up)
    boolean onLevel;        //trigger level to turn hi-voltage side ON (LOW for relays, HIGH for triacs)
    boolean offLevel;       //trigger level to turn hi-voltage side OFF (HIGH for relays, LOW for triacs)
};

class DeviceConfigurator {
private:
    static uint8_t getPinFromString(const String& rep);
    static boolean getLevelFromString(const String& rep);
    const char* configFileName;
    void initWebPortalForConfigCapture();
    void validateConfig(const char *name, const String& value, uint8 min, uint8 max);
    String readConfigValueFromFile(const char *name);

public:
    DeviceConfigurator();
    DeviceConfigurator(const char *configFileName);
    ~DeviceConfigurator();
    IOTConfig getConfig();
};


#endif
