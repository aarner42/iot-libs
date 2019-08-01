#ifndef SinricSwitch_H
#define SinricSwitch_H

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

 
#include <Arduino.h>
#include <WebSocketsClient.h> 
#include <ArduinoJson.h> 

#include <ESP8266WebServer.h>
#include <StreamString.h>


typedef void (* vCallBack)();
 
class SinricSwitch {
private:
        ESP8266WebServer *server = NULL;
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
        void startSinricClient(const char* apiKey);
        void handleRoot();
        void handleReset();
        void sinricOn(String id);
        void sinricOff(String id);
        void sinricLoop();
        void webLoop();
        void setPowerStateOnServer(String value);
public:
        SinricSwitch();
        SinricSwitch(const char* api_key,  const String deviceID, unsigned int port, vCallBack on, vCallBack off, vCallBack alert, vCallBack reboot, vCallBack reset);
        ~SinricSwitch();
        void loop();
        void setPowerState(bool);
        boolean getPowerState();
        void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};
 
#endif
