//******************************************************************************************************************************
// JSON config file actions
//******************************************************************************************************************************
bool loadConfig() {

  File configFile = SPIFFS.open("/lclconfig.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  }
  Serial.println("Opened config file successfully");

  size_t size = configFile.size();
  if (size > 1024) {
    DEBUG_PRINTLN("Config file size is too large");
    return false;
  }
  DEBUG_PRINT("Congif file size is: "); DEBUG_PRINTLN(size);
  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);              // buffer is named 'buf' and is set to the size of the opened json file
  configFile.readBytes(buf.get(), size);                    // read bytes from congfigFile (read from SPIFFs) into the file buffer named 'buf'
  //  DynamicJsonBuffer jsonBuffer;
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());     // make a JasonObject of name "json" to store the file contents read from
  if (!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }
  uint8_t modeArraySize =     json["buttonMode"].size();
  uint8_t pwmArraySize =      json["buttonPWM"].size();
  json["Version"].as<String>().toCharArray(gdiVersion, 30);
  json["SSID"].as<String>().toCharArray(ssid, 30);
  json["Password"].as<String>().toCharArray(password, 30);
  json["hostName"].as<String>().toCharArray(hostName, 30);
  json["temperatureChoice"].as<String>().toCharArray(temperatureChoice, 30);
  json["APName"].as<String>().toCharArray(apSSID, 30);
  json["APPass"].as<String>().toCharArray(apPASS, 30);
  
//  DEBUG_PRINT("Loaded gdiVersion: "); DEBUG_PRINTLN(gdiVersion);
//  DEBUG_PRINT("Loaded SSID: "); DEBUG_PRINTLN(ssid);
//  DEBUG_PRINT("Loaded Password: "); DEBUG_PRINTLN(password);
//  DEBUG_PRINT("Loaded HostName: "); DEBUG_PRINTLN(hostName);
//  DEBUG_PRINT("Loaded Temp Choice: "); DEBUG_PRINTLN(temperatureChoice);
//  DEBUG_PRINT("Loaded apSSID: "); DEBUG_PRINTLN(apSSID);
//  DEBUG_PRINT("Loaded apPASS: "); DEBUG_PRINTLN(apPASS);
//  DEBUG_PRINTLN("Reading Input Modes from JSON File");
//  DEBUG_PRINT("Mode Array size is "); DEBUG_PRINTLN(modeArraySize);
  const char* tempName = "x";
  for (int i = 0; i < modeArraySize; i++) {
    buttons[i].mode = json["buttonMode"][i];  // choose mode of input buttons, 0 = door, 1 = light
    inputs[i].mode  = json["inputMode"][i];   // choose mode of input terminals, 0 = door, 1 = light
    tempName              = json["buttonNames"][i];
    htmlButtonNames[i]    = tempName;         // cast a string explict to fix???? TO DO TO DO
    buttons[i].show       = json["show"][i];
    buttons[i].onTime     = json["onTime"][i];
  }
  for (int i = 0; i < modeArraySize; i++) {
    DEBUG_PRINT("IO Config button "); DEBUG_PRINT(i + 1); DEBUG_PRINT(" is "); DEBUG_PRINTLN(buttons[i].mode);
  }
  Serial.println("IOConfig data read");
  for (int i = 0; i < modeArraySize; i++) {
    DEBUG_PRINT("IO Input ");         DEBUG_PRINT(i + 1); DEBUG_PRINT(" is "); DEBUG_PRINTLN(inputs[i].mode);
  }
  for (int i = 0; i < modeArraySize; i++) {
    DEBUG_PRINT("Button name "); DEBUG_PRINT(i + 1); DEBUG_PRINT(" is "); DEBUG_PRINTLN(htmlButtonNames[i]);
  }
  for (int i = 0; i < modeArraySize; i++) {
    DEBUG_PRINT("Button Show "); DEBUG_PRINT(i + 1); DEBUG_PRINT(" is "); DEBUG_PRINTLN(buttons[i].show);
  }
  for (int i = 0; i < modeArraySize; i++) {
    DEBUG_PRINT("Button onTime "); DEBUG_PRINT(i + 1); DEBUG_PRINT(" is "); DEBUG_PRINTLN(buttons[i].onTime);
  }
  DEBUG_PRINTLN("Reading PWM Values from JSON File");
  DEBUG_PRINT("PWM Array size is "); DEBUG_PRINTLN(pwmArraySize);
  for (int i = 0; i < pwmArraySize; i++) {
    panelButtonPWM[i] = json["buttonPWM"][i];  // Read PWM Value for button illumination, 0 - 1024
    DEBUG_PRINT("Button PWM_"); DEBUG_PRINT(i); DEBUG_PRINT(" is "); DEBUG_PRINTLN(panelButtonPWM[i]);
  }
  tempSlope     = json["tempSlope"];
  DEBUG_PRINT("Temperature Slope Calibration is: "); DEBUG_PRINTLN(tempSlope);
  tempIntercept = json["tempIntercept"];
  DEBUG_PRINT("Temperature Slope Intercept is: "); DEBUG_PRINTLN(tempIntercept);
  return true;
}
//******************************************************************************************************************************
// Save settings to config file *LUDZINC*
//******************************************************************************************************************************
bool saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootSave = jsonBuffer.createObject();
  String tempString = "";
  
  rootSave["Version"]   = gdiVersion;
  rootSave["SSID"]      = ssid;
  rootSave["Password"]  = password;
  rootSave["hostName"]  = hostName;
  rootSave["temperatureChoice"] = temperatureChoice;
  rootSave["APName"]    = apSSID;
  rootSave["APPass"]    = apPASS;
  Serial.println("Save Config creating nested array's");
  JsonArray& buttonMode   = rootSave.createNestedArray("buttonMode");
  JsonArray& inputMode    = rootSave.createNestedArray("inputMode");
  JsonArray& buttonNames  = rootSave.createNestedArray("buttonNames");
  JsonArray& show         = rootSave.createNestedArray("show");
  JsonArray& onTime       = rootSave.createNestedArray("onTime");
  JsonArray& buttonPWM    = rootSave.createNestedArray("buttonPWM");
  Serial.println("Save Config saving temp cal numbers");
  rootSave["tempSlope"]       = tempSlope;
  rootSave["tempIntercept"]   = tempIntercept;
  Serial.println("Save Config adding button modes.");
  for (int i = 0; i < 4; i++) {
    buttonMode.add(buttons[i].mode);
    inputMode.add(inputs[i].mode);
    buttonNames.add(htmlButtonNames[i]);
    show.add(buttons[i].show);
    onTime.add(buttons[i].onTime);
  }
  Serial.println("Save Config adding PWM data");
  for (int i = 0; i < 2; i++) {
    buttonPWM.add(panelButtonPWM[i]);
  }
  //  File configFile   = SPIFFS.open("/lclconfig.json", "w");
  Serial.println("Save Config opening SPIFFS File");
  File configFile   = SPIFFS.open("/test.json", "w"); // this is for testinng - swap names next
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  rootSave.printTo(configFile);
  Serial.println("Config Saved.");
  return true;
}

