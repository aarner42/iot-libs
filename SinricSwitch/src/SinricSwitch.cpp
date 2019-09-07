#include "SinricSwitch.h"


//<<constructor>>
SinricSwitch::SinricSwitch() {
    Serial.println("default constructor called");
    }

//useful constructor
SinricSwitch::SinricSwitch(String apiKey, String device_id, unsigned int port, vCallBack onCB,
                           vCallBack offCB, vCallBack alertCB, vCallBack rebootCB, vCallBack resetCB) {
    deviceID = std::move(device_id);
    connectedToSinric=false;
    onCallback = onCB;
    offCallback = offCB;
    alertCallback = alertCB;
    rebootCallback = rebootCB;
    resetCallback = resetCB;
    powerState = false;
    startWebServer(port);
    startSinricClient(std::move(apiKey));
    heartbeatTimestamp = 0;
    pingTimeStamp = 0;
    Serial.print("Registered switch with deviceID=[");
    Serial.print(deviceID);
    Serial.println("]");
}

//<<destructor>>
SinricSwitch::~SinricSwitch() = default;

void SinricSwitch::sinricLoop() {
    webSocket.loop();

    if (connectedToSinric) {
        uint64_t now = millis();
        pingTimeStamp = now;
        // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
        if ((now < 10000) || (now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
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


void SinricSwitch::webLoop() {
    if (server != nullptr) {
        server->handleClient();
        delay(1);
    }
}

void SinricSwitch::loop() {
    webLoop();
    sinricLoop();
}

void SinricSwitch::startWebServer(unsigned int localPort) {
    server = new ESP8266WebServer(localPort);

    server->on("/", [&]() {
        handleRoot();
    });

    server->on("/on", [&]() {
        onCallback();
    });
    server->on("/off", [&]() {
        offCallback();
    });
    server->on("/reboot", [&]() {
        rebootCallback();
    });
    server->on("/resetAll", [&]() {
        handleReset();
    });


    server->begin();

    Serial.print("WebServer started on port: ");
    Serial.println(localPort);
}

void SinricSwitch::startSinricClient(const String& apiKey) {
    webSocket.begin("iot.sinric.com", 80, "/");

    // event handler uses anonymous function to pass in instance method instead of free-member/static
    webSocket.onEvent([&](WStype_t t, uint8_t *p, size_t l) {
        webSocketEvent(t, p, l);
    });
    webSocket.setAuthorization("apikey", apiKey.c_str());
    Serial.println("WebSocket handlers/authorization registered...");

    // try again every 60 s if connection has failed
    webSocket.setReconnectInterval(60000);
    Serial.println("Setup complete...");
}

void SinricSwitch::webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {

    switch (type) {
        case WStype_ERROR: {
            connectedToSinric = true;
            Serial.printf("[WSc] Webservice ERROR!\n");
        }
            break;
        case WStype_FRAGMENT_TEXT_START: {
            connectedToSinric = true;
            Serial.printf("[WSc] Webservice Text Fragment begin\n");
        }
            break;
        case WStype_FRAGMENT_BIN_START: {
            connectedToSinric = true;
            Serial.printf("[WSc] Webservice Binary Fragment begin\n");
        }
            break;
        case WStype_FRAGMENT_FIN: {
            connectedToSinric = true;
            Serial.printf("[WSc] Webservice Fragment finished\n");
        }
            break;
        case WStype_FRAGMENT: {
            connectedToSinric = true;
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

            DynamicJsonDocument jsonDoc(1024);
            auto error = deserializeJson(jsonDoc, payload);
            if (error) {
                Serial.printf("SEVERE - Can't deserialize JSON object from WebSocket stream:%s!\n", error.c_str());
                return;
            }

            String deviceId = jsonDoc["deviceId"];
            String action = jsonDoc["action"];

            if (action == "setPowerState") { // Switch or Light
                String value = jsonDoc["value"];
                if (value == "ON") {
                    sinricOn(deviceId);
                } else {
                    sinricOff(deviceId);
                }
            } else if (action == "test") {
                Serial.println("[WSc] received test command from sinric.com");
            }
        }
            break;
        case WStype_BIN:
            connectedToSinric = true;
            Serial.printf("[WSc] get binary length: %u\n", length);
            break;
        case WStype_PING:
            connectedToSinric = true;
            break;
        case WStype_PONG:
            connectedToSinric = true;
            break;
    }
}

void SinricSwitch::sinricOn(const String& id) {
    if (deviceID.startsWith(id))
    {
        Serial.print("Turn on  device id: [");
        Serial.print(id);
        Serial.print("] == [");
        Serial.print(deviceID);
        Serial.print("] cmp: ");
        Serial.println(deviceID.startsWith(id));
        powerState = true;
        onCallback();
    } else {
        Serial.print("Turn on  for unknown device id: [");
        Serial.print(id);
        Serial.print("] != [");
        Serial.print(deviceID);
        Serial.print("] cmp: ");
        Serial.println(deviceID.startsWith(id));

    }
}

void SinricSwitch::sinricOff(const String& id) {
    if (deviceID.startsWith(id))
    {
        Serial.print("Turn off device id: [");
        Serial.print(id);
        Serial.print("] == [");
        Serial.print(deviceID);
        Serial.print("] cmp: ");
        Serial.println(deviceID.startsWith(id));
        powerState = false;
        offCallback();
    } else {
        Serial.print("Turn off for unknown device id: [");
        Serial.print(id);
        Serial.print("] != [");
        Serial.print(deviceID);
        Serial.print("] cmp: ");
        Serial.println(deviceID.startsWith(id));
    }
}

void SinricSwitch::handleRoot() {
    uint64_t now = millis();
    char buff[21];
    sprintf(buff, "%" PRIu64, now / 1000);
    server->send(200, "text/plain",
                 "Uptime: " + String(buff) + " seconds.  Call /reset if you want to reset me (deviceID=" + deviceID +
                 ")...");
}

void SinricSwitch::handleReset() {
    server->send(200, "text/plain", "OMG WERE GONNA DIE!!!!");
    delay(250);
    resetCallback();
}

void SinricSwitch::setPowerState(bool newState) {
    if (powerState != newState) {
        Serial.print("Changing switch state...relayClosed:");
        Serial.println(newState);
        setPowerStateOnServer(newState ? ON_STATE : OFF_STATE);
    }
    powerState = newState;
}

// Call ONLY If status changed manually. DO NOT CALL THIS IN loop() and overload the server. 
void SinricSwitch::setPowerStateOnServer(const char *value) {
    Serial.print("Updating status on server...");

    String sendString = String(deviceID.c_str());  //god c sucks balls when it comes to strings.  stupid carriage returns
    sendString.replace("\r", "");

    DynamicJsonDocument outbound(128);
    outbound["deviceId"] = sendString;
    outbound["action"] = "setPowerState";
    outbound["value"] = value;
    StreamString databuf;
    serializeJson(outbound, databuf);
    webSocket.sendTXT(databuf);
}
