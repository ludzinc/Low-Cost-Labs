//******************************************************************************************************************************
// Query companion microcontroller for Temperature sensor information for display
// now reads from onboard temp sensor- rename to queryTemperature
//******************************************************************************************************************************
void queryENVData() {
  Serial.println("Requesting Environmentals data for MQTT payload.");
  readSHT30();
}
//******************************************************************************************************************************
// Read temperature from SHT30
//******************************************************************************************************************************
void readSHT30() {
  if (sht30.get() == 0) {
    digitalWrite(redLED, HIGH);
    Serial.print("Temperature in Celsius : ");
    Serial.println(sht30.cTemp);
    Serial.print("Relative Humidity : ");
    Serial.println(sht30.humidity);
    Serial.println();
    currentTemperature = sht30.cTemp;
    currentHumidity = sht30.humidity;
  } else {
    Serial.println("Error!");
  }
  digitalWrite(redLED, LOW);

}
//******************************************************************************************************************************
// Check if time up for reading sensor
//******************************************************************************************************************************
void envStatus() {
  char buf[10];
  now = millis();
  // Publishes new temperature and humidity every 30 seconds
  if (now - lastMeasure > loopTime) {
    lastMeasure = now;
    Serial.println("Requesting Environmentals (looptime expired).");
    readSHT30();
  }
}
