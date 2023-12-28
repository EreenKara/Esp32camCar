#include <ReactESP.h>


#include <WiFi.h>
#include "esp_camera.h"
#include <Firebase_ESP_Client.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


/*************************
Camera Configuration    */

#define CAMERA_MODEL_AI_THINKER

void cameraConfiguration(){

  
int PWDN_GPIO_NUM= 32;
int  RESET_GPIO_NUM= -1;
int  XCLK_GPIO_NUM= 0;
int  SIOD_GPIO_NUM =26;
int  SIOC_GPIO_NUM= 27;

int  Y9_GPIO_NUM= 35;
int  Y8_GPIO_NUM= 34;
int  Y7_GPIO_NUM =39;
int  Y6_GPIO_NUM= 36;
int  Y5_GPIO_NUM= 21;
int  Y4_GPIO_NUM= 19;
int  Y3_GPIO_NUM= 18;
int  Y2_GPIO_NUM= 5;
int  VSYNC_GPIO_NUM= 25;
int HREF_GPIO_NUM= 23;
int PCLK_GPIO_NUM =22;
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
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


  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;
  // if(psramFound()){
  //   config.frame_size = FRAMESIZE_UXGA;
  //   config.jpeg_quality = 10;
  //   config.fb_count = 2;
  // } else {
  //   config.frame_size = FRAMESIZE_SVGA;
  //   config.jpeg_quality = 12;
  //   config.fb_count = 1;
  // }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
    // delay(1000);

    // // ESP.restart();
  }


}
/* Camera Configuration
**************************/


/* FireBase 
***************/


FirebaseData fbdo;

  
FirebaseAuth auth;
FirebaseConfig config;
void firebaseAyarlamalar()
{

String API_KEY = "AIzaSyByWu65oLXCvsiv5GlWu0CrkLj7XCQgyfw";
String DATABASE_URL ="araba-e81cd-default-rtdb.europe-west1.firebasedatabase.app"; 
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if(Firebase.signUp(&config,&auth,"",""))
  {
    Serial.println("signup ok");
  }
  else{
    Serial.println("hata");
  }

  Firebase.begin(&config,&auth);
  Firebase.reconnectWiFi(true);

  config.token_status_callback=tokenStatusCallback;
}

/* Firebase
********************/




void WifiBaglan(){
  const char* ssid     = "ErenK";
  const char* password = "12321aab";
  WiFi.begin(ssid, password);
  if(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
}



/*******************
* Async ***********/
#include <ReactESP.h>

using namespace reactesp;
ReactESP app;



camera_fb_t * fb=NULL;
bool birinci=false;
bool ikinci=false;

void send1()
{
  Firebase.RTDB.setBlob( &fbdo,"/Fotograf", fb->buf,fb->len);
  birinci=true;
}

void send2(){
  Firebase.RTDB.setBlob( &fbdo,"/deneme", fb->buf,fb->len);
  ikinci=true;
}
/******Async*****
*****************/


void setup() {
  Serial.begin(115200);
  WifiBaglan();
  cameraConfiguration();
  firebaseAyarlamalar();
  app.onTick(send1);
  app.onTick(send2);
}

void loop() {
  birinci=false;
  ikinci=false;
  fb = esp_camera_fb_get();
  
  if(fb!=NULL){
    app.tick();
    while(true)
    {
      if(birinci && ikinci) break;
    }
  } 
  
  
  esp_camera_fb_return(fb);
}
