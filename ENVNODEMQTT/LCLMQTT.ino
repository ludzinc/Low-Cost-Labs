//******************************************************************************************************************************
// MQTT callback - action message from subscribed topic
//******************************************************************************************************************************
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  // Feel free to add more if statements to control more GPIOs with MQTT
//  if (topic == "Garage/Light") {
 if (topic == mqttSubscription) {
    Serial.print("Subscribed Topic has raised a request : ");
    if (messageTemp == "on") {
      digitalWrite(LED_BUILTIN, LOW);   // Active Low LED
      Serial.print("On");
    }
    else if (messageTemp == "off") {
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print("Off");
    }
    // cehck format messageTemp (is it JSON data?)
    parseJSON(messageTemp);
  }
  Serial.println();
}
//******************************************************************************************************************************
// MQTT reconnect - called when required to subscribe
//******************************************************************************************************************************
boolean reconnect() {
  String tempString = "";
  tempString.concat(MQTTClientName);
  tempString.concat(" (");
  tempString.concat(hostName);
  tempString.concat(") ");
  tempString.concat("connecting.");
  if (client.connect(MQTTClientName.c_str())) {
    Serial.println("MQTT broker connected.");
    // Once connected, publish an announcement...
    client.publish("INFO", tempString.c_str());
    // ... and resubscribe
    //    client.subscribe("Garage/Light");
    client.subscribe(mqttSubscription);
    // TO DO - subscribe to correct topic - add to config file
    // NEED a SUBSCRIPTION and PUBLISH option
    // TO DO - add subscription topic to config file
  } else {
    Serial.print("MQTT broker not connected.  Will retry in "); Serial.print(reconnectTime / 1000); Serial.println(" seconds.");
  }
  return client.connected();
}
