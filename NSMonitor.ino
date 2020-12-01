/************************************************************************
   ESP8266 Nightscout Mini Monitor
   v 1.0 // January 2020 // @andyfry001
   Sketch to display NightScout info on SSD1306 OLED 128x64 using ESP8266
*************************************************************************/
#include <SPI.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <ArduinoJson.h>
#include <TimeLib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define LED_RED 0 //Red LED pin D3
#define LED_GREEN 12 //Green LED pin D6
#define LED_BLUE 14 //blue LED pin D5
const int buttonPin = 13;    // the number of the pushbutton pin

//Fingerprint from ssl certificate
const char* fingerprint = "083b717202436ecaed428693ba7edf81c4bc6230";
const char* ssid = "WiFry";
const char* password = "GP9TeMftybiu";

//Nightscout website
const char* NSEntries = "https://fryanightscout.herokuapp.com/api/v1/entries.json";
const char* NSCurrent = "https://fryanightscout.herokuapp.com/api/v1/entries/current.json";
const char* NSDeviceStatus = "https://fryanightscout.herokuapp.com/api/v1/devicestatus?count=1";

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); //declare display object

//logo bitmap
static const unsigned char PROGMEM logo_bmp[] = {
  // 'NS', 42x42px
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7f, 0x80, 0x00, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x70, 0x00, 0x00,
  0x00, 0x07, 0x00, 0x38, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0xfe,
  0x00, 0x00, 0x00, 0x1f, 0xf3, 0xfe, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0x00, 0x00, 0x00, 0x3b,
  0xde, 0xf7, 0x00, 0x00, 0x00, 0x37, 0xed, 0xfb, 0x00, 0x00, 0x00, 0x3e, 0x6d, 0x9f, 0x00, 0x00,
  0x00, 0x3c, 0x6d, 0x8f, 0x00, 0x00, 0x00, 0x36, 0x6d, 0x9b, 0x00, 0x00, 0x00, 0x37, 0xed, 0xfb,
  0x00, 0x00, 0x00, 0x39, 0x8c, 0x63, 0x00, 0x00, 0x00, 0x38, 0x0c, 0x07, 0x00, 0x00, 0x00, 0x1c,
  0x0c, 0x0e, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x3e, 0x00, 0x00,
  0x00, 0x0f, 0x80, 0x7c, 0x00, 0x00, 0x00, 0x0d, 0xff, 0xec, 0x00, 0x00, 0x00, 0x06, 0x7f, 0x98,
  0x00, 0x00, 0x00, 0x06, 0x18, 0x18, 0x00, 0x00, 0x00, 0x03, 0x30, 0x30, 0x00, 0x00, 0x00, 0x03,
  0xf0, 0x70, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x60, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0xe1, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x73, 0x80, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM FortyFiveDown[] = {
  //FortyFiveDown arrow 8x8px
  0x40, 0xe0, 0x71, 0x3b, 0x1f, 0x0f, 0x1f, 0x3f
};

static const unsigned char PROGMEM FortyFiveUp[] = {
  // 'FortyFiveUp', 8x8px
  0x3f, 0x1f, 0x0f, 0x1f, 0x3b, 0x71, 0xe0, 0x40
};

static const unsigned char PROGMEM DoubleDown[] = {
  // 'DoubleDown', 10x10px
  0x00, 0x00, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00, 0xff, 0xc0,
  0x73, 0x80, 0x21, 0x00
};

static const unsigned char PROGMEM DoubleUp[] = {
  // 'DoubleUp', 10x10px
  0x21, 0x00, 0x73, 0x80, 0xff, 0xc0, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00, 0x21, 0x00,
  0x21, 0x00, 0x00, 0x00
};

ESP8266WiFiMulti WiFiMulti;

//Initialise variables
int arrow = 0;
int sgv = 0;
float delta = 0;
float COB = 0;
float IOB = 0;
String payload;
const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(14) + 230;
const size_t devicestatus_capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(66) + JSON_ARRAY_SIZE(67) + JSON_ARRAY_SIZE(68) + JSON_OBJECT_SIZE(1) + 2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + 2 * JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(12) + 800;
//BG targets
float lowBG = 4.0;
float highBG = 8.0;
float vHighBG = 12.0;
//display
bool simpleDisp = false;
int buttonState;
int lastButtonState = LOW;
//time variables
int m = 0;
int h = 0;
int s = 0;
unsigned long startMillis;
unsigned long currentMillis = 0;
unsigned long lastPingMillis = 0;
const unsigned long clockPeriod = 1000;  //update display every 1s
const unsigned long pingPeriod = 300000;  //ping NS every 5 min
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers
time_t tmNS = 0; //NS entry timestamp

//graph points locations
const int x[12] = {6, 11, 16, 21, 26, 31, 36, 41, 46, 51, 56, 61};
int y[12] = { 0 };

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  pinMode(LED_RED,   OUTPUT);   // sets the LED pins as outputs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE,  OUTPUT);
  analogWrite(LED_RED,   50);
  analogWrite(LED_GREEN, 50);
  analogWrite(LED_BLUE,  50);

  pinMode(buttonPin, INPUT);

  //splash screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 0);
  display.println("Nightscout Monitor");
  drawLogo(42, 42);
  display.display();
  delay(5000);
  display.clearDisplay();

  startMillis = millis();

  //Start WiFI connection
  WiFiMulti.addAP(ssid, password);
  while ( WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //get initial data from NS
  DynamicJsonDocument docA(10 * capacity);
  docA = getNSInfo(NSEntries, 10 * capacity); //read last 10 entries
  if (!docA.isNull()) {
    const char* NSTimeStamp = docA[0]["dateString"]; // "2020-01-09T18:01:48.769Z"
    setNSTimeStamp(NSTimeStamp);
    sgv = docA[0]["sgv"];
    delta = docA[0]["delta"]; // 2.028
    const char* arrowDir = docA[0]["direction"]; // "Flat"
    arrow = arrowDirection(arrowDir); //turn char into int

    lastPingMillis = millis();
  }
  uint16_t j = 11;
  uint16_t i = 0;
  for (j = 11; j > 1; j--) {
    //update graph y values with last 10 NS entries
    int sgvTemp = docA[i]["sgv"];
    y[j] = getY(sgvTemp / 18.01559);
    i++;
  }
  //--------IOB and COB--------------------------------------------
  DynamicJsonDocument docB(devicestatus_capacity);
  docB = getNSInfo(NSDeviceStatus, devicestatus_capacity);
  if (!docB.isNull()) {
    COB = docB[0]["openaps"]["suggested"]["COB"];
    IOB = docB[0]["openaps"]["iob"]["iob"];
    Serial.print("COB: ");
    Serial.println(COB, 2);
    Serial.print("IOB: ");
    Serial.println(IOB, 2);
  }

  //update display with initial data
  //drawCOB(COB);
  drawIOBCOB(IOB, COB);
  updateLED(sgv);
  drawBG(sgv, delta);
  drawArrow(arrow);
  if (!simpleDisp) {
    drawGraph(x, y);
    drawClock(h, m);
  }
  display.display();

  //Sync update cycle to NS updates
  //difference between NS timestamp and ping time
  unsigned long diff = now() - tmNS;
  //shift ping time by diff to sync calls (+small offset)
  diff = (diff - 10) * 1000; //add 5 sec offset to allowNS update
  lastPingMillis -= diff;
}

void loop() {
  checkButtonPush();

  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  //if 60s have elapsed
  if (currentMillis - lastPingMillis >= pingPeriod)
  {
    //getNSinfo------------------------------------
    Serial.println("get NS info...");
    DynamicJsonDocument docA(capacity);
    docA = getNSInfo(NSCurrent, capacity);

    //Device status for IOB and COB
    DynamicJsonDocument docB(devicestatus_capacity);
    docB = getNSInfo(NSDeviceStatus, devicestatus_capacity);

    //check if JSon object is empty
    //parse NS info
    if (!docA.isNull()) {
      const char* NSTimeStamp = docA[0]["dateString"]; // "2020-01-09T18:01:48.769Z"
      setNSTimeStamp(NSTimeStamp);

      sgv = docA[0]["sgv"];
      Serial.print("sgv: ");
      Serial.println(sgv);

      delta = docA[0]["delta"]; // 2.028
      Serial.print("delta: ");
      Serial.println(delta);

      const char* arrowDir = docA[0]["direction"]; // "Flat"
      arrow = arrowDirection(arrowDir); //turn char into int
      Serial.print("direction: ");
      Serial.print(arrowDir);
      Serial.print(", arrow number: ");
      Serial.println(arrow);
    }

    //parse IOB and COB
    if (!docB.isNull()) {
      COB = docB[0]["openaps"]["suggested"]["COB"];
      IOB = docB[0]["openaps"]["iob"]["iob"];
      Serial.print("COB: ");
      Serial.println(COB, 2);
      Serial.print("IOB: ");
      Serial.println(IOB, 2);
    }

    //update graph points
    for (int i = 0; i < 11; i++) {
      y[i] = y[i + 1];
      Serial.print(" y[");
      Serial.print(i);
      Serial.print("]: ");
      Serial.println(y[i]);
    }
    y[11] = getY(sgv / 18.01559);
    Serial.print("y[11]: ");
    Serial.println(y[11]);
    lastPingMillis = currentMillis;  //IMPORTANT to save the current time otherwise loop alwasy true
  }//end server ping (5min) if

  //update display every second------------------------
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= clockPeriod)  //test if 1 sec elapsed
  {
    time_t current = now();
    h = hour(current);
    m = minute(current);
    s = second(current);

    //--------update display-----------
    updateLED(sgv);
    display.clearDisplay();
    //updateBG value
    drawBG(sgv, delta);
    //update arrow
    drawArrow(arrow);

    if (!simpleDisp) {
      //drawCOB(COB);
      drawIOBCOB(IOB, COB);
      drawGraph(x, y); //update graph
      drawClock(h, m); //update clock
      //time since last update
      drawTimeSince(current);
    }
    display.display();
    /*Serial.print("h:m ");
      Serial.print(h);
      Serial.print(":");
      Serial.print(m);
      Serial.print(":");
      Serial.println(s);*/
    startMillis = currentMillis; //save loop time
  }//end 1 sec if
}//end loop()--------------------------------------------------

