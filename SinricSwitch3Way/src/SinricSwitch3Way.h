#ifndef SinricSwitch3Way_H
#define SinricSwitch3Way_H

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

 
#include <Arduino.h>
#include <WebSocketsClient.h> 
#include <ArduinoJson.h> 

#include <ESP8266WebServer.h>
#include <StreamString.h>

typedef void (* vCallBack)();

 
class SinricSwitch3Way {
private:
        ESP8266WebServer *server = NULL;
        const char* deviceID;
        bool powerState;
        bool connectedToSinric;
        vCallBack toggleCallback;
        vCallBack alertCallback;
        vCallBack resetCallback;
        WebSocketsClient webSocket;
        uint64_t heartbeatTimestamp;
        uint64_t pingTimeStamp;
        void startWebServer(unsigned int localPort);
        void startSinricClient(const char* apiKey);
        void handleRoot();
        void handleReset();
        void handleToggle();
        void sinricOn(String id);
        void sinricOff(String id);
        void sinricLoop();
        void webLoop();
        void setPowerStateOnServer(String value);
public:
        SinricSwitch3Way();
        SinricSwitch3Way(const char* api_key, const char* device_id, unsigned int port, vCallBack toggleCB, vCallBack alertCB, vCallBack resetCallback);
        ~SinricSwitch3Way();
        void loop();
        void setPowerState(bool newState);
        boolean getPowerState();
        void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};
 
#endif
