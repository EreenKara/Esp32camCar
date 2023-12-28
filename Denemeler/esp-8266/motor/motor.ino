#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <iostream>
#include <sstream>
#include <string.h>

/***************
Web Server    */

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

/* Web Server
****************/

/* FireBase 
***************/

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyByWu65oLXCvsiv5GlWu0CrkLj7XCQgyfw"
#define DATABASE_URL "araba-e81cd-default-rtdb.europe-west1.firebasedatabase.app" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* Firebase
********************/

struct MOTOR_PINS
{
  int pinEn;  
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {2, 5, 4},  //RIGHT_MOTOR Pins (EnA, IN1, IN2)
  {2, 14, 12},  //LEFT_MOTOR  Pins (EnB, IN3, IN4)
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
  .keys {
      text-align: center;
      width: 400px;
      height: 200px;
      margin: auto;
    }
  .arr {
  cursor: pointer;
  width: 100px;
  height: 100px;
  text-align: center;
  line-height: 100px;
  background: grey;
  color: white;
  font-size: 50px;
  border-right: 10px solid darken(grey, 20);
  border-bottom: 10px solid darken(grey, 20);
  border-left: 10px solid darken(grey, 15);
  border-top: 10px solid darken(grey, 10);  
  display: inline-block;
  margin: 5px;
  transition: all .05s linear;
  user-select: none;
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

.form-div
{
    width: 50%;
    margin:0 auto;
    margin-top: 300px;
}
form{
    display: flex;
    flex-direction: column;
    justify-content: center;
}
    </style>
  
  </head>
  <body class="noselect" align="center" style="background-color:white">
     
    <!--h2 style="color: teal;text-align:center;">Wi-Fi Camera &#128663; Control</h2-->
    
    <div id="mainTable">
        <div class="fotograf"><img id="cameraImage" src="" style="width:400px;height:250px"></div>
        <div class="keys">
            <div id="up" class="up arr" onmousedown='sendButtonInput("MoveCar","1")' onmouseup='sendButtonInput("MoveCar","0")' ontouchstart='sendButtonInput("MoveCar","1")' ontouchend='sendButtonInput("MoveCar","0")'><i class="fa fa-arrow-up"></i></div>
            <br />
            <div id="left" class="left arr" ontouchstart='sendButtonInput("MoveCar","3")' ontouchend='sendButtonInput("MoveCar","0")'><i class="fa fa-arrow-left"></i></div>  
            <div id="down" class="down arr" ontouchstart='sendButtonInput("MoveCar","2")' ontouchend='sendButtonInput("MoveCar","0")'><i class="fa fa-arrow-down"></i></div>
            <div id="right" class="right arr" ontouchstart='sendButtonInput("MoveCar","4")' ontouchend='sendButtonInput("MoveCar","0")'><i class="fa fa-arrow-right"></i></div>
        </div>
        <div class="sliderlar">
            <div class="slidecontainer">
                <input type="range" min="0" max="255" value="150" class="slider" id="Speed" oninput='sendButtonInput("Speed",value)'>
            </div>
            <div class="slidecontainer">
                <input type="range" min="0" max="255" value="0" class="slider" id="Light" oninput='sendButtonInput("Light",value)'>
            </div>
        </div>
    </div>
    <div class="form-div">
      <form action="/changeWifi" method="get">
          <label>Wifi: <input type="text" name="ssid"></label>
          <label>Password: <input type="text" name="password"></label>
          <button style="width: 80px;" type="submit">Bağlan</button>
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
      // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      // Serial.printf("WebSocket client #%u disconnected\n", client->id());
      moveCar(0);
      // ledcWrite(PWMLightChannel, 0);  
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
          analogWrite(2, valueInt);
        }
        else if (key == "Light")
        {
          // ledcWrite(PWMLightChannel, valueInt);         
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
      // Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      cameraClientId = client->id();
      break;
    case WS_EVT_DISCONNECT:
      // Serial.printf("WebSocket client #%u disconnected\n", client->id());
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
// void sendCameraPicture()
// {
//   if (cameraClientId == 0)
//   {
//     return;
//   }
//   unsigned long  startTime1 = millis();
//   //capture a frame
//   camera_fb_t * fb = esp_camera_fb_get();
//   if (!fb) 
//   {
//       Serial.println("Frame buffer could not be acquired");
//       return;
//   }

//   unsigned long  startTime2 = millis();
//   wsCamera.binary(cameraClientId, fb->buf, fb->len);



//   esp_camera_fb_return(fb);
    
//   //Wait for message to be delivered
//   while (true)
//   {
//     AsyncWebSocketClient * clientPointer = wsCamera.client(cameraClientId);
//     if (!clientPointer || !(clientPointer->queueIsFull()))
//     {
//       break;
//     }
//     delay(2);
//   }
  
//   unsigned long  startTime3 = millis();  
//   Serial.printf("Time taken Total: %d|%d|%d\n",startTime3 - startTime1, startTime2 - startTime1, startTime3-startTime2 );
// }

void setUpPinModes()
{
      
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinEn, OUTPUT);    
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);  
  }

  moveCar(STOP);
}
const char* takePictureFromSerial()
{
  const char* fotograf=NULL;
  if(Serial.available()) 
  {
    fotograf = Serial.readString().c_str();
    Serial.println("Tamam");
  }
  return fotograf;
}

void sendPictureToFirebase(const char* fotograf){  // buraya GPS verisi de ilave edilecek
  if(fotograf!=NULL)
  {
    FirebaseJson json;
    json.set("Fotograf",fotograf);
    json.set("GPS",0);
    Firebase.RTDB.setJSON( &fbdo,"/KonumveFotograf", &json);
  }

}

// void reconnectAnotherWifi(AsyncWebServerRequest *request){
//   const char * ssid;
//   const char * password;
//   AsyncWebParameter* p1 = request->getParam("ssid");
//   ssid=p1->value().c_str();
//   Serial.println(ssid);
//   AsyncWebParameter* p2 = request->getParam("password");
//   password=p2->value().c_str();
//   Serial.println(password);
  
//   // WiFi.disconnect();
//   // WiFi.begin(ssid,password);
//   WiFi.begin("bilgisayar","5:b9L602");

//   request->redirect("/");
// }

void firebaseAyarlamalar()
{
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

void serverBaslat()
{
  const char *apssid = "Araba";
  const char *appassword = "12345678";

  WiFi.softAP(apssid, appassword);

  server.on("/", HTTP_GET, handleRoot);
  // server.on("/changeWifi", HTTP_GET, reconnectAnotherWifi);
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
  delay(4000);
}

void setup(void) 
{
  Serial.begin(115200);
  WifiBaglan();
  setUpPinModes();
  Serial.println("Tamam");
  serverBaslat();
  // firebaseAyarlamalar();
  

}

void loop() 
{
  wsCamera.cleanupClients(); 
  wsCarInput.cleanupClients(); 

  // sendPictureToFirebase(takePictureFromSerial());

}