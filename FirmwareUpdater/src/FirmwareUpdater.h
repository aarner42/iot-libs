//
// Created by aarne on 8/31/2019.
//

#ifndef FIRMWAREUPDATER_H
#define FIRMWAREUPDATER_H

#define SKETCH_OTA_UPDATER_HOST "http://192.168.1.176/"
#define SKETCH_FW_VERSION_URI   "/version"


#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266httpUpdate.h>
#include <Ticker.h>

class FirmwareUpdater {
private:
    Ticker internalTicker;
    const char* sketchName;
    const char* currentVersion;
    int duration;
    volatile bool updateCheckDue;
    String versionURL;
    void setUpdateDue();
    bool isServerVersionNewer(String &serverVersion);
    void updateFirmwareFromServer(String &serverVersion);
public:
    FirmwareUpdater();
    FirmwareUpdater(const char* sketchName, const char* currentVersion, int duration);
    ~FirmwareUpdater();
    bool isUpdateCheckDue();
    String getUpdateUrl();
    void checkServerForUpdate();
};

#endif //OTAUPDATEPOC_FIRMWAREUPDATER_H
