//@author: Mehmet BULUT

//Libraries
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <LittleFS.h>
#include <WiFi.h>
#include <SoftwareSerial.h>
//Tokens for RTDB connection
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <math.h>
#include <cmath> 
using namespace std;


SoftwareSerial sim808(13, 15);  // RX, TX

//Network credentials
const char* ssid = "";
const char* password = "";

//Firebase project api key
#define API_KEY ""

//Authortized Email and password
#define USER_EMAIL ""
#define USER_PASSWORD ""

//Firebase storage bucket id
#define STORAGE_BUCKET_ID ""

//Database url
#define DATABASE_URL ""
//Data path for photo
#define FILE_PHOTO_PATH "/photo.jpg"
#define BUCKET_PHOTO "/data/photo.jpg"

// Camera pins according to my ESP32-Cam model(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

//Variables for GPS module
unsigned long loopTime = 0;
unsigned long serialTime = 0;
char GPSData[100] = {0};
char GPSTemp[100];
double latitude, longitude, fenceLatitude, fenceLongitude, fenceRadius, realDistance = 0.0;
bool isPetIn = false;
String msgFrmSIM;
String msgToSIM;
#define earthRadiusKm 6371.0


//Boolean variables for flow control
bool takeNewPhoto = false;
bool isFbReady = false;
bool isPhotoUploaded = false;

//Firebase data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;
unsigned long sendDataPrevMillis = 0;
unsigned long getDataPrevMillis = 0;
int intValue = 0;
int count = 1;

void initWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void initCamera() {
  //OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  //Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}

//LittleFS file system for saving photo
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
    ESP.restart();
  } else {
    delay(500);
    Serial.println("LittleFS mounted successfully");
  }
}

//Capture Photo and Save it to LittleFS
void capturePhotoSaveLittleFS(void) {
  camera_fb_t* fb = NULL;
  // Skip first 3 frames
  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }

  //Take a new photo
  fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }

  //Photo file name
  Serial.printf("Picture file name: %s\n", FILE_PHOTO_PATH);
  File file = LittleFS.open(FILE_PHOTO_PATH, FILE_WRITE);

  //Insert the data in the photo file
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len);  // payload (image), payload length
    Serial.print("The picture has been saved in ");
    Serial.print(FILE_PHOTO_PATH);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  //Close the file
  file.close();
  esp_camera_fb_return(fb);
}



void setsim808() {
  Serial.println("at komutlar manuel gonderildi");
  delay(10);

  sim808.print("AT+CGPSPWR=1\r\n");
  delay(200);  // gps on
  sim808.print("AT+CGNSSEQ=RMC\r\n");
  delay(200);  // gps on
  //sim808.print("AT+CGPSRST=0\r\n");             delay(100); // cold restart

  sim808.print("AT&W\r\n");
  delay(700);  // STORE active profile
}

//Reads messages from SIM
void readsimPort() {
  serialTime = loopTime;
  delay(1);
  while ((loopTime - serialTime) < 200) {
    loopTime = millis();
    while (sim808.available()) {
      loopTime = millis();
      serialTime = loopTime;
      char c = sim808.read();  //gets one byte from serial buffer
      msgFrmSIM += c;           //makes the string readString
    }
  }
  sim808.flush();
  delay(700);
}

void readSerialPort() {
  while (Serial.available()) {
    delay(10);
    if (Serial.available() > 0) {
      char d = Serial.read();  //gets one byte from serial buffer
      msgToSIM += d;           //makes the string readString
    }
  }
  Serial.flush();
  delay(700);
}


//Reading GPS from SIM808 module
void rdGPS() {
  sim808.print("AT+CGNSINF\r\n");
  serialTime = loopTime;
  delay(100);
  while ((loopTime - serialTime) < 200) {
    int index = 0;
    loopTime = millis();
    while (sim808.available()) {
      delay(10);
      loopTime = millis();
      serialTime = loopTime;
      char c = sim808.read();  //Gets one byte from serial buffer
      GPSData[index++] = c;    //Makes the string 
    }
  }
  sim808.flush();
  delay(700);
    strcpy(GPSTemp, GPSData);
   char *tmp1 = NULL;
   tmp1 = strtok(GPSTemp, ",");
  tmp1 = strtok(NULL, ",");
  tmp1 = strtok(NULL, ",");
  tmp1 = strtok(NULL, ",");
  latitude = atof(tmp1);
  tmp1 = strtok(NULL, ",");
  longitude = atof(tmp1);

  //Sending data to the Firebase RTDB
  GPSToFirebase();
}

