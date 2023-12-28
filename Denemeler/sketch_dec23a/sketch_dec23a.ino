
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


int deger=0;
const char *ssid = "ErenK";
const char *password = "12321aab";

#define USER_EMAIL "errennkaaraa@hotmail.com"
#define USER_PASSWORD "123123arabA"
#define STORAGE_BUCKET_ID "araba-e81cd.appspot.com"
String FIREBASE_AUTH = "cTJHF2Aqs0YzVk2CA0C8drrGedFqyqgLH57Ct1Qx";

#define API_KEY "AIzaSyByWu65oLXCvsiv5GlWu0CrkLj7XCQgyfw"
#define DATABASE_URL "araba-e81cd-default-rtdb.europe-west1.firebasedatabase.app" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void firebaseAyarlamalar(){

// WiFi.begin(ssid, password);
//     Serial.println();

//   while (WiFi.status() != WL_CONNECTED) {
//     Serial.printf(".");
//     delay(2000);
//     // if ((StartTime+10000) < millis()) break;
//   }

  // config.database_url = DATABASE_URL;
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if(Firebase.signUp(&config,&auth,"",""))
  {
    Serial.println("signup ok");
  }
  else{
    Serial.println("hata");
  }
  
  
  // auth.user.email = USER_EMAIL;
  // auth.user.password = USER_PASSWORD;
  
  
  
  
  
  Firebase.begin(&config,&auth);
  Firebase.reconnectWiFi(true);

  config.token_status_callback=tokenStatusCallback;



  // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  // Firebase.reconnectWiFi(true);
  // Firebase.setMaxRetry(firebaseData, 3);
  // Firebase.setMaxErrorQueue(firebaseData, 30);
  // Firebase.enableClassicRequest(firebaseData, true);
  Serial.println("Ayarlar tamam");

}

// void resimGonder(){
//   String resim=Photo2Base64();
//   String jsonData = "{\"photo\":\"" + resim + "\"}";
//   String photoPath = "/aracResim";
//   // Firebase.setString(firebaseData, photoPath, resim);
//   FirebaseJson json2;
//   json2.set("photo",resim);
//   Firebase.setJson((firebaseData, photoPath, json2))
// }

void setup() {

  Serial.begin(115200);
  firebaseAyarlamalar();
}


void loop() {
  // if (Serial.available()) Serial.println(Serial.readString());
  // delay(100);
  deger++;
  Serial.println("VeriGönderiecek");

  Firebase.RTDB.setString( &fbdo,"/Veri", "merhaba"+);
  Serial.println("VeriGönderildi");
  delay(4000);
}
