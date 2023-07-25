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

int level_fingerprintregister=0;


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
}
///////////////////////////////////////////

void insert_fingerprint() {
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("Wifi Connected");
    HTTPClient http;
    int id = finger.templateCount + 3;//start with 3
    Serial.println(id);
    while(level_fingerprintregister==0){
      getFingerprintEnroll(id);
    }
    String postData = "fingerid="+String(id)+"&action=insert_fingerprint";
    http.begin(wificlient,"http://192.168.1.8/BIOMETRICS/fingerprint-actions/process.php");              
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  
    int httpCode = http.POST(postData);
    String payload = http.getString();
    if(httpCode > 0){
      Serial.println(payload);
    }
    reset_counterlevel();
    http.end();
  }
}

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





void loop() {
 if(register_fingerprint()==1){
  insert_fingerprint();
 }
// else{
//  
// }
 delay(2000);    
}
