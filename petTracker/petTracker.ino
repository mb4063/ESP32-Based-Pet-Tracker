

//Libraries
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include "WiFi.h"
//Tokens for RTDB connection
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"



//Network credentials
const char* ssid = "El Gato(2.4)";
const char* password = "";

//Firebase project api key
#define API_KEY ""

//Authortized Email and password
#define USER_EMAIL "mehmetbulut4063@outlook.com"
#define USER_PASSWORD ""

//Firebase storage bucket id
#define STORAGE_BUCKET_ID "gs://pet-tracker-e6c45.appspot.com"

//Database url
#define DATABASE_URL "https://pet-tracker-e6c45-default-rtdb.europe-west1.firebasedatabase.app/"

//Firebase data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;
unsigned long sendDataPrevMillis = 0;
unsigned long getDataPrevMillis = 0;
bool signupOK = false;
int intValue=0;
int count = 1;

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}
 

void setup() {
 // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
  Serial.println();
  Serial.print("Connect. IP Adress: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  
  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  configF.database_url = DATABASE_URL;

  //Sign up to Firebase
  if (Firebase.signUp(&configF, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", configF.signer.signupError.message.c_str());
  }
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
}
void loop() {
  // veritabanındaki  test/int  tablo/veri hücresine veri yazalım
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)) {
      Serial.println("YAZMA TAMAM");
      Serial.println("DİZİN: " + fbdo.dataPath());
      Serial.println("VERİ TİPİ: " + fbdo.dataType());
    }
    else {
      Serial.println("HATA");
      Serial.println("HATA SEBEBİ: " + fbdo.errorReason());
    }
    count++;

    

  } 
 // Database testing
 if (Firebase.ready() && signupOK && (millis() - getDataPrevMillis > 16000 || getDataPrevMillis == 0)) {
   getDataPrevMillis = millis();
   if (Firebase.RTDB.getInt(&fbdo, "/test/int")) {
      if (fbdo.dataType() == "int") { //Checking for received data if it is int
        intValue = fbdo.intData();
        Serial.println(intValue);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
 }

}
