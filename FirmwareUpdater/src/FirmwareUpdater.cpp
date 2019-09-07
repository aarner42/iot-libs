#include "FirmwareUpdater.h"

FirmwareUpdater::FirmwareUpdater() {
    Serial.println("default constructor called");
}

FirmwareUpdater::FirmwareUpdater(const char* sketchName, const char* currentVersion, int duration) {
    Serial.printf("Usable FirmwareUpdater Constructor called with duration %d\n", duration);
    this->duration = duration;
    this->sketchName = sketchName;
    this->currentVersion = currentVersion;
    this->updateCheckDue = true;
    versionURL = "";
    versionURL.concat(SKETCH_OTA_UPDATER_HOST);
    versionURL.concat(sketchName);
    versionURL.concat(SKETCH_FW_VERSION_URI);

    internalTicker.attach_ms(duration, std::bind(&FirmwareUpdater::setUpdateDue, this));
    Serial.println("FirmwareUpdater Constructor finishing - timers registered");
}

FirmwareUpdater::~FirmwareUpdater() {
    //nothing to destroy but objects
}

void FirmwareUpdater::setUpdateDue() {
    Serial.println("Ticker fired to set updateDue method in fwFetcher");
    this->updateCheckDue = true;
}

bool FirmwareUpdater::isUpdateCheckDue() {
    if (this->updateCheckDue) {
        this->updateCheckDue = false;
        return true;
    }
    return false;
}

String FirmwareUpdater::getUpdateUrl() {
    return versionURL;
}

bool FirmwareUpdater::isServerVersionNewer(String &serverVersion)  {
    Serial.print("Latest version of sketch FW is: ");
    Serial.println(serverVersion);//make sure we haven't gotten some rubbish back from the server
    Serial.println("Comparing server <--> local versions of sketch:");
    Serial.print(serverVersion);
    Serial.print(" <--> ");
    Serial.print(currentVersion);

    if (serverVersion.length() == strlen(currentVersion)) {
        //this is probably a sensible version string - at least the formats match
        if (serverVersion.equalsIgnoreCase(currentVersion)) {
            Serial.println(" Versions match - done");
            return false;
        } else {
            Serial.println(" No match - time for an OTA update!");
            return true;
        }
    } else {
        Serial.print("Unable to make sense of server version:");
        Serial.println(serverVersion);
        return false;
    }
}

void FirmwareUpdater::updateFirmwareFromServer(String &serverVersion)  {//ota update
    String otaUpdaterPath = SKETCH_OTA_UPDATER_HOST "/";
    otaUpdaterPath.concat(sketchName);
    otaUpdaterPath.concat("/");
    otaUpdaterPath.concat(serverVersion);
    otaUpdaterPath.concat(".bin");
    WiFiClient client;
    Serial.println("**************************  OTA UPDATE *****************************");
    Serial.print("[");
    Serial.print(otaUpdaterPath);
    Serial.println("]");
    Serial.println("********************************************************************");
    ESPhttpUpdate.rebootOnUpdate(false);
    ESPhttpUpdate.update(client, otaUpdaterPath, "");
    //code below never runs if update succeeds.
    Serial.print("Any OTA Errors:");
    Serial.println(ESPhttpUpdate.getLastErrorString());
    Serial.println("Blank if OK - delaying before restart");
    delay(2500);
    Serial.println("Rebooting....");
    ESP.restart();
}

void FirmwareUpdater::checkServerForUpdate()  {
    Serial.println("\n\nServer Update check Function....");
    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] operations beginning...");
    Serial.print("Connecting to ");
    Serial.println(getUpdateUrl());

    if (http.begin(client, getUpdateUrl())) {  // HTTP
        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                String serverVersion = http.getString();
                if (isServerVersionNewer(serverVersion)) {
                    updateFirmwareFromServer(serverVersion);
                }
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", HTTPClient::errorToString(httpCode).c_str());
            }

        } else {
            Serial.printf("[HTTP} Unable to connect - code %d\n", httpCode);
        }
    }
    http.end();
}