DynamicJsonDocument getNSInfo(const char* apiAddress, size_t jsonDocSize) {
  //function get JSON info from nightscout website
  // wait for WiFi
  Serial.println("waiting for connection...");
  DynamicJsonDocument doc(jsonDocSize);
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);
    //client->setInsecure(); to use https without certificate fingerprint (insecure)

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, apiAddress)) {  // HTTPS

      //get headers
      const char* headerNames[] = { "Date" };
      https.collectHeaders(headerNames, sizeof(headerNames) / sizeof(headerNames[0]));

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      if (httpCode > 0) { // httpCode will be negative on error
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          //get date from header
          String datestr = https.header("Date");
          char* date = (char*)malloc(30);
          datestr.toCharArray(date, 30);
          Serial.print("Date header: ");
          Serial.println(date);
          //update time
          updateTime(date);
          free(date);

          //http GET payload
          String payload = https.getString();
          char* payload1 = (char*)malloc(https.getSize() + 1);
          payload.toCharArray(payload1, payload.length() + 1);
          Serial.print("payload1: ");
          Serial.println(payload1);
          https.end();

          //parsing----------
          DeserializationError error = deserializeJson(doc, payload1);
          free(payload1);
          if (error) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(error.c_str());
            return doc;
          }
          return doc;
        }
      }
      else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
        return doc;
      }

      https.end();
    }
    else {
      Serial.printf("[HTTPS] Unable to connect\n");
      return doc;
    }
  }
  else {
    Serial.println("WiFi not connected");
  }
}

