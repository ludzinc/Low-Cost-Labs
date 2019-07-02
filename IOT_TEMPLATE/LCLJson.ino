//******************************************************************************************************************************
// JSON config file actions
//******************************************************************************************************************************
bool loadConfig() {

  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return false;
  } else {
    Serial.println("Opened config file successfully");

    size_t size = configFile.size();
    if (size > 1024) {
      Serial.println("Config file size is too large");
      return false;
    }
    Serial.print("Congif file size is: "); Serial.println(size);
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);              // buffer is named 'buf' and is set to the size of the opened json file
    configFile.readBytes(buf.get(), size);                    // read bytes from congfigFile (read from SPIFFs) into the file buffer named 'buf'
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());     // make a JasonObject of name "json" to store the file contents read from
    if (!json.success()) {
      Serial.println("Failed to parse config file");
      return false;
    }
    json["SSID"].as<String>().toCharArray(ssid, 30);
    Serial.print("SSID is : ");Serial.println(ssid);
    json["Password"].as<String>().toCharArray(password, 30);
    Serial.print("Password is : ");Serial.println(password);
    json["hostName"].as<String>().toCharArray(hostName, 30);
    json["APName"].as<String>().toCharArray(apSSID, 30);
    json["APPass"].as<String>().toCharArray(apPASS, 30);
    return true;
  }
}//end loadConfig()
//******************************************************************************************************************************
// Save settings to config file
//******************************************************************************************************************************
bool saveConfig() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& rootSave = jsonBuffer.createObject();
  String tempString = "";

  rootSave["SSID"]      = ssid;
  rootSave["Password"]  = password;
  rootSave["hostName"]  = hostName;
  rootSave["APName"]    = apSSID;
  rootSave["APPass"]    = apPASS;
  File configFile   = SPIFFS.open("/config.json", "w");
  Serial.println("Save Config opening SPIFFS File");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }
  rootSave.printTo(configFile);
  Serial.println("Config Saved.");
  return true;
}//end saveConfig()
