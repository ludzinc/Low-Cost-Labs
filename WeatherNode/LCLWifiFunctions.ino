//******************************************************************************************************************************
// testWiFi - if not connected to local network, start as Access Point
//******************************************************************************************************************************
int testWiFi() {
  uint8_t connectionAttempts = 0;
  uint8_t blinkLED = 0;

  Serial.println("Waiting for Wifi to connect.");
  while ( connectionAttempts < maxConnectionAttempts ) {
    Serial.print(".");
    blinkLED++;
    if (blinkLED > 1) {
      blinkLED = 0;
      digitalWrite(BUILTIN_LED, HIGH);
    } else {
      digitalWrite(BUILTIN_LED, LOW);
    }
    delay(500);
    connectionAttempts++;
    if (WiFi.status() == WL_CONNECTED) {
      digitalWrite(BUILTIN_LED, LOW);
      digitalWrite(LED1, HIGH);
      return 0;
    }
  }
  Serial.println("");
  Serial.print("WiFi Status: ");
  Serial.println(WiFi.status());
  Serial.println("Connection to WiFi failed, starting Access Point");
  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(LED2, HIGH);
  return 1;
}//end testWiFi()
//******************************************************************************************************************************
// Set Up Wifi as an Access Point
//******************************************************************************************************************************
void startAccessPoint() {
  //TO DO - why network scan here?  ok for debugging but not needed now
  String tempName = "";
  String checkName = "";
  String buildName = "";

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  Serial.println("SCANNING for available Networks");
  int n = WiFi.scanNetworks();
  DEBUG_PRINTLN("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  //  WiFi.softAP(apSSID, apPASS);
  WiFi.softAP(apSSID);
  Serial.print("Access Point Started. IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.printf("MAC address = %s\n", WiFi.softAPmacAddress().c_str());
  tempName = WiFi.softAPmacAddress().c_str();
  tempName.replace(":", "");
  //  Serial.print("Temp ESP name is:"); Serial.println(tempName);
  buildName = tempName;
  tempName = ApPrefix + buildName;
  checkName = apSSID;
  if (tempName.equals(apSSID)) {   // Should only run once, when default settings file is loaded to SPIFFS
    Serial.print("ESP name is:"); Serial.println(tempName);
  } else {
    DEBUG_PRINTLN("AP SSID is default factory setting.  Updating to include mac address.");
    tempName.toCharArray(apSSID, tempName.length() + 1);
    DEBUG_PRINT("New apSSID is:"); Serial.println(apSSID);
    saveConfig();
    DEBUG_PRINTLN("Config Saved, Resetting ESP");
    ESP.reset();
  }
  Serial.println("Access point started.");
}
