/*************************************************** 
  This is an example sketch for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit BMP085 Breakout 
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to 
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#define SS_PIN D4
#define RST_PIN D2
WiFiClient wificlient;
#include <Adafruit_Fingerprint.h>

// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
// uncomment this line:
// #define mySerial Serial1

// For UNO and others without hardware serial, we must use software serial...
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
// comment these two lines if using hardware serial
//#define Finger_Rx 14 //D5
//#define Finger_Tx 12 //D6
#define Finger_Rx 5 //D1
#define Finger_Tx 4 //D2
SoftwareSerial mySerial(Finger_Rx, Finger_Tx);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int level_fingerprintregister=0;// not registered/registered
//GET ID IN DATABASE;
int fingerp_id = 0;
//IF SUCCESS DELETE 1 IF NOT = 0
int delete_status=0;
//if finger detected and matched 1 else 0
int finger_detected=0;


const char *ssid = "MITSI-Admin"; //WIFI NAME OR HOTSPOT
const char *password = "M@ssiv3its_2017"; //WIFI PASSWORD POR MOBILE HOTSPOT PASSWORD

MFRC522 mfrc522(SS_PIN, RST_PIN);
void setup() {
   delay(1000);
   Serial.begin(9600);
   finger.begin(57600);//start fingerprint
   WiFi.mode(WIFI_OFF);    
   delay(1000);
   WiFi.mode(WIFI_STA);
   WiFi.begin(ssid, password);
   Serial.println("");

   Serial.print("Connecting");
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
   Serial.println("");
   Serial.print("Connected to ");
   Serial.println(ssid);
   Serial.print("IP address: ");
   Serial.println(WiFi.localIP()); 
   if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
}
//////////////////REGISTER FINGERPRINT YES/NO///////////////////
int register_fingerprint() {//ready=1 standby=0
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    String postData = "action=register_fingerprint";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/read.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode > 0){
      if(payload=="ready"){
        return 1;
      }
      return 0;
    }
    http.end();
  }
  return 0;
}
///////////know if successfully registered the finger////////////
void successfully_registeredf(){
  level_fingerprintregister++;
}
void reset_counterlevel(){
  level_fingerprintregister=level_fingerprintregister*0;
  delete_status=delete_status*0;
  finger_detected=finger_detected*0;
}
///////////////////////////////////////////
///////////////Insert FingerprintDB/////////////////
void insert_fingerprint() {
  if(WiFi.status() == WL_CONNECTED) {
    while(level_fingerprintregister==0){
      getFingerprintEnroll(fingerp_id);
    }
    Serial.println("Successfully enrolled the finger");
    reset_counterlevel();
  }
}
////////////////////////Identify FINGER ID////////////////////
void verify_fingerid(){
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    String postData = "action=get_fingerprintid";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/read.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode > 0){
      Serial.println("Finger ID Selected: "+payload);
      fingerp_id = payload.toInt();
    }
    http.end();
  }
}
////////////////////delete fingerprint/////////////
uint8_t deleteFingerprint(int id) {
  uint8_t p = -1;
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    delete_status = delete_status+1;
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }   
}
////////////////signal off/////////////////////////
void finger_registeroff(){
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    String postData = "action=register_off";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/read.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode <= 0){
      Serial.println("ERROR");
    }
    http.end();
  }
}
//////////////////////////////////////////


////////////GET FINGERPRINT ENROLL////////////////////
uint8_t getFingerprintEnroll(int id) {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    successfully_registeredf();
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}

///////////////////////////////////////////////
//////////////Delete Fingerprint Y/N///////////
int delete_fingerprint(){
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    String postData = "action=delete_fingerprint";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/reset.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode > 0){
      if(payload=="yes"){
        return 1;
      }
      return 0;
    }
    http.end();
  }
  return 0;
}

///////////////////////////////////////////////
////////////DELETE FINGER ID SELECTED//////////
int delete_fingerid(){
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    String postData = "action=get_fingerprintid";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/reset.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode > 0){
      Serial.println("Finger ID to be deleted: "+payload);
      int fingerdelete_id = payload.toInt();
      return fingerdelete_id;
    }
    http.end();
    return 0;
  }
  return 0;
}
//////////////////////////////////////////////
/////////////Delete Fingerprint//////////////
void delete_fingerprint(int id){
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    while(delete_status==0){ //1 is success
      deleteFingerprint(id);
    }
    Serial.println("Successfully Deleted");
    reset_counterlevel();
    http.end();
  }
}
////////////////////////////////////////////
/////////////Finger Search/////////////////
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  finger_detected=finger.fingerID;//MATCH AND FOUND
  return finger.fingerID;
}
////////////////////////////////////////////////
///////////////ATTENDANCE FINGERPRINT///////////
void attendance_fingerprint(){
  getFingerprintID();
  if(finger_detected!=0){
    fingerprint_timein(finger_detected);
    reset_counterlevel();
  }
}
void fingerprint_timein(int id){
   if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    Serial.println(id);
    HTTPClient http;
    String postData = "fingerid="+String(id)+"&action=timeintimeout_insert";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/process.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode > 0){
      Serial.println("Success Time in");
      Serial.println(payload);
    }
    http.end();
  }
}
////////////////////////////////////////////////


void loop() {
  //ATTENDANCE FINGERPRINT
  attendance_fingerprint();
 //DELETE FINGERPRINT
 if(delete_fingerprint()==1){
  int delete_id = delete_fingerid();
  finger_registeroff();
  if(delete_id!=0){
    delete_fingerprint(delete_id);
  }
 }
 //INSERT FINGERPRINT
 if(register_fingerprint()==1){//register to slot of fingerprint flash mem
  verify_fingerid();
  finger_registeroff();
  insert_fingerprint();
 }
 

 
// else{
//  
// }
 delay(2000);    
}
