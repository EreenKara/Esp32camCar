#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
// #include "Base64.h"
#include "esp_camera.h"
#include <sstream>
#include <WiFi.h>
#include <stdlib.h>
#include <pthread.h>


/**********************
******Web Server******/

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


AsyncWebServer server(80);
AsyncWebSocket wsCamera("/Camera");
AsyncWebSocket wsCarInput("/CarInput");
uint32_t cameraClientId = 0;



const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
    .arrows {
      font-size:40px;
      color:red;
    }
    td.button {
      background-color:black;
      border-radius:25%;
      box-shadow: 5px 5px #888888;
    }
    td.button:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }

    .noselect {
      -webkit-touch-callout: none; /* iOS Safari */
        -webkit-user-select: none; /* Safari */
         -khtml-user-select: none; /* Konqueror HTML */
           -moz-user-select: none; /* Firefox */
            -ms-user-select: none; /* Internet Explorer/Edge */
                user-select: none; /* Non-prefixed version, currently
                                      supported by Chrome and Opera */
    }

    .slidecontainer {
      width: 100%;
    }

    .slider {
      -webkit-appearance: none;
      width: 100%;
      height: 15px;
      border-radius: 5px;
      background: #d3d3d3;
      outline: none;
      opacity: 0.7;
      -webkit-transition: .2s;
      transition: opacity .2s;
    }

    .slider:hover {
      opacity: 1;
    }
  
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }

    .slider::-moz-range-thumb {
      width: 25px;
      height: 25px;
      border-radius: 50%;
      background: red;
      cursor: pointer;
    }
  .form-div
  {
      width: 50%;
      margin:0 auto;
      margin-top: 300px;
  }
    </style>
  
  </head>
  <body class="noselect" align="center" style="background-color:white">
     
    <!--h2 style="color: teal;text-align:center;">Wi-Fi Camera &#128663; Control</h2-->
    
    <table id="mainTable" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr>
        <td id=fotograf><img id="cameraImage" src="" style="width:400px;height:250px"></td>
      </tr> 
      <tr>
        <td></td>
        <td class="button" onmousedown='sendButtonInput("MoveCar","4")' onmouseup='sendButtonInput("MoveCar","0")' ontouchstart='sendButtonInput("MoveCar","1")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8679;</span></td>
        <td></td>
      </tr>
      <tr>
        <td class="button" onmousedown='sendButtonInput("MoveCar","2")' onmouseup='sendButtonInput("MoveCar","0")' ontouchstart='sendButtonInput("MoveCar","3")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8678;</span></td>
        <td class="button"></td>    
        <td class="button" onmousedown='sendButtonInput("MoveCar","1")' onmouseup='sendButtonInput("MoveCar","0")'  ontouchstart='sendButtonInput("MoveCar","4")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8680;</span></td>
      </tr>
      <tr>
        <td></td>
        <td class="button" onmousedown='sendButtonInput("MoveCar","3")' onmouseup='sendButtonInput("MoveCar","0")' ontouchstart='sendButtonInput("MoveCar","2")' ontouchend='sendButtonInput("MoveCar","0")'><span class="arrows" >&#8681;</span></td>
        <td></td>
      </tr>
      <tr/><tr/>
      <tr>
        <td style="text-align:left"><b>Speed:</b></td>
        <td colspan=2>
         <div class="slidecontainer">
            <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendButtonInput("Speed",value)'>
          </div>
        </td>
      </tr>        
      <tr>
        <td style="text-align:left"><b>Light:</b></td>
        <td colspan=2>
          <div class="slidecontainer">
            <input type="range" min="0" max="255" value="0" class="slider" id="Light" oninput='sendButtonInput("Light",value)'>
          </div>
        </td>   
      </tr>
    </table>
    <div class="form-div">
      <form action="/changeWifi" method="get">
          <label>Wifi: <input type="text" name="ssid"></label>
          <label>Password: <input type="text" name="password"></label>
          <button style="width: 80px;" type="submit">Baglan</button>
      </form>
    </div>
    <script>
      var webSocketCameraUrl = "ws:\/\/" + window.location.hostname + "/Camera";
      var webSocketCarInputUrl = "ws:\/\/" + window.location.hostname + "/CarInput";      
      var websocketCamera;
      var websocketCarInput;
      
      function initCameraWebSocket() 
      {
        websocketCamera = new WebSocket(webSocketCameraUrl);
        websocketCamera.binaryType = 'blob';
        websocketCamera.onopen    = function(event){};
        websocketCamera.onclose   = function(event){setTimeout(initCameraWebSocket, 2000);};
        websocketCamera.onmessage = function(event)
        {
          var imageId = document.getElementById("cameraImage");
          imageId.src = URL.createObjectURL(event.data);
        };
      }
      
      function initCarInputWebSocket() 
      {
        websocketCarInput = new WebSocket(webSocketCarInputUrl);
        websocketCarInput.onopen    = function(event)
        {
          var speedButton = document.getElementById("Speed");
          sendButtonInput("Speed", speedButton.value);
          var lightButton = document.getElementById("Light");
          sendButtonInput("Light", lightButton.value);
        };
        websocketCarInput.onclose   = function(event){setTimeout(initCarInputWebSocket, 2000);};
        websocketCarInput.onmessage = function(event){};        
      }
      
      function initWebSocket() 
      {
        initCameraWebSocket ();
        initCarInputWebSocket();
      }

      function sendButtonInput(key, value) 
      {
        var data = key + "," + value;
        websocketCarInput.send(data);
      }
    
      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      
    </script>
  </body>    
</html>
)HTMLHOMEPAGE";


void rotateMotor(int motorNumber, int motorDirection);
void moveCar(int inputValue);


void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}

void onCarInputWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      moveCar(0);
      ledcWrite(3, 0);  
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        std::istringstream ss(myData);
        std::string key, value;
        std::getline(ss, key, ',');
        std::getline(ss, value, ',');
        Serial.printf("Key [%s] Value[%s]\n", key.c_str(), value.c_str()); 
        int valueInt = atoi(value.c_str());     
        if (key == "MoveCar")
        {
          moveCar(valueInt);        
        }
        else if (key == "Speed")
        {
          // Speeed channel => 2
          ledcWrite(2, valueInt);
        }
        else if (key == "Light")
        {
          // light Channel =>3
          ledcWrite(3, valueInt);         
        }     
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void onCameraWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      cameraClientId = 0;
      break;
    case WS_EVT_DATA:
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}


/* Web Server
****************/


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


/**********************
**       Motor     ***/

struct MOTOR_PINS
{
  int pinEn;  
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {12, 13, 15},  //RIGHT_MOTOR Pins (EnA, IN1, IN2)
  {12, 14, 2},  //LEFT_MOTOR  Pins (EnB, IN3, IN4)
};
#define LIGHT_PIN 4 

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define STOP 0

#define RIGHT_MOTOR 0
#define LEFT_MOTOR 1

#define FORWARD 1
#define BACKWARD -1


void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);     
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}

void moveCar(int inputValue)
{
  Serial.printf("Got value as %d\n", inputValue);  
  switch(inputValue)
  {

    case UP:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, FORWARD);     
      break;
  
    case DOWN:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case LEFT:
      rotateMotor(RIGHT_MOTOR, FORWARD);
      rotateMotor(LEFT_MOTOR, BACKWARD);  
      break;
  
    case RIGHT:
      rotateMotor(RIGHT_MOTOR, BACKWARD);
      rotateMotor(LEFT_MOTOR, FORWARD); 
      break;
 
    case STOP:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;
  
    default:
      rotateMotor(RIGHT_MOTOR, STOP);
      rotateMotor(LEFT_MOTOR, STOP);    
      break;
  }
}



void setUpPinModes()
{
  const int PWMFreq = 1000; /* 1 KHz */
  const int PWMResolution = 8;
  const int PWMSpeedChannel = 2;
  const int PWMLightChannel = 3;
  //Set up PWM
  ledcSetup(PWMSpeedChannel, PWMFreq, PWMResolution);
  ledcSetup(PWMLightChannel, PWMFreq, PWMResolution);
      
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinEn, OUTPUT);    
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  

    /* Attach the PWM Channel to the motor enb Pin */
    ledcAttachPin(motorPins[i].pinEn, PWMSpeedChannel);
  }
  moveCar(STOP);

  pinMode(LIGHT_PIN, OUTPUT);    
  ledcAttachPin(LIGHT_PIN, PWMLightChannel);
}


