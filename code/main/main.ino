#include <WebServer.h>
#include <EEPROM.h>
#include<WiFi.h>
#include<FirebaseESP32.h>
#include "ADE7753.h"
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
WebServer webServer(80);
#define FIREBASE_HOST "https://json-fa813-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "WnBvsMeq0P34bK8NySYi88Q6F5WIDdqJeOa6hw7I"
FirebaseData firebaseData;
FirebaseJson json;
#define SETTING_HOLD_TIME 3000
unsigned long settingTimeout = 0;
char* ssid_ap = "ESP";
char* pass_ap = "12345678";
char* ssid;
char* pass;
String ssid1;
String pass1;
String ssid2;
String pass2;
int cs = 5;
int mosi = 23;
int miso = 19;
int sck = 18;
float v, v1, vol;
float i, i1, cur;
float vrms;
float irms;
ADE7753 met;
IPAddress ip_ap(192,168,1,2);
IPAddress gateway_ap(192,168,1,2);
IPAddress subnet_ap(255,255,255,0);

String path;
String data_from_firebase;
int led = 27;
int ledGreen = 32;
int ledRed = 33;
int button = 26 ;
int button1= 25;
int state = 0;
String k;
String ghet;
String setOn;
String timeOn;
int time_On;
String setOff;
String timeOff;
int time_Off;

