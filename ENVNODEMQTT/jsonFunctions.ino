//******************************************************************************************************************************
//  Parse rx's websocket text and check if it's a valid request
//******************************************************************************************************************************
void parseWebsocketText(String text, uint8_t num) {
  Serial.println("Parseing Web Socket.");
  String message = "{\"echo\":\"Invalid Message Received.\"}";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonData = jsonBuffer.parseObject(text);
  const char* type = jsonData["type"];
  if ((jsonData["type"]) == "request") {
    if ((jsonData["data"]) == "configTemperature") {
      message = sendTemperature();
    } else if ((jsonData["data"]) == "localName") {
      message = sendLocalName();
    } else if ((jsonData["data"]) == "listSSIDs") {
      message = sendSSID();
    } else if ((jsonData["data"]) == "getEnvironmentals") {
      message = sendEnvironmentals();
    } else if ((jsonData["data"]) == "MQTTSettings") {
      message = sendMQTTSettings();
    } else {
      Serial.println("Unknown JSON Type");
    }
  } else if ((jsonData["type"]) == "saveNetworkData") {
    message = saveNetworkConfig(jsonData);
  } else if ((jsonData["type"]) == "saveTemperature") {
    message = saveTemperatureConfig(jsonData);
  } else if ((jsonData["type"]) == "saveTemperature") {
    message = saveTemperatureConfig(jsonData);
  } else if ((jsonData["type"]) == "saveMQTTdata") {
    message = saveMQTTConfig(jsonData);
  }

  Serial.println("Sending Websocket Message:");
  Serial.println(message);
  webSocket.sendTXT(num, message.c_str(), message.length());  // Send update data
}
//******************************************************************************************************************************
//  Scan for available SSIDs and send as JSON list to setting page to load.
//******************************************************************************************************************************
String sendSSID() {
  String jsonSSIDList = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["echo"] = "listSSIDs";
  Serial.println("SCANNING for available Networks");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
    return "No networks found";
  }
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    JsonArray& SSIDNames = root.createNestedArray("SSIDName");
    JsonArray& SSIDRSSIs = root.createNestedArray("SSIDRSSI");
    JsonArray& SSIDTypes = root.createNestedArray("SSIDType");
    for (int i = 0; i < n; ++i)
    {
      SSIDNames.add(WiFi.SSID(i));
      SSIDRSSIs.add(WiFi.RSSI(i));
      //SSIDTypes.add(WiFi.encryptionType(i));
      if (WiFi.encryptionType(i) == 2) {
        SSIDTypes.add("TKIP (WPA)");
      } else if (WiFi.encryptionType(i) == 4) {
        SSIDTypes.add("CCMP (WPA)");
      } else if (WiFi.encryptionType(i) == 5) {
        SSIDTypes.add("WEP");
      } else if (WiFi.encryptionType(i) == 7) {
        SSIDTypes.add("NONE");
      } else if (WiFi.encryptionType(i) == 8) {
        SSIDTypes.add("AUTO");
      } else {
        SSIDTypes.add("UNKNOWN");
      }
    }
  }
  root.printTo(jsonSSIDList);
  return jsonSSIDList;
}
//******************************************************************************************************************************
//  Send local name after recieving request
//******************************************************************************************************************************
String sendLocalName() {
  String jsonLocalName = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["echo"] = "localName";
  root["LocalName"] = hostName;
  root.printTo(jsonLocalName);
  return jsonLocalName;
}
//******************************************************************************************************************************
//  Send local name after recieving request
//******************************************************************************************************************************
String sendTemperature() {
  String jsonLocalName = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  queryENVData();                                  // get temperature from I2C sensor (rename routine) TO DO TO DO
  root["echo"] = "configTemperature";
  root["mode"] = temperatureChoice;
  root["slope"] = tempSlope;
  root["intercept"] = tempIntercept;
  root["data"] = currentTemperature;
  root.printTo(jsonLocalName);
  return jsonLocalName;
}
//******************************************************************************************************************************
// Save Network Configuration from json data to file
//******************************************************************************************************************************
String saveNetworkConfig(JsonObject &jsonData) {
  jsonData["newSSID"].as<String>().toCharArray(ssid, 30);
  jsonData["newPass"].as<String>().toCharArray(password, 30);
  jsonData["newLocalName"].as<String>().toCharArray(hostName, 30);
  saveConfig();
  String  message = "{\"echo\":\"saveNetworkData\"}";
  return message;
}
//******************************************************************************************************************************
// Save Temperature Configuration (calibration) from json data to file (C to F conversion in html page)
//******************************************************************************************************************************
String saveTemperatureConfig(JsonObject &jsonData) {
  String tempString = jsonData["mode"];
  tempString.toCharArray(temperatureChoice, 10);
  tempSlope         = jsonData["slope"];
  tempIntercept     = jsonData["intercept"];
  saveConfig();
  String message = "{\"echo\":\"saveTemperature\"}";
  return message;
}
//******************************************************************************************************************************
// Save MQTT Configuration from json data to file
//******************************************************************************************************************************
String saveMQTTConfig(JsonObject &jsonData) {
  String tempString = jsonData["mode"];
  jsonData["newMqttServer"].as<String>().toCharArray(mqttServer, 30);
  jsonData["newMqttSubscription"].as<String>().toCharArray(mqttSubscription, 30);
  jsonData["newMqttPublish"].as<String>().toCharArray(mqttPublish, 30);
  saveConfig();
  String message = "{\"echo\":\"saveMQTTdata\"}";
  return message;
}
//******************************************************************************************************************************
//  Send temperature and humidity data
//******************************************************************************************************************************
String sendEnvironmentals() {
  String jsonLocalName = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  queryENVData();                                  // get temperature from I2C sensor (rename routine) TO DO TO DO
  root["echo"] = "getEnvironmentals";
  root["mode"] = temperatureChoice;
  root["slope"] = tempSlope;
  root["intercept"] = tempIntercept;
  root["temperature"] = currentTemperature;
  root["humidity"] = currentHumidity;
  root.printTo(jsonLocalName);
  return jsonLocalName;
}
//******************************************************************************************************************************
//  Send MQTT settings data
//******************************************************************************************************************************
String sendMQTTSettings() {
  String jsonLocalName = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["echo"] = "MQTTSettings";
  root["mqttServer"] = mqttServer;
  root["mqttSubscription"] = mqttSubscription;
  root["mqttPublish"] = mqttPublish;
  root.printTo(jsonLocalName);
  return jsonLocalName;
}
//******************************************************************************************************************************
//  Format environmentals data (Temperature and Humidity) into json string
//******************************************************************************************************************************
String formatEnvironmentals(float temperature, float humidity) {
  String jsonEnvironmentals = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["device"] = hostName;
  JsonObject& envData = root.createNestedObject("envData");
  envData["temperature"] = temperature;
  envData["humidity"] = humidity;
  root.printTo(jsonEnvironmentals);
  return jsonEnvironmentals;
}
//******************************************************************************************************************************
//  Parse recieved data
//******************************************************************************************************************************
//String parseJSON(String text) {
void parseJSON(String text) {
  // what gets num??????
  Serial.println("Parseing MQT:");
  String message = "{\"echo\":\"Invalid Message Received.\"}";
  // do i need this??
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonData = jsonBuffer.parseObject(text);
  Serial.print("Checking if valid JSON: ");
  if ((jsonData.success()) == 1) { // data is JSON format
    Serial.println("Success");
    const char* action = jsonData["action"];
    //    Serial.print("action is: ");Serial.println(action);
    if ((jsonData["action"]) == "command") {
      Serial.println("Command is detected");
      if ((jsonData["io"]) == 0) {
        Serial.println("Actioning IO 01");
        if ((jsonData["state"]) == "on") {
          digitalWrite(LED_BUILTIN, LOW);
          Serial.println("LED_BUILTIN is ON");
        } else {
          if ((jsonData["state"]) == "off") {
            digitalWrite(LED_BUILTIN, HIGH);
            Serial.println("LED_BUILTIN is OFF");
          }
        }
      }
      if ((jsonData["io"]) == 5) {
        Serial.println("Actioning IO D5");
        if ((jsonData["state"]) == "on") {
          //          digitalWrite(GREEN, HIGH);
          Serial.println("GREEN LED is ON");
        } else {
          if ((jsonData["state"]) == "off") {
            //            digitalWrite(GREEN, LOW);
            Serial.println("GREEN LED is OFF");
          }
        }
      }
      if ((jsonData["io"]) == 6) {
        Serial.println("Actioning IO D6");
        if ((jsonData["state"]) == "on") {
          //          digitalWrite(RED, HIGH);
          Serial.println("RED LED is ON");
        } else {
          if ((jsonData["state"]) == "off") {
            //            digitalWrite(RED, LOW);
            Serial.println("RED LED is OFF");
          }
        }
      }
    }
  } else {
    Serial.println("Text is not JSON format");
  }
  //  return text;
}