/* Motor     ******
******************/





/* FireBase 
***************/

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


void firebaseAyarlamalar()
{

  String API_KEY = "AIzaSyByWu65oLXCvsiv5GlWu0CrkLj7XCQgyfw";
  String DATABASE_URL = "araba-e81cd-default-rtdb.europe-west1.firebasedatabase.app" ;
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


/*******************
* Async ***********/
// #include <ReactESP.h>

// using namespace reactesp;
// ReactESP app;




/******Async*****
*****************/

void reconnectAnotherWifi(AsyncWebServerRequest *request){
  const char * ssid;
  const char * password;
  AsyncWebParameter* p1 = request->getParam("ssid");
  ssid=p1->value().c_str();
  Serial.print(ssid);
  Serial.print(" ");
  AsyncWebParameter* p2 = request->getParam("password");
  password=p2->value().c_str();
  Serial.println(password);
  
  WiFi.disconnect();
  delay(5);
  WiFi.begin(ssid,password);
  // WiFi.begin("bilgisayar","5:b9L602");

  request->redirect("/");
}



// void* serveraYolla(void * params){
//   if(cameraClientId != 0){

//     wsCamera.binary(cameraClientId, fb->buf, fb->len);
//     pthread_exit(0);
//   }

//     pthread_exit(0);
// }
camera_fb_t * fb=NULL;


// asenkron calisan fonksiyon
// void* firebaseYolla(void * params){
//   // if(WiFi.status() != WL_CONNECTED){

//   //   pthread_exit(0);
//   //   return NULL;
//   // }
//   if(!fb)
//   {
//     Serial.println("Null geliyor buraya");
//     pthread_exit(0);
//     return NULL;

//   }
//   Firebase.RTDB.setBlob( &fbdo,"/Fotograf", fb->buf,fb->len);

//   pthread_exit(0);

// }


void serverBaslat()
{
  const char *apssid = "ErensAraba";
  const char *appassword = "eren123456";

  WiFi.softAP(apssid, appassword);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/changeWifi", HTTP_GET, reconnectAnotherWifi);
  server.onNotFound(handleNotFound);

  wsCamera.onEvent(onCameraWebSocketEvent);
  server.addHandler(&wsCamera);

  wsCarInput.onEvent(onCarInputWebSocketEvent);
  server.addHandler(&wsCarInput);

  server.begin();
}



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

int sayac=0;

pthread_t firebaseThread;
pthread_t serverThread;
pthread_attr_t attr;


void setup() {
  Serial.begin(115200);
  WifiBaglan();
  setUpPinModes();
  cameraConfiguration();
  serverBaslat();
  firebaseAyarlamalar();
  // app.onTick(serveraYolla);
  // app.onTick(firebaseYolla);
  pthread_attr_init(&attr);
  
}

void loop() {
  wsCamera.cleanupClients(); 
  wsCarInput.cleanupClients(); 
  
  if(cameraClientId==0)
  {
    return;
  }
  
  fb = esp_camera_fb_get();
  if(!fb){
    return;
  } 
  if(sayac==60){
    // ((camera_fb_t*)params)
    wsCamera.binary(cameraClientId, fb->buf, fb->len);
    Firebase.RTDB.setBlob( &fbdo,"/Fotograf", fb->buf,fb->len);
    // pthread_create(&firebaseThread,&attr,firebaseYolla,NULL);
    // pthread_join(firebaseThread,NULL);
    sayac=0;
  }
  else{
    wsCamera.binary(cameraClientId, fb->buf, fb->len);
    while (true)
    {
      AsyncWebSocketClient * clientPointer = wsCamera.client(cameraClientId);
      if (!clientPointer || !(clientPointer->queueIsFull()))
      {
        break;
      }
      delay(1);
    }
  }
  esp_camera_fb_return(fb);
  sayac++;


}