//=========Biến chứa mã HTLM Website==//
const char MainPage[] PROGMEM = R"=====(
   <!DOCTYPE html> 
  <html>
   <head> 
       <title>ESP32</title> 
       <style> 
          body {text-align:center;background-color:#222222; color:white}
          input {
            height:25px; 
            width:270px;
            font-size:20px;
            margin: 10px auto;
          }
          #ledconnect{
              outline: none;
              margin: 0px 5px -1px 5px;
              width: 15px;
              height: 15px;
              border: solid 1px #00EE00;
              background-color: #00EE00;
              border-radius: 50%;
              -moz-border-radius: 50%;
              -webkit-border-radius: 50%;
          }

          .button_setup {
            height:30px; 
            width:280px; 
            margin:10px 0;
            background-color:#3C89BC;
            border-radius:10px;
            outline:none;
            color:white;
            font-size:20px;
            font-weight: bold;
          }
          .button_wifi{
            height:50px; 
            width:90px; 
            margin:10px 0;
            outline:none;
            color:white;
            font-size:15px;
            font-weight: bold;
          }
          #button_save {
            background-color:#00BB00;
            border-radius:5px;
          }
          #button_restart {
            background-color:#FF9900;
            border-radius:5px;
          }
          #button_reset {
            background-color:#CC3300;
            border-radius:5px;
          }
           #button_scan {
            background-color:#CC3300;
            border-radius:5px;
          }
       </style>
       <meta name="viewport" content="width=device-width,user-scalable=0" charset="UTF-8">
   </head>
   <body> 
      <div><h1>Web Server</h1></div>
      <div id="content"> 
        <div id="homecontrol" style="height:340px";>
          <div><input class="button_setup" type="button" onclick="configurewifi()" value="Configure WiFi"/></div>
          
        </div>
        
        <div id="wifisetup" style="height:340px; font-size:20px; display:none";>
        <div> <h2>WIFI</h2> </div>
        <div> <p> <pan id = "ssidAll"> </pan> </p><div> 


          <div><button id="button_scan" class="button_wifi" onclick="scan()">Scan</button></div>
          <div style="text-align:left; width:270px; margin:5px 25px">SSID: </div>
          <div><input id="ssid"/></div>
          <div style="text-align:left; width:270px; margin:5px 25px">Password: </div>
          <div><input id="pass"/></div>
          <div>
            <button id="button_save" class="button_wifi" onclick="writeEEPROM()">SAVE</button>
            <button id="button_restart" class="button_wifi" onclick="restartESP()">RESTART</button>
            <button id="button_reset" class="button_wifi" onclick="clearEEPROM()">RESET</button>
          </div>
          <div><input class="button_setup" type="button" onclick="backHOME()" value="Back home"/></div>
          <div id="reponsetext"></div>
        </div>
        
      </div>
      <script>
        //-----------Hàm khởi tạo đối tượng request----------------
        function create_obj(){
          td = navigator.appName;
          if(td == "Microsoft Internet Explorer"){
            obj = new ActiveXObject("Microsoft.XMLHTTP");
          }else{
            obj = new XMLHttpRequest();
          }
          return obj;
        }
        //------------Khởi tạo biến toàn cục-----------------------------
        var xhttp = create_obj();//Đối tượng request cho setup wifi
        var xhttp_statusD = create_obj();//Đối tượng request cho cập nhật trạng thái
        var d1,d2,d3,d4;
        var ledconnect = 1;
        //===================Khởi tạo ban đầu khi load trang=============
        window.onload = function(){
          document.getElementById("homecontrol").style.display = "block";
          document.getElementById("wifisetup").style.display = "none";
          getIPconnect();//Gửi yêu cầu lấy IP kết nối
          getstatusD();//Gửi yêu cầu lấy trạng thái các chân điều khiển
          scan();
        }
        //===================IPconnect====================================
        //--------Tạo request lấy địa chỉ IP kết nối----------------------
        function getIPconnect(){
          xhttp.open("GET","/getIP",true);
          xhttp.onreadystatechange = process_ip;//nhận reponse 
          xhttp.send();
        }
                //-----------Kiểm tra response IP và hiển thị------------------
        function process_ip(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            document.getElementById("ipconnected").innerHTML=ketqua;       
          }
        }
        function scan(){
          xhttp.open("GET", "/scan", true);
          xhttp.onreadystatechange = process_wifi;
          xhttp.send();
        }
        function process_wifi(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            ketqua = xhttp.responseText; 
            document.getElementById("ssidAll").innerHTML = ketqua;
          }
        }
        
        




        //===========Configure WiFi=====================================
        function configurewifi(){
          document.getElementById("homecontrol").style.display = "none";
          document.getElementById("wifisetup").style.display = "block";
        }
        //-----------Thiết lập dữ liệu và gửi request ssid và password---
        function writeEEPROM(){
          if(Empty(document.getElementById("ssid"), "Please enter ssid!")&&Empty(document.getElementById("pass"), "Please enter password")){
            var ssid = document.getElementById("ssid").value;
            var pass = document.getElementById("pass").value;
            xhttp.open("GET","/writeEEPROM?ssid="+ssid+"&pass="+pass,true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          }
        }
        function clearEEPROM(){
          
            xhttp.open("GET","/clearEEPROM",true);
            xhttp.onreadystatechange = process;//nhận reponse 
            xhttp.send();
          
        }
        function restartESP(){
          
            xhttp.open("GET","/restartESP",true);
            xhttp.send();
            
          
        }
        //-----------Kiểm tra response -------------------------------------------
        function process(){
          if(xhttp.readyState == 4 && xhttp.status == 200){
            //------Updat data sử dụng javascript----------
            ketqua = xhttp.responseText; 
            document.getElementById("reponsetext").innerHTML=ketqua;       
          }
        }
       //============Hàm thực hiện chứ năng khác================================
       //--------Cập nhật trạng thái tự động sau 2000 giây----------------------
        setInterval(function() {
          getstatusD();
        },2000);
       //--------Load lại trang để quay về Home control-------------------------
        function backHOME(){
          window.onload();
        }
       //----------------------------CHECK EMPTY--------------------------------
       function Empty(element, AlertMessage){
          if(element.value.trim()== ""){
            alert(AlertMessage);
            element.focus();
            return false;
          }else{
            return true;
          }
       }
       //------------------------------------------------------------------------
      </script>
   </body> 
  </html>
)=====";

void write_data_fb(int data_of_button){
  Firebase.setString(firebaseData,path + "/data",String(data_of_button));
  
  }
int read_data_fb(){
   Firebase.getString(firebaseData,path + "/data",k);
   if(k == "1"){return 1;}
   else {return 0;}
  }