//Captures a new photo
void takePhoto() {
  capturePhotoSaveLittleFS();
    takeNewPhoto = false;
    isPhotoUploaded = false;
  
  //Sending takeNewPhoto as false to the RTDB
  if (Firebase.ready()) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setInt(&fbdo, "camera/bool", takeNewPhoto)) {
      Serial.println("Writing Successful!");
      Serial.println("Path: " + fbdo.dataPath());
      Serial.println("Data type: " + fbdo.dataType());
    } else {
      Serial.println("ERROR: " + fbdo.errorReason());
    }
  }
}

//Getting values from Firebase
void getFromRTDB() {
  if(Firebase.ready()) {
    if (Firebase.RTDB.getInt(&fbdo, "/camera/bool")) {
      Serial.println("Reading");
      if (fbdo.dataType() == "boolean") {
        Serial.println("it is bool");  //Checking for received data if it is bool
        takeNewPhoto = fbdo.boolData();
        Serial.println(takeNewPhoto);
      }
    } else {
      Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.getDouble(&fbdo, "/fence/latitude")) {
      Serial.println("Reading");
      if (fbdo.dataType() == "double") { //Checking for received data if it is Double
        fenceLatitude = fbdo.doubleData();
        Serial.println(fenceLatitude);
      }
    } else {
       Serial.println(fbdo.errorReason());
    }
    delay(1);

    if (Firebase.RTDB.getFloat(&fbdo, "/fence/longitude")) {
      Serial.println("Reading");
      if (fbdo.dataType() == "float") { //Checking for received data if it is Double
        fenceLongitude = fbdo.floatData();
        Serial.println(fenceLongitude);
      }
    } else {
        Serial.println(fbdo.errorReason());
    }

    if (Firebase.RTDB.getInt(&fbdo, "/fence/fenceRadius")) {
      Serial.println("Reading");
      if (fbdo.dataType() == "int") { //Checking for received data if it is Double
        fenceRadius = 1.0 * fbdo.intData();
        Serial.println(fenceRadius);
      }
    } else {
        Serial.println(fbdo.errorReason());
    }
  }
}

