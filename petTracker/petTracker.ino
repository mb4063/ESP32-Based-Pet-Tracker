

//Libraries
#include <Firebase_ESP_Client.h>
#include "WiFi.h"
#include <addons/TokenHelper.h>



//Network credentials
const char* ssid = "";
const char* password = "";

//Firebase project api key
#define API_KEY ""

//Authortized Email and password
#define USER_EMAIL ""
#define USER_PASSWORD ""

//Firebase storage bucket id
#define STORAGE_BUCKET_ID "REPLACE_WITH_YOUR_STORAGE_BUCKET_ID"

//Firebase data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}
 // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();

  
  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);

void setup() {
 
}
void loop() {
  // put your main code here, to run repeatedly:

}