int getY(float bg) {
  if (bg >= 11.6) {
    return 15;
  }
  //low BG
  else if (bg < 2.5) {
    return 59;
  }
  else {
    //calc y value
    int y = floor(60 - ((bg - 2.5) * 5));
    return y;
  }
}//getY

void drawGraph(const int x[12], int y[12]) {
  display.setTextSize(1);
  for (int i = 0; i < 12; i++) {
    //for initial readings
    if (y[i] == 0) {
      //do nothing
    }
    //high BG
    else if (y[i] <= 15 && y[i] > 0) {
      display.setCursor(x[i], 15);
      display.write(30);//30 triangle up
    }
    //low BG
    else if (y[i] >= 59) {
      display.setCursor(x[i], 59);
      display.write(31);//31 triangle down
    }
    else {
      display.setCursor(x[i], y[i]);
      display.write(7); //7  point
    }
  }
  // target range
  display.setCursor(0, 31);
  display.print("8");
  display.setCursor(0, 51);
  display.print("4");
  display.drawFastHLine(7, 35, 59, SSD1306_WHITE); //8 mmol/l
  display.drawFastHLine(7, 55, 59, SSD1306_WHITE); //4 mmol/l
  //display.drawFastHLine(0, 17, 70, SSD1306_WHITE);//upper limit 11.6mmol/l
  //display.drawFastHLine(0, 63, 70, SSD1306_WHITE);//lower limit 2.6mmol/l
}//drawGraph

