#ifndef DeviceConfigurator_H
#define DeviceConfigurator_H

#define DEVICE_ID_PARAM   "deviceID"
#define SINRIC_KEY_PARAM  "apiKey"

#include <Arduino.h>
#include <FS.h>
#include <ESPAsyncWebServer.h>

void initWebPortalForConfigCapture();
void validateConfig(const char *name, String value, uint8 expected);
String readConfigValueFromFile(const char *name);

#endif