void setup() {
  Serial.begin(9600);
  EEPROM.begin(512);       //Khởi tạo bộ nhớ EEPROM
  SPI.begin();
  pinMode(cs, OUTPUT);
  delay(10);
  met.Init(5, 4000000);
  met.gainSetup(0x80, 0x00, 0x00, 0x00);
  met.resetStatus();
  pinMode(led, OUTPUT);
  pinMode(button, INPUT_PULLUP);
  pinMode(button1, INPUT_PULLUP);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledRed,OUTPUT);
  
  if(read_EEPROM()){
    checkConnection();
  }else{
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
    WiFi.softAP(ssid_ap,pass_ap,1,false);
   // Serial.println("Soft Access Point mode!");
   // Serial.print("Please connect to");
   // Serial.println(ssid_ap);
   // Serial.print("Web Server IP Address: ");
   // Serial.println(ip_ap);
  }
  webServer.on("/",mainpage);
  webServer.on("/getIP",get_IP);
  webServer.on("/writeEEPROM",write_EEPROM);
  webServer.on("/restartESP",restart_ESP);
  webServer.on("/clearEEPROM",clear_EEPROM);
  webServer.on("/scan", scan);
  webServer.begin();


  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000*60);
  Firebase.setwriteSizeLimit(firebaseData, "tiny");
}

void loop() {
  webServer.handleClient();
  if(WiFi.status() == WL_CONNECTED){
  digitalWrite(ledGreen,1);
  digitalWrite(ledRed,0);
  if(digitalRead(button) == 1){
    delay(5);
    if(digitalRead(button) == 1){
      while(digitalRead(button) == 1);
      digitalWrite(led, !digitalRead(led));
      write_data_fb(digitalRead(led));
    }
  }
  else{
    digitalWrite(led, read_data_fb());
  }
 Firebase.getString(firebaseData,path +"/off/set",setOff);
    if(setOff == "1"){
      Firebase.getString(firebaseData,path +"/off/time",timeOff);
      time_Off = timeOff.toInt();
      digitalWrite(led,0);
      delay(time_Off * 1000);
      digitalWrite(led,1);
      
      Firebase.setString(firebaseData,path + "/off/set","0");
      Firebase.setString(firebaseData,path + "/off/time","0");
      Firebase.setString(firebaseData,path + "/data","1");
      }
  Firebase.getString(firebaseData,path +"/on/set",setOn);
    if(setOn == "1"){
      Firebase.getString(firebaseData,path +"/on/time",timeOn);
      time_On = timeOn.toInt();
      digitalWrite(led,1);
      delay(time_On * 1000);
      digitalWrite(led,0);
      
      Firebase.setString(firebaseData,path + "/on/set","0");
      Firebase.setString(firebaseData,path + "/on/time","0");
      Firebase.setString(firebaseData,path + "/data","0");
      }
  Firebase.getString(firebaseData,path + "/get",ghet);
    if(ghet == "1"){
      vrms = getVrms1();
      irms = getIrms1();

      Firebase.setString(firebaseData,path + "/Vrms",String(getVrms1()));
      Firebase.setString(firebaseData,path + "/Irms",String(getIrms1()));
      Firebase.setString(firebaseData,path + "/get","0");
    }
}
if(WiFi.status() != WL_CONNECTED){
  digitalWrite(ledRed,1);
  digitalWrite(ledGreen,0);
  if(digitalRead(button) == 1){
    delay(5);
    if(digitalRead(button) == 1){
      while(digitalRead(button) == 1);
      digitalWrite(led, !digitalRead(led));
    }
  }
}
if(digitalRead(button1) == HIGH && (settingTimeout + SETTING_HOLD_TIME) <= millis()){
    blynk();
    clear_EEPROM();
    settingTimeout = millis();
    if(read_EEPROM()){
      checkConnection();
    }else{
      WiFi.disconnect();
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
      WiFi.softAP(ssid_ap,pass_ap,1,false);
     // Serial.println("Soft Access Point mode!");
     // Serial.print("Please connect to");
    //  Serial.println(ssid_ap);
     // Serial.print("Web Server IP Address: ");
     // Serial.println(ip_ap);
    }
    }else if(digitalRead(button1) == LOW){
        settingTimeout = millis();
    }
}

void mainpage(){
  String s = FPSTR(MainPage);
  webServer.send(200,"text/html",s);
}

