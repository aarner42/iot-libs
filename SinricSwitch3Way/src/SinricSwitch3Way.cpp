#include "SinricSwitch3Way.h"


   
//<<constructor>> 
SinricSwitch3Way::SinricSwitch3Way(){
    Serial.println("default constructor called");
}

//useful constructor
SinricSwitch3Way::SinricSwitch3Way(String apiKey, String device_id, unsigned int port, vCallBack toggleCB, vCallBack alertCB, vCallBack rebootCB, vCallBack resetCB){
    deviceID = device_id;
    toggleCallback = toggleCB;
    alertCallback = alertCB;
    resetCallback = resetCB;
    powerState = false;
    startWebServer(port);
    startSinricClient(apiKey);
    heartbeatTimestamp = 0;
    pingTimeStamp = 0;
    Serial.print("Registered switch with deviceID=");
    Serial.println(deviceID);
}
 
//<<destructor>>
SinricSwitch3Way::~SinricSwitch3Way(){/*nothing to destruct*/}

void SinricSwitch3Way::sinricLoop() {
    webSocket.loop();
    
    if(connectedToSinric) {
        uint64_t now = millis();
        pingTimeStamp = now;
        // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
        if((now < 10000) || (now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
              heartbeatTimestamp = now;
              webSocket.sendTXT("H");  
              webSocket.sendPing();        
        }
    } else {
        if (pingTimeStamp == 0 || ((millis() - pingTimeStamp) > 60000)) {
            Serial.println("No connection to Sinric cloud - sending data to force webSocket to realize disconnect happened");
            webSocket.sendTXT("H");      
            pingTimeStamp = millis();

            //take action on connection failure...
            alertCallback();
        }
    }
}


void SinricSwitch3Way::webLoop(){
    if (server != NULL) {
        server->handleClient();
        delay(1);
    }
}

void SinricSwitch3Way::loop() {
    webLoop();
    sinricLoop();
}

void SinricSwitch3Way::startWebServer(unsigned int localPort){
  server = new ESP8266WebServer(localPort);

  server->on("/", [&]() {
    handleRoot();
  });
 
  
  server->on("/reset", [&]() {
    handleReset();
  });

  server->on("/toggle", [&]() {
    handleToggle();
  });

  //server->onNotFound(handleNotFound);
  server->begin();

  Serial.print("WebServer started on port: ");
  Serial.println(localPort);
}

void SinricSwitch3Way::startSinricClient(String apiKey) {
  webSocket.begin("iot.sinric.com", 80, "/");
 
  // event handler
//  webSocket.onEvent(&*this->webSocketEvent());
  webSocket.onEvent([&](WStype_t t, uint8_t * p, size_t l) {
    webSocketEvent(t, p, l);
  });
  webSocket.setAuthorization("apikey", apiKey.c_str());
  Serial.println("WebSocket handlers/authorization registered...");
  
  // try again every 60 s if connection has failed
  webSocket.setReconnectInterval(60000);
  Serial.println("Setup complete...");   
}

void SinricSwitch3Way::webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  Serial.print("WebSocketEvent:");
  Serial.println(type);
  
  switch(type) {
    case WStype_ERROR: {
      connectedToSinric=true;
      Serial.printf("[WSc] Webservice ERROR!\n");
      }
      break;
    case WStype_FRAGMENT_TEXT_START: {
      connectedToSinric=true;
      Serial.printf("[WSc] Webservice Text Fragment begin\n");
    }
    break;
    case WStype_FRAGMENT_BIN_START: {
      connectedToSinric=true;
      Serial.printf("[WSc] Webservice Binary Fragment begin\n");
    }
    break;
    case WStype_FRAGMENT_FIN: {
      connectedToSinric=true;
      Serial.printf("[WSc] Webservice Fragment finished\n");
    }
    break;
    case WStype_FRAGMENT: {
      connectedToSinric=true;
      Serial.printf("[WSc] Webservice Fragment\n");
    }
    break;
    case WStype_DISCONNECTED: {
      connectedToSinric = false;    
      Serial.printf("[WSc] Webservice disconnected from sinric.com!\n");
      }
      break;
    case WStype_CONNECTED: {
      connectedToSinric = true;
      Serial.printf("[WSc] Service connected to sinric.com at url: iot.sincric.com %s\n", payload);
      Serial.printf("Waiting for commands from sinric.com ...\n");        
      }
      break;
    case WStype_TEXT: {
      connectedToSinric = true;
        Serial.printf("[WSc] get text: %s\n", payload);
        // Example payloads

        // For Switch or Light device types
        // {"deviceId": xxxx, "action": "setPowerState", value: "ON"} // https://developer.amazon.com/docs/device-apis/alexa-powercontroller.html

        // For Light device type
        // Look at the light example in github
          
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload); 
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "setPowerState") { // Switch or Light
            String value = json ["value"];
            if(value == "ON") {
                sinricOn(deviceId);
            } else {
                sinricOff(deviceId);
            }
        }
        else if (action == "test") {
            Serial.println("[WSc] received test command from sinric.com");
        }
      }
      break;
    case WStype_BIN:
      connectedToSinric = true;
      Serial.printf("[WSc] get binary length: %u\n", length);
      break;
  }
}

void SinricSwitch3Way::sinricOn(String id) {
  if (id == deviceID) // Device ID of first device
  {  
    Serial.print("Turn on device id: ");
    Serial.print(id);
    Serial.print("...");
    if (!powerState) { toggleCallback(); Serial.println("-OK"); } else { Serial.println("...Current flow indicates circuit already closed."); }
  } else {
    Serial.print("Turn on for unknown device id: ");
    Serial.println(id);    
  }     
}

void SinricSwitch3Way::sinricOff(String id) {
   if (id == deviceID) // Device ID of first device
   {  
     Serial.print("Turn off Device ID: ");
     Serial.print(id);
     Serial.print("...");
     if (powerState) { toggleCallback(); Serial.println("-OK"); } else { Serial.println("...Current flow indicates circuit already open."); }
   } else {
     Serial.print("Turn off for unknown device id: ");
     Serial.println(id);    
  }
}

void SinricSwitch3Way::handleRoot(){
  uint64_t now = millis();
  char buff[21];
  sprintf(buff, "%" PRIu64, now/1000);
  server->send(200, "text/plain", 
  "Uptime: " + String(buff) + " seconds.  Call /reset if you want to reset me (deviceID=" + deviceID + ")...");
}

void SinricSwitch3Way::handleReset(){
  server->send(200, "text/plain", "OMG WERE GONNA DIE!!!!");
  delay(250);
  resetCallback();
}

void SinricSwitch3Way::handleToggle(){
  server->send(200, "text/plain", "Toggling switch...");
  delay(250);
  toggleCallback();
}

void SinricSwitch3Way::setPowerState(bool newState) {
  if (powerState != newState) {
    Serial.print("Changing switch state...currentFlowing:");
    Serial.println(newState);
    String newValue = newState ? "ON" : "OFF";
    setPowerStateOnServer(newValue.c_str());
  }
  powerState = newState;
}

// Call ONLY If status changed manually. DO NOT CALL THIS IN loop() and overload the server. 
void SinricSwitch3Way::setPowerStateOnServer(const char* value) {
  Serial.print("Updating status on server.  Setting deviceID:");
  Serial.print(deviceID);
  Serial.print(" new state:");
  Serial.println(value);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["deviceId"] = deviceID;
  root["action"] = "setPowerState";
  root["value"] = value;
  StreamString databuf;
  root.printTo(databuf);
  webSocket.sendTXT(databuf);
}
