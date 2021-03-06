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
    } else if ((jsonData["data"]) == "configButtons") {
      message = sendButtonConfig();
    } else if ((jsonData["data"]) == "configButtonPWM") {
      message = sendButtonPWM();
    } else if ((jsonData["data"]) == "localName") {
      message = sendLocalName();
    } else if ((jsonData["data"]) == "listSSIDs") {
      message = sendSSID();
    } else {
      Serial.println("Unknown JSON Type");
    }
  } else if ((jsonData["type"]) == "saveNetworkData") {
    message = saveNetworkConfig(jsonData);
  } else if ((jsonData["type"]) == "saveButtons") {
    message = saveButtonConfig(jsonData);
  } else if ((jsonData["type"]) == "saveTemperature") {
    message = saveTemperatureConfig(jsonData);
  } else if ((jsonData["type"]) == "saveButtonPWM") {
    message = saveButtonPWMConfig(jsonData);
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
// Send Button Config Info when requested
//******************************************************************************************************************************
// harmonise variable names across files.  in lclconfig.json delayValue should be onTime
// use onTime (save to config file) and show (save to config file)
// TO DO TO DO
String sendButtonConfig() {
  String jsonButtonConfig = "";
  const size_t bufferSize = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(2) + 4 * JSON_OBJECT_SIZE(4);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  root["echo"] = "configButtons";
  JsonArray& buttonNames  = root.createNestedArray("buttonNames");
  JsonArray& buttonMode   = root.createNestedArray("buttonMode");
  JsonArray& show         = root.createNestedArray("show");
  JsonArray& onTime       = root.createNestedArray("onTime");
  JsonArray& buttonPWM    = root.createNestedArray("buttonPWM");
  char tempName[4][50];
  for (int i = 0; i < 4; i++) {
    buttonNames.add(htmlButtonNames[i]);
    buttonMode.add(buttons[i].mode);
    show.add(buttons[i].show);
    onTime.add(buttons[i].onTime);
  }
  for (int i = 0; i < 2; i++) {
    buttonPWM.add(panelButtonPWM[i]);
  }
  root.printTo(jsonButtonConfig);
  return jsonButtonConfig;
}
//******************************************************************************************************************************
// Send Button PWM Info when requested
//******************************************************************************************************************************
String sendButtonPWM() {
  String jsonButtonConfig = "";
  const size_t bufferSize = JSON_ARRAY_SIZE(4) + JSON_OBJECT_SIZE(2) + 4 * JSON_OBJECT_SIZE(4);
  DynamicJsonBuffer jsonBuffer(bufferSize);
  JsonObject& root = jsonBuffer.createObject();
  root["echo"] = "configButtonPWM";
  JsonArray& buttonPWM    = root.createNestedArray("buttonPWM");
  for (int i = 0; i < 2; i++) {
    buttonPWM.add(panelButtonPWM[i]);
  }
  root.printTo(jsonButtonConfig);
  return jsonButtonConfig;
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
// Save Buttons Configuration from json data to file
//******************************************************************************************************************************
String saveButtonConfig(JsonObject &jsonData) {
  for (int i = 0; i < 4 ; i++) {
    buttons[i].mode     = jsonData["buttonMode"][i];  // choose mode of input buttons, 0 = door, 1 = light
    inputs[i].mode      = jsonData["buttonMode"][i];  // choose mode of input buttons, 0 = door, 1 = light
    htmlButtonNames[i]  = jsonData["buttonNames"][i].as<String>();
    buttons[i].show     = jsonData["show"][i];
    buttons[i].onTime   = jsonData["onTime"][i];  // choose mode of input buttons, 0 = door, 1 = light
  }
  saveConfig();
  String message = "{\"echo\":\"saveButtons\"}";
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
// Save front panel button PWM settings from json data to file (0 - 1023 for PWM values)
//******************************************************************************************************************************
String saveButtonPWMConfig(JsonObject &jsonData) {
  for (int i = 0; i < 2 ; i++) {
    int temp = 0;
    temp = jsonData["buttonPWM"][i];
    Serial.print("SHOWING PWM DATA :");Serial.println(temp);
    panelButtonPWM[i] =  jsonData["buttonPWM"][i];      // 0 - 1023 for dim - bright
  }
  saveConfig();
  analogWrite(greenButtonLED, panelButtonPWM[0]);
  analogWrite(redButtonLED, panelButtonPWM[1]);
  String message = "{\"echo\":\"saveButtonPWM\"}";
  return message;
}