void updateTime(char* date) {
  //date format from server header: Tue, 14 Jan 2020 09:36:39 GMT
  tmElements_t timeArray;
  time_t tm; //time

  timeArray.Day = atoi(&date[5]);
  timeArray.Month = 1;
  timeArray.Year = atoi(&date[12]) - 1970; //unix offset
  timeArray.Hour = atoi(&date[17]);
  timeArray.Minute = atoi(&date[20]);;
  timeArray.Second = atoi(&date[23]);;

  tm =  makeTime(timeArray);
  setTime(tm);

  //Serial.print("Unix timestamp set to: ");
  //Serial.println(tm);
  Serial.print("Time set to: ");
  Serial.print(hour(tm));
  Serial.print(":");
  Serial.print(minute(tm));
  Serial.print(":");
  Serial.println(second(tm));
}

void setNSTimeStamp(const char* date) {
  //date format "2020-01-09T18:01:48.769Z"
  tmElements_t timeArray;

  timeArray.Day = atoi(&date[8]);
  timeArray.Month = atoi(&date[5]);
  timeArray.Year = atoi(&date[0]) - 1970; //unix offset
  timeArray.Hour = atoi(&date[11]);
  timeArray.Minute = atoi(&date[14]);
  timeArray.Second = atoi(&date[17]);

  tmNS =  makeTime(timeArray);

  //Serial.print("NS UNIX timestamp set to: ");
  //Serial.println(tmNS);
  Serial.print("NS timestamp set to: ");
  Serial.print(hour(tmNS));
  Serial.print(":");
  Serial.print(minute(tmNS));
  Serial.print(":");
  Serial.println(second(tmNS));
}

void drawTimeSince(time_t current) {
  display.setCursor(75, 30);//x75, y55
  display.setTextSize(1);
  time_t test = current - tmNS;
  Serial.print("test time_t: ");
  Serial.println(test);
  int timeSince = minute(test);
  Serial.print("current: ");
  Serial.println(current);
  Serial.print("minute current: ");
  Serial.println(minute(current));
  Serial.print("tmNS: ");
  Serial.println(tmNS);
  Serial.print("Minute tmNS: ");
  Serial.println(minute(tmNS));
  Serial.print("Time since last update: ");
  Serial.println(timeSince);
  
  display.print(timeSince);
  display.println("' ago");
}

void drawBG(float bg, float delta) {
  //BG value
  bg = bg / 18.01559; //convert to mmoll/l
  delta = delta / 18.01559;

  //Normal Display
  int x1 = 75;
  int y1 = 3;//y27
  int textSize1 = 2;
  int textSize2 = 1;
  int x2 = x1 + 7;
  int y2 = y1 + 17; //45;

  //Simple display
  if (simpleDisp) {
    x1 = 25;
    y1 = 10;
    textSize1 = 4;
    textSize2 = 2;
    x2 = x1 + 14;
    y2 = y1 + 35; //45;
  }

  //Delta value
  display.setTextSize(textSize2);
  display.setCursor(x1, y2);
  display.write(30); //delta
  display.setCursor(x2, y2);
  if (delta > 0.0) {
    display.print("+");
  }
  display.print(delta, 1);

  //BG value
  if (bg > 9.9) {
    //display extra digit
    x1 = x1 - 10;
  }
  display.setCursor(x1, y1);
  display.setTextSize(textSize1);
  display.println(bg, 1); //1 decimal place
}

void drawClock(int h, int m) {
  display.setCursor(0, 5);
  display.setTextSize(1);
  if (h < 10) {
    display.print('0');
  }
  display.print(h);
  display.print(":");
  if (m < 10) {
    display.print('0');
  }
  display.print(m);
}

