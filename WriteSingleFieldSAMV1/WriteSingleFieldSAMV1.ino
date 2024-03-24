#include <M5StickCPlus.h>
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h" // always include thingspeak header file after other header files and custom macros

#define LED_PIN 10

int lastUpdate = 0; // Added semicolon at the end of the line
int currMillis = 0; // Added semicolon at the end of the line
char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password

WiFiClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;

// setup for maxMag IMU
float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float maxX = 0.0F;
float maxY = 0.0F;
float maxZ = 0.0F;

float magSq = 0.0F;

float mag = 0.0F;

float maxMag = 0.0F;

void setup() {
  Serial.begin(115200);  // Initialize serial
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  WiFi.mode(WIFI_STA);   
  
  //start the thingspeak
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  M5.begin();             // Init M5StickC Plus.  初始化 M5StickC Plus
  M5.IMU.Init();          // Init IMU.  初始化IMU
  M5.Lcd.setRotation(3);  // Rotate the screen. 将屏幕旋转
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(3);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
}

void loop() {
  currMillis = millis();

  //used to reset m5stickc
  M5.update();  // Read the press state of the key.  读取按键 A, B, C 的状态
  if (M5.BtnA.wasReleased()) {  // If the button A is pressed.  如果按键 A 被按下
    maxMag = 0.0F;
  }

M5.IMU.getAccelData (&accX, &accY, &accZ);

  float xSq = sq(accX);
  float ySq = sq(accY);
  float zSq = sq(accZ);

   magSq = xSq + ySq + zSq;

   mag = sqrt(magSq);

  if (mag > maxMag) {
    maxMag = mag;
  }

  M5.Lcd.setCursor(30, 30);
  M5.Lcd.printf("%5.2f", mag);

  M5.Lcd.setCursor(30, 60);
  M5.Lcd.printf("%5.2f", maxMag);

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  if (currMillis - lastUpdate >= 20000) {
    int x = ThingSpeak.writeField(myChannelNumber, 1, maxMag, myWriteAPIKey);
    if (x == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }

    // reset lastUpdate = currMillis
    lastUpdate = currMillis;
  }
}
