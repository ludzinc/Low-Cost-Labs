//******************************************************************************************************************************
// Format bytes
//******************************************************************************************************************************
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
// determine file type by extension
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}//end formatBytes()
//******************************************************************************************************************************
// Read requested file from SPIFFS, extracting .gzip contents if required
//******************************************************************************************************************************
bool handleFileRead(String path) {
  String GPIO = "";
  int valGPIO = 0;
  Serial.println("handleFileRead: " + path);
  Serial.print("Processing Requst: "); Serial.println(path);
  if (path.endsWith("/")) path += "home.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  String pathIcons = "/icons" + path;

  if (path.startsWith("/GPIO_")) {
    GPIO = path.substring(6,7);
    Serial.print("GPIO Selected is: ");Serial.println(GPIO);
    valGPIO=GPIO.toInt();
    Serial.print("VAL_GPIO Selected is: ");Serial.println(GPIO);
    if(path.endsWith("_ON")){
      digitalWrite(valGPIO, LOW);
    } else {
      digitalWrite(valGPIO, HIGH);
    }
    server.send(204, "text/plain", "");
    // Action completed.  No requirement to serve file: exit.
    return true;                                        
  }

  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(pathIcons) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    if (SPIFFS.exists(pathIcons)) {
      path = "/icons" + path;
    }

  } else {
    path = "/404.html";
    Serial.println("File not found, Load 404 Page.");
    contentType = "text/html";
  }
  Serial.print("Content Type is : "); Serial.print(contentType); Serial.print(" and path is : "); Serial.println(path);
  File file = SPIFFS.open(path, "r");                   // load requested file from SPIFFS
  if (!file) {
    Serial.println("SPIFFS open failed.  Show 404.");   // something has gone horribly wrong (should at least find 404.html)
    delay(500);
    return false;                                       // calling fucntion can load a basic 404 text response
  }
  size_t sent = server.streamFile(file, contentType);   // stream file to web reader.
  file.close();
  return true;
}//end handleFileRead()
//******************************************************************************************************************************
// Handle file upload
//******************************************************************************************************************************
void handleFileUpload() {
  if (server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("handleFileUpload Name: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}
//******************************************************************************************************************************
// Handle file delete
//******************************************************************************************************************************
void handleFileDelete() {
  if (server.args() == 0) return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return server.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}//end handleFileDelete()
//******************************************************************************************************************************
// Handle file create
//******************************************************************************************************************************
void handleFileCreate() {
  if (server.args() == 0)
    return server.send(500, "text/plain", "BAD ARGS");
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/")
    return server.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return server.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return server.send(500, "text/plain", "CREATE FAILED");
  (200, "text/plain", "");
  path = String();
}//end handleFileCreate()
//******************************************************************************************************************************
// Handle File List
//******************************************************************************************************************************
void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}//end handleFileList