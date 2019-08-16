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
        String deviceID;
        bool powerState;
        bool connectedToSinric;
        vCallBack toggleCallback;
        vCallBack alertCallback;
        vCallBack resetCallback;
        vCallBack rebootCallback;
        WebSocketsClient webSocket;
        uint64_t heartbeatTimestamp;
        uint64_t pingTimeStamp;
        void startWebServer(unsigned int localPort);
        void startSinricClient(String apiKey);
        void handleRoot();
        void handleReset();
        void handleToggle();
        void sinricOn(String id);
        void sinricOff(String id);
        void sinricLoop();
        void webLoop();
        void setPowerStateOnServer(const char *value);
public:
        SinricSwitch3Way();
        SinricSwitch3Way(String api_key, String device_id, unsigned int port, vCallBack toggleCB, vCallBack alertCB, vCallBack reboot, vCallBack reset);
        ~SinricSwitch3Way();
        void loop();
        void setPowerState(bool);
        boolean getPowerState();
        void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
};
 
#endif