//The Firebase Storage upload callback function
void fcsUploadCallback(FCS_UploadStatusInfo info) {
  if (info.status == firebase_fcs_upload_status_init) {
    Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
  } else if (info.status == firebase_fcs_upload_status_upload) {
    Serial.printf("Uploaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
  } else if (info.status == firebase_fcs_upload_status_complete) {
    Serial.println("Upload completed\n");
    FileMetaInfo meta = fbdo.metaData();
    Serial.printf("Name: %s\n", meta.name.c_str());
    Serial.printf("Bucket: %s\n", meta.bucket.c_str());
    Serial.printf("contentType: %s\n", meta.contentType.c_str());
    Serial.printf("Size: %d\n", meta.size);
    Serial.printf("Generation: %lu\n", meta.generation);
    Serial.printf("Metageneration: %lu\n", meta.metageneration);
    Serial.printf("ETag: %s\n", meta.etag.c_str());
    Serial.printf("CRC32: %s\n", meta.crc32.c_str());
    Serial.printf("Tokens: %s\n", meta.downloadTokens.c_str());
    Serial.printf("Download URL: %s\n\n", fbdo.downloadURL().c_str());
  } else if (info.status == firebase_fcs_upload_status_error) {
    Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
  }
}

//Sending GPS values and isPetIn to the RTDB
void GPSToFirebase() {

  if (Firebase.ready()) {
    sendDataPrevMillis = millis();

    if (Firebase.RTDB.setDouble(&fbdo, "gps/latitude", latitude)) {
      Serial.println("Writing Successful!");
      Serial.println("Path: " + fbdo.dataPath());
      Serial.println("Data type: " + fbdo.dataType());
    } else {
      Serial.println("ERROR: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setDouble(&fbdo, "gps/longitude", longitude)) {
      Serial.println("Writing Successful!");
      Serial.println("Path: " + fbdo.dataPath());
      Serial.println("Data type: " + fbdo.dataType());
    } else {
      Serial.println("ERROR: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "fence/realDistance", realDistance)) {
      Serial.println("Writing Successful!");
      Serial.println("Path: " + fbdo.dataPath());
      Serial.println("Data type: " + fbdo.dataType());
    } else {
      Serial.println("ERROR: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setBool(&fbdo, "fence/isPetIn", isPetIn)) {
      Serial.println("Writing Successful!");
      Serial.println("Path: " + fbdo.dataPath());
      Serial.println("Data type: " + fbdo.dataType());
    } else {
      Serial.println("ERROR: " + fbdo.errorReason());
    }
    
  }
}

//Haversine formula for calculating distance in KM
static double haversine(double lat1, double lon1,
                        double lat2, double lon2)
    {
        // distance between latitudes
        // and longitudes
        double dLat = (lat2 - lat1) *
                      (M_PI / 180.0);
        double dLon = (lon2 - lon1) * 
                      M_PI / 180.0;
 
        // convert to radians
        lat1 = (lat1) * (M_PI / 180.0);
        lat2 = (lat2) * (M_PI / 180.0);
 
        // apply formulae
        double a = pow(sin(dLat / 2), 2) + 
                   pow(sin(dLon / 2), 2) * 
                   cos(lat1) * cos(lat2);
        double rad = 6371;
        double c = 2 * asin(sqrt(a));
        return rad * c;
    }

    
void setup() {
  // Serial port for debugging purposes
   Serial.begin(9600);
  delay(1);
  sim808.begin(9600);
  delay(1);
  //Set the sim808 by sending AT Commands
  setsim808();  //Only for First run. After that please comment this.

  initWiFi();
  initLittleFS();
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();
  Serial.println();
  Serial.print("Connect. IP Adress: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Firebase
  //Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  configF.database_url = DATABASE_URL;


  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h
  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
  if (Firebase.ready())
    Serial.println("Firebase ready!");
}

void loop() {
  loopTime = millis();
  delay(100);
  readsimPort();
  delay(100);
  readSerialPort();
  delay(100);
  if (msgToSIM != "") {
    Serial.print("Serial port : ");
    Serial.println(msgToSIM);
    sim808.print(msgToSIM);
    msgToSIM = "";
  }
  if (msgFrmSIM != "") {
    Serial.println("sim808 respond:  ");
    Serial.println(msgFrmSIM);
    msgFrmSIM = "";
  }

  //20 second interval for getting GPS data from module and other values from RTDB
  if (Firebase.ready() && (millis() - getDataPrevMillis > 20000 || getDataPrevMillis == 0)) {
    getDataPrevMillis = millis();

    //Getting GPS and sending it to the RTDB
    rdGPS();
    delay(1000);
    Serial.printf("latitude: %f\n", latitude);
    Serial.printf("longitude: %f\n", longitude);
    
    //Getting values from database
    getFromRTDB();
    
  }
  
  if (takeNewPhoto) {
    takePhoto();
  }
  delay(1);
  if (Firebase.ready() && !isPhotoUploaded) {
    Serial.print("Uploading picture... ");

    //MIME type should be valid to avoid the download problem.
    //The file systems for flash and SD/SDMMC can be changed in FirebaseFS.h.
    if (Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID /* Firebase Storage bucket id */, FILE_PHOTO_PATH /* path to local file */, mem_storage_type_flash /* memory storage type, mem_storage_type_flash and mem_storage_type_sd */, BUCKET_PHOTO /* path of remote file stored in the bucket */, "image/jpeg" /* mime type */, fcsUploadCallback)) {
      Serial.printf("\nDownload URL: %s\n", fbdo.downloadURL().c_str());
      isPhotoUploaded = true;
    } else {
      Serial.println(fbdo.errorReason());
    }
  }

  //Getting distance to the geofence in meters
  realDistance = haversine(fenceLatitude, fenceLongitude,latitude ,longitude );
  if(realDistance > fenceRadius) {
    isPetIn = false;
    Serial.println("real");
  } else {
    isPetIn = true;
    Serial.println("fence");
  }
}
