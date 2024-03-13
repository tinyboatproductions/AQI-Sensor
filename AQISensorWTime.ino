/*
ESP Write to File
3/12/24
Referances
NTP time : https://lastminuteengineers.com/esp32-ntp-server-date-time-tutorial/
File Com: https://www.instructables.com/Light-Color-Tracker/
OLED: https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/

*/

//Stuff needed for the SD card
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include "Wire.h"

//Stuff needed for the NTP sever (getting the time from the internet)
#include "WiFi.h"
#include "time.h"

//Stuff for the screen
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET    -1

#include "SECRECTS.h"

//Stuff needed to get the time
const char* ssid = SECRECT_SSID;
const char* password = SECRECT_PSW;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -21600;
const int daylightOffset_sec = 3600;

//global variables beacuase I cannot be assed to pass these back and forth without screwing it up
String fileName = "";
String timeString = "";
File myFile;
const int CS = 5;


//declare the display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Stuff to make the AQI sensor work, all good practices are out the window, i just know how to write basic stuff dont @ me
int AQIpin = 34;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;//sampe 30s ;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

float runningTotal = 0;
int numMeasurments = 1;
int goodValues = 0;
float avg = 0;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //Create a serial output for debugging
  Serial.println();
  Serial.println("Begin");
  wifiConnection();
  setupSD();
  pinMode(AQIpin, INPUT); // setup the AQI sensor pin
  displaySetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  take a reading
  update the time string
  Write the reading to the file
  add to the total number of readings
  Check the reading
    if it is good,
      add to the number of good values
      get the current average reading
    else
      add to the number of bad values
  */
  duration = pulseIn(AQIpin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
  {
    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    numMeasurments += 1;

    if(concentration > 1){
      goodValues += 1;
      runningTotal += concentration;
      avg = runningTotal / goodValues;
    } else {
      concentration = 0.0;
    }
    lowpulseoccupancy = 0;
    starttime = millis();
    Serial.print("Concentration: ");
    Serial.println(concentration);  // this seems block for 30 seconds so no delay is needed
    printLocalTime(); //update the time to be current
    //writeToFile();    //add the new reading to the file with the time
    //myFile = SD.open(fileName, FILE_WRITE);
    //char dataString = timeString.c_str() +  ", " + concentration + "\n";
    appendFile(SD, fileName.c_str(), timeString.c_str());
    appendFile(SD, fileName.c_str(), ",");
    String Temp = String(concentration);
    appendFile(SD, fileName.c_str(), Temp.c_str());
    appendFile(SD, fileName.c_str(), "\n");
    /*
    myFile.seek(0);
    myFile.print(timeString);
    myFile.print(',' );
    myFile.print(concentration);
    myFile.print('\n');
    myFile.close();
    Serial.println("printed to file.");
    */
    displayUpdate();  //Update the OLED display
  }

}

//Connect to the wifi and get the time, pass back to the setup function
void wifiConnection(){
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

//Setup writing to the SD card, create the file name and add a header to the file
void setupSD(){
  Serial.print("Initializing SD card...");
  if (!SD.begin(CS)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  //Create the file name with the current time
  fileName = "/" + String(timeString) + ".txt";
  Serial.print("File Name: ");
  Serial.println(fileName);
  myFile = SD.open("fileName.txt", FILE_WRITE);

  //Make sure the card is there
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("no Card");
  }

  //Add a heading line to the file
  if (myFile){
    Serial.println("Creating file...");
    myFile = SD.open(fileName, FILE_WRITE);
    myFile.print("test");
    myFile.print("time, reading");
    myFile.close();
  } else {
    Serial.println("Failed to create file");
  }
}

//setup the display and print out a ready message
void displaySetup(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Ready!");
  display.display();
  Serial.println("Screen setup done.");
}


//Take a reading from the sensor
void takeReading(){

}

//get the current time and put it onto the global time string
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A %B %d %Y %H.%M.%S");
  char time[32];
  strftime(time,32, "%B%d%Y%H.%M.%S", &timeinfo);
  timeString = time;
  //Serial.println(time);
  return;
}

//Write the current time string and the given value to the file
void writeToFile(){
  myFile = SD.open(fileName, FILE_WRITE);
  myFile.seek(0);
  myFile.print(timeString);
  myFile.print(", ");
  myFile.println(concentration);
  myFile.close();
  Serial.println("printed to file.");
}

//update the OLED display
void displayUpdate(){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Val:");
  display.println(concentration);
  display.print("Avg:");
  display.println(avg);
  //display.print("Num:");
  //display.println(numMeasurments);
  //display.println("==========");
  display.print("% OK:");
  display.print(goodValues / numMeasurments * (100.0));
  display.display();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}