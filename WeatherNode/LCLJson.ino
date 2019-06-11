//******************************************************************************************************************************
// JSON config file actions
//******************************************************************************************************************************
bool loadConfig() {

  File configFile = SPIFFS.open("/lclconfig.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  } else {
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
    json["Version"].as<String>().toCharArray(wnVersion, 30);
    json["SSID"].as<String>().toCharArray(ssid, 30);
    json["Password"].as<String>().toCharArray(password, 30);
    json["hostName"].as<String>().toCharArray(hostName, 30);
    json["temperatureChoice"].as<String>().toCharArray(temperatureChoice, 30);
    json["APName"].as<String>().toCharArray(apSSID, 30);
    json["APPass"].as<String>().toCharArray(apPASS, 30);
    const char* tempName = "x";
    tempSlope     = json["tempSlope"];
    DEBUG_PRINT("Temperature Slope Calibration is: "); DEBUG_PRINTLN(tempSlope);
    tempIntercept = json["tempIntercept"];
    DEBUG_PRINT("Temperature Slope Intercept is: "); DEBUG_PRINTLN(tempIntercept);
    return true;
  }
}
//******************************************************************************************************************************
// Save settings to config file
//******************************************************************************************************************************
bool saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootSave = jsonBuffer.createObject();
  String tempString = "";

  rootSave["Version"]   = wnVersion;
  rootSave["SSID"]      = ssid;
  rootSave["Password"]  = password;
  rootSave["hostName"]  = hostName;
  rootSave["temperatureChoice"] = temperatureChoice;
  rootSave["APName"]    = apSSID;
  rootSave["APPass"]    = apPASS;
//  Serial.println("Save Config creating nested array's");
//  JsonArray& buttonMode   = rootSave.createNestedArray("buttonMode");
//  JsonArray& inputMode    = rootSave.createNestedArray("inputMode");
//  JsonArray& buttonNames  = rootSave.createNestedArray("buttonNames");
//  JsonArray& show         = rootSave.createNestedArray("show");
//  JsonArray& onTime       = rootSave.createNestedArray("onTime");
//  JsonArray& buttonPWM    = rootSave.createNestedArray("buttonPWM");
  Serial.println("Save Config saving temp cal numbers");
  rootSave["tempSlope"]       = tempSlope;
  rootSave["tempIntercept"]   = tempIntercept;
//  Serial.println("Save Config adding button modes.");
  File configFile   = SPIFFS.open("/lclconfig.json", "w");
  Serial.println("Save Config opening SPIFFS File");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  rootSave.printTo(configFile);
  Serial.println("Config Saved.");
  return true;
}
