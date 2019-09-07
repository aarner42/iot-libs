#ifndef SinricSwitch_H
#define SinricSwitch_H

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

 
#include <Arduino.h>
#include <WebSocketsClient.h> 
#include <ArduinoJson.h> 

#include <ESP8266WebServer.h>
#include <StreamString.h>

#define ON_STATE  "ON"
#define OFF_STATE "OFF"

typedef void (* vCallBack)();
 
class SinricSwitch {
private:
        ESP8266WebServer *server = nullptr;
        String deviceID;
        bool powerState;
        bool connectedToSinric;
        vCallBack onCallback;
        vCallBack offCallback;
        vCallBack alertCallback;
        vCallBack resetCallback;
        vCallBack rebootCallback;
        WebSocketsClient webSocket;
        uint64_t heartbeatTimestamp;
        uint64_t pingTimeStamp;
        void startWebServer(unsigned int localPort);
        void startSinricClient(const String& apiKey);
        void handleRoot();
        void handleReset();
        void sinricOn(const String& id);
        void sinricOff(const String& id);
        void sinricLoop();
        void webLoop();
        void setPowerStateOnServer(const char *value);
public:
        SinricSwitch();
        SinricSwitch(String api_key,  String deviceID, unsigned int port, vCallBack on, vCallBack off, vCallBack alert, vCallBack reboot, vCallBack reset);
        ~SinricSwitch();
        void loop();
        void setPowerState(bool);
        void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};
 
#endif
