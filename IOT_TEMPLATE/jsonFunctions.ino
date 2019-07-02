//******************************************************************************************************************************
//  Parse rx's websocket text and check if it's a valid request
//******************************************************************************************************************************
void parseWebsocketText(String text, uint8_t num) {
  Serial.println("Parseing Web Socket.");
  String message = "{\"echo\":\"Invalid Message Received.\"}";
  DynamicJsonBuffer jsonBuffer;
  JsonObject& jsonData = jsonBuffer.parseObject(text);
  const char* type = jsonData["type"];
  const char* data = jsonData["data"];
  if ((jsonData["type"]) == "queryGPIO") {
    Serial.println("Processing queryGPIO");
    message = processGPIO(jsonData);
  } else {
    Serial.println("Unknown Request Type");
  }
  Serial.println("Sending Websocket Message:");
  Serial.println(message);
  webSocket.sendTXT(num, message.c_str(), message.length());  // Send update data
}//end parseWebsocketText()
//******************************************************************************************************************************
// Echo GPIO status to Client
//******************************************************************************************************************************
String processGPIO(JsonObject &jsonData) {
  const char* GPIO = "";
  String state = "";
  int valGPIO = 0;
  String jsonLocalName = "";
  GPIO = jsonData["data"];
  Serial.print("GPIO from jsonDATA is: "); Serial.println(GPIO);
  //  GPIO = GPIO.substring(6,7);
  valGPIO = atoi(GPIO);
  Serial.print("Processing GPIO: "); Serial.println(valGPIO);
  Serial.print("Reading GPIO gives: "); Serial.println(digitalRead(valGPIO));
  if (digitalRead(valGPIO) == 1) {  // active low LED on WEMOS D1 MINI!
    state = "OFF";
  } else {
    state = "ON";
  }
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["echo"] = "Status GPIO";
  root["status"] = state;
  root.printTo(jsonLocalName);
  Serial.println("Sending processed GPIO status");
  return jsonLocalName;
}//end processGPIO()