void drawArrow(int arrowInt) {
  int xOrigin = 115;
  int yOrigin = 5;//27;
  display.setTextSize(2);
  if (simpleDisp) {
    xOrigin = 105;
    yOrigin = 15;
    display.setTextSize(3);
  }

  if (arrowInt == 1) {
    //Right
    display.setCursor(xOrigin, yOrigin);
    display.write(26);//right
  }
  else if (arrowInt == 2) {
    //Up arrow
    display.setCursor(xOrigin, yOrigin);
    display.write(24);//up
  }
  else if (arrowInt == 3) {
    //FOrtyFiveUp
    display.drawBitmap(xOrigin + 2, yOrigin, FortyFiveUp, 8, 8, 1);
  }
  else if (arrowInt == 4) {
    //FortyFiveDown
    display.drawBitmap(xOrigin + 2, yOrigin + 5, FortyFiveDown, 8, 8, 1);
  }
  else if (arrowInt == 5) {
    //Down
    display.setCursor(xOrigin, yOrigin);
    display.write(25);//down
  }
  else if (arrowInt == 6) {
    //DoubleUp
    display.drawBitmap(xOrigin, yOrigin + 2 , DoubleUp, 10, 10, 1);
  }
  else if (arrowInt == 7) {
    //DoubleDown
    display.drawBitmap(xOrigin, yOrigin + 2, DoubleDown, 10, 10, 1);
  }
  else {
    display.setCursor(xOrigin, yOrigin);
    display.write('-');
  }
}

int arrowDirection(const char* arrow) {
  int arrowDir = 0;
  char flat[5] = "Flat";
  char up[9] = "SingleUp";
  char FFUp[12] = "FortyFiveUp";
  char FFDown[14] = "FortyFiveDown";
  char down[11] = "SingleDown";
  char dUp[9] = "DoubleUp";
  char dDown[11] = "DoubleDown";

  if (strcmp(arrow, flat) == 0) {
    arrowDir = 1;
  }
  else if (strcmp(arrow, up) == 0) {
    arrowDir = 2;
  }
  else if (strcmp(arrow, FFUp) == 0) {
    arrowDir = 3;
  }
  else if (strcmp(arrow, FFDown) == 0) {
    arrowDir = 4;
  }
  else if (strcmp(arrow, down) == 0) {
    arrowDir = 5;
  }
  else if (strcmp(arrow, dUp) == 0) {
    arrowDir = 6;
  }
  else if (strcmp(arrow, dDown) == 0) {
    arrowDir = 7;
  }
  else {
    arrowDir = 0;
  }
  return arrowDir;
}

void drawLogo(int LOGO_WIDTH, int LOGO_HEIGHT) {
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    14,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
}

void updateLED(int sgv) {
  analogWriteRange(255);
  float dim = 0.10; //0.25 is 25% brightness

  float bg = sgv / 18.01559;
  if (bg < lowBG) {
    //low
    Serial.println("LED low");
    analogWrite(LED_RED, 255 - floor(55 * dim)); //HIGH is off
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_BLUE, 255);
  }
  else if (bg > lowBG && bg < highBG) {
    //target
    analogWrite(LED_RED, 255); //HIGH is off
    analogWrite(LED_GREEN, 255 - floor(55 * dim));
    analogWrite(LED_BLUE, 255);
  }
  else if (bg > highBG && bg < vHighBG) {
    //high
    analogWrite(LED_RED, 255 - floor(255 * dim)); //HIGH is off
    analogWrite(LED_GREEN, 255 - floor(110 * dim));
    analogWrite(LED_BLUE, 255);
  }
  else if (bg > vHighBG) {
    //very high
    analogWrite(LED_RED, 255 - floor(55 * dim)); //HIGH is off
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_BLUE, 255);
  }
  else {
    analogWrite(LED_RED, 0); //HIGH is off
    analogWrite(LED_GREEN, 255);
    analogWrite(LED_BLUE, 0);
  }
}

void checkButtonPush() {
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        simpleDisp = !simpleDisp;
        Serial.print("simpleDisp ");
        Serial.println(simpleDisp);
      }
    }
  }
  lastButtonState = reading;
}

void drawIOBCOB(float iob, float cob) {
  display.drawRoundRect(72, 41, 48, 11, 3, SSD1306_WHITE);
  display.setCursor(75, 43);
  display.setTextSize(1);
  display.print("IOB ");
  display.print(iob, 1);

  display.drawRoundRect(72, 53, 48, 11, 3, SSD1306_WHITE);
  display.setCursor(75, 55);
  display.setTextSize(1);
  display.print("COB ");
  display.print(cob, 0);
}