boolean read_EEPROM(){
  Serial.println("Reading EEPROM...");
  if(EEPROM.read(0)!=0){
    ssid2 = "";
    pass2 = "";
    for (int i=0; i<32; ++i){
      ssid2 += char(EEPROM.read(i));  
    }
    Serial.print("SSID: ");
    Serial.println(ssid2);
    for (int i=32; i<96; ++i){
      pass2 += char(EEPROM.read(i));  
    }
    Serial.print("PASSWORD: ");
    Serial.println(pass2);
  //  ssid2 = ssid2.c_str();
  //  pass2 = pass2.c_str();
    ssid = new char[ssid2.length()+1];
    strcpy(ssid, ssid2.c_str());
    pass = new char[pass2.length()+1];
    strcpy(pass, pass2.c_str());
    return true;
  }else{
    Serial.println("Data wifi not found!");
    return false;
  }
}
//---------------SETUP WIFI------------------------------
void checkConnection() {
  Serial.println();
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.print("Check connecting to ");
  Serial.println(ssid2);
  WiFi.begin(ssid,pass);
  int count=0;
  while(count < 50){
    if(WiFi.status() == WL_CONNECTED){
      Serial.println();
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("Web Server IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    }
    delay(200);
    Serial.print(".");
    count++;
  }
  Serial.println("Timed out.");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ip_ap, gateway_ap, subnet_ap);
  WiFi.softAP(ssid_ap,pass_ap,1,false);
  Serial.println("Soft Access Point mode!");
  Serial.print("Please connect to ");
  Serial.println(ssid_ap);
  Serial.print("Web Server IP Address: ");
  Serial.println(ip_ap);
}
void write_EEPROM(){
  ssid1 = webServer.arg("ssid");
  pass1 = webServer.arg("pass");
  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 96; ++i) {
    EEPROM.write(i, 0);           
    delay(10);
  }
  for (int i = 0; i < ssid1.length(); ++i) {
    EEPROM.write(i, ssid1[i]);
  }
  for (int i = 0; i < pass1.length(); ++i) {
    EEPROM.write(32 + i, pass1[i]);
  }
  EEPROM.commit();
  Serial.println("Writed to EEPROM!");
  Serial.print("SSID: ");
  Serial.println(ssid1);
  Serial.print("PASS: ");
  Serial.println(pass1);
  String s = "Wifi configuration saved!";
  webServer.send(200, "text/html", s);
}
void restart_ESP(){
  String s = "Restarted!";
  webServer.send(200, "text/html", s);
  ESP.restart();
}
void clear_EEPROM(){
  Serial.println("Clear EEPROM!");
  for (int i = 0; i < 512; ++i) {
    EEPROM.write(i, 0);           
    delay(10);
  }
  EEPROM.commit();
  String s = "Device has been reset!";
  webServer.send(200,"text/html", s);
  
}
int covert_dBm_percent(int dbm){
  if(dbm < -100){
    return 0;
    }
  if(dbm > -50){
    return 100;
    }
    else{
      return 2* (dbm +100);
      }
  }
void scan(){
    
    int a = WiFi.scanNetworks();
    String ssid0 = WiFi.SSID(0);
    String rssi0 = String(covert_dBm_percent(WiFi.RSSI(0)));
    String ssid1 = WiFi.SSID(1);
    String rssi1 = String(covert_dBm_percent(WiFi.RSSI(1)));
    String ssid2 = WiFi.SSID(2);
    String rssi2 = String(covert_dBm_percent(WiFi.RSSI(2)));
    String ssid3 = WiFi.SSID(3);
    String rssi3 = String(covert_dBm_percent(WiFi.RSSI(3)));
    String ssid4 = WiFi.SSID(4);
    String rssi4 = String(covert_dBm_percent(WiFi.RSSI(4)));
    
    String ssidAll = "SSID1:" + ssid0 + "("+rssi0 +"%"+")" +"\n"+"\n"+ "SSID2:" + ssid1 +"("+rssi1+ "%"+")"+ "\n" + "SSID3:" + ssid2+"("+rssi2 +"%"+")" +"\n"+"SSID4:" + ssid3+"("+rssi3+ "%"+")" +"\n"+ "SSID5:" + ssid4+"("+rssi4+ "%"+")";
    webServer.send(200,"text/html", ssidAll);

    
  }
  void get_IP(){
  String s = WiFi.localIP().toString();
  webServer.send(200,"text/html", s);
}
float getVrms1(){
   v=  (float)met.vrms();
 v1 = 220.00/19.00 * 48 * (0.5/0x17D338);
 vol = v * v1;
return vol;

  }
 float getIrms1(){
   i=  (float)met.irms();
 i1 = (0.5 / 0x1C82B3) * i * 2.00 / 3.00;
 return i1;
  }
  void blynk(){
    for(int i = 0; i < 10; i++){
    digitalWrite(ledRed, 1);
    delay(50);
    digitalWrite(ledRed, 0);
    delay(50);
    }
    }
