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
    } else {
      Serial.println("Unknown JSON Type");
    }
  } else if ((jsonData["type"]) == "saveNetworkData") {
    message = saveNetworkConfig(jsonData);
  } else if ((jsonData["type"]) == "saveTemperature") {
    message = saveTemperatureConfig(jsonData);
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
  queryTemperature();                                  // get temperature from I2C sensor (rename routine) TO DO TO DO
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
//  Send temperature and humidity data
//******************************************************************************************************************************
String sendEnvironmentals() {
  String jsonLocalName = "";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  queryTemperature();                                  // get temperature from I2C sensor (rename routine) TO DO TO DO
  root["echo"] = "getEnvironmentals";
  root["mode"] = temperatureChoice;
  root["slope"] = tempSlope;
  root["intercept"] = tempIntercept;
  root["temperature"] = currentTemperature;
  root["humidity"] = currentHumidity;
  root.printTo(jsonLocalName);
  return jsonLocalName;
}
