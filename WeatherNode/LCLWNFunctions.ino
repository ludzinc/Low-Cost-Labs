//******************************************************************************************************************************
// Query companion microcontroller for Temperature sensor information for display
// now reads from onboard temp sensor- rename to queryTemperature
//******************************************************************************************************************************
void queryTemperature() {
  Serial.println("Requesting Temperature");        // Read temp darta from TCN75A Sensor
  readSHT30();
}
//******************************************************************************************************************************
// Read temperature from SHT30
//******************************************************************************************************************************
void readSHT30() {
  if (sht30.get() == 0) {
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
    readSHT30();
    // Publishes Temperature and Humidity values
    sprintf(buf, "%.1f", sht30.cTemp);
    //    client.publish("Garage/Temperature", buf);
    sprintf(buf, "%.1f", sht30.humidity);
    //    client.publish("Garage/Humidity", buf);
  }
}
