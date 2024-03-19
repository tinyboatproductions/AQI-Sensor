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

//Password and stuff
#include "arduino_secrets.h"

//SparkFun AQI Sensor
#include "SparkFun_ENS160.h"

//Stuff needed to get the time
const char* ssid = SECRET_SSID;
const char* password = SECRET_PSWD;

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

//ENS stuff
SparkFun_ENS160 myENS; 
int ensStatus;
int ppb = 0;
int ppm = 0;
int aqi = 0;
int flags = 0;

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(115200); //Create a serial output for debugging
  Serial.println();
  Serial.println("Begin");
  wifiConnection();
  setupSD();
  pinMode(AQIpin, INPUT); // setup the AQI sensor pin
  displaySetup();
  ensSetup();
}

void loop() {
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
    appendFile(SD, fileName.c_str(), timeString.c_str()); //Add the time to the file as a string
    appendFile(SD, fileName.c_str(), ",");
    String Temp = String(concentration); //Convert the reading from a float to a string
    appendFile(SD, fileName.c_str(), Temp.c_str());

    

    //Print AQI Stuff
    Serial.print("Air Quality Index (1-5) : ");
    aqi = myENS.getAQI();
		Serial.println(aqi);

    String TempAQI = String(aqi); //Convert the reading from a float to a string
    appendFile(SD, fileName.c_str(), TempAQI.c_str()); //Add the aqi to the file as a string
    appendFile(SD, fileName.c_str(), ",");

		Serial.print("Total Volatile Organic Compounds: ");
    ppb = myENS.getTVOC();
		Serial.print(ppb);
		Serial.println("ppb");
    
    String TempPPB = String(ppb); //Convert the reading from a float to a string
    appendFile(SD, fileName.c_str(), TempPPB.c_str()); //Add the TVOC to the file as a string
    appendFile(SD, fileName.c_str(), ",");


		Serial.print("CO2 concentration: ");
    ppm = myENS.getECO2();
		Serial.print(ppm);
		Serial.println("ppm");

    String TempPPM = String(ppm); //Convert the reading from a float to a string
    appendFile(SD, fileName.c_str(), TempPPM.c_str()); //Add the CO2 to the file as a string
    appendFile(SD, fileName.c_str(), ",");
    appendFile(SD, fileName.c_str(), "\n");

	  Serial.print("Gas Sensor Status Flag (0 - Standard, 1 - Warm up, 2 - Initial Start Up): ");
    flags = myENS.getFlags();
    Serial.println(flags);

    displayUpdate();  //Update the OLED display
		Serial.println();
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
  myFile.close();

  //Make sure the card is there
  uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
    Serial.println("no Card");
  }

  appendFile(SD, fileName.c_str(), "Time, Concentration, AQI, TVOC, CO2,\n");

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

void ensSetup(){
  if( !myENS.begin() )
	{
		Serial.println("Could not communicate with the ENS160, check wiring.");
		while(1);
	}
  if( myENS.setOperatingMode(SFE_ENS160_RESET) )
  Serial.println("Ready.");
	delay(100);
	myENS.setOperatingMode(SFE_ENS160_STANDARD);
  ensStatus = myENS.getFlags();
	Serial.print("Gas Sensor Status Flag (0 - Standard, 1 - Warm up, 2 - Initial Start Up): ");
	Serial.println(ensStatus);
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
  display.print("2.5:");
  display.println(concentration);
  display.print("Avg:");
  display.println(avg);
  display.print("VOC:");
  display.println(ppb);
  display.print("CO2:");
  display.println(ppm);

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
