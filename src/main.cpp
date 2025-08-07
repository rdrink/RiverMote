/*
  OpenLog
  By: Nathan Seidle
  SparkFun Electronics
  Date: February 2nd, 2018
*/

#include "Particle.h"
#include <Wire.h>
#include "SparkFun_Qwiic_OpenLog_Arduino_Library.h"
#include "location.h"
#include "DS2482-RK.h"
OpenLog sdLog; //Create instance
LocationPoint point = {};

SYSTEM_MODE(AUTOMATIC);

SerialLogHandler logHandler(LOG_LEVEL_INFO);

/***************************************************
 DFRobot Gravity: Analog TDS Sensor / Meter 
 Created 2017-8-22
 By Jason <jason.ling@dfrobot.com@dfrobot.com>
 
 ****************************************************/
#define TdsSensorPin A0
#define VREF 3.3      // analog reference voltage of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;

// Gravity turbidity sensor //
#define turbiditySensorPin A2

#define sampleInterval 40U
#define logInterval 3*1000U

#define waterPin D3 // 100k pulldown to gnd (pin 34)

DS2482 ds(Wire, 0);

// #define DEBUGGING // uncomment for serial debugging; comment for logging

// -- Global vars -- //
String data_str = "null";
int tdsMedian = -0;
int tdsSampMax = -999;
int tdsSampMin = 999;

float turbiditySensorValue = -0;
float turbidityMin = 999;
float turbidityMax = -999;

float tempValue = -999;

int senseWater = 0;

String date_time;
char buf[256];
long lat = 0.0, lon = 0.0; 

String formatCSV(const String &date, int tMed, float turbidity, float temp) { // These -> will have to be manually expanded for now
    String csv_data =
        "Date: " + date + "," +
        "Latitude: " + point.latitude + "," +
        "Longitude: " + point.longitude + "," +
        "Speed: " + point.speed + "," +
        "Course: " + point.heading + "," +
        "TDS Median: " + String(tMed) + "," +
        "Turbidity: "+ String(turbidity);
        "Temp C°: "+ String(temp);
    return csv_data;
}

// old school JSON //
String formatJSON(const String &date, int tMed, float turbidity, float temp) { // These -> will have to be manually expanded for now
    String json_data = String::format(
        "{\"date\":%s,\"lat\":%f,\"lng\":%f,\"speed\":%f,\"course\":%f,\"tdsMedian\":%f,\"turbidity\":%f\"temp\":%f}",
        date.c_str(),
        point.latitude, point.longitude,
        point.speed,
        point.heading,
        tMed,
        turbidity,
        temp
    ).c_str();
    return json_data;
}


void logToSD(String info) {
    sdLog.println(info);
    sdLog.syncFile();
}

float tdsCompensation(float value) {
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      // -- should use actual water temp here -- //
      float compensationVolatge=value/compensationCoefficient;  //temperature compensation
      //tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      tdsValue=(133.42* pow(compensationVolatge,3)- 255.86* pow(compensationVolatge, 2) + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      return tdsValue;
}

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
	  bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
	  for (i = 0; i < iFilterLen - j - 1; i++) 
          {
	    if (bTab[i] > bTab[i + 1]) 
            {
		bTemp = bTab[i];
	        bTab[i] = bTab[i + 1];
		bTab[i + 1] = bTemp;
	     }
	  }
      }
      if ((iFilterLen & 1) > 0)
	bTemp = bTab[(iFilterLen - 1) / 2];
      else
	bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void setup() {
    Wire.begin(); //Initialize I2C
    sdLog.begin(); //Open connection to OpenLog (no pun intended)
    Serial.begin(9600);
    pinMode(TdsSensorPin,INPUT);
    pinMode(turbiditySensorPin, INPUT);
    
    while(!Particle.connected()) {}; // wait to connect before powering bus
    pinMode(D7,OUTPUT); digitalWrite(D7,HIGH); // Enable Muon power
    Time.zone(-5); // Set CDT time zone (CST is -6)

    // Initialize GPS
    LocationConfiguration config;
    config.enableAntennaPower(GNSS_ANT_PWR);
    Location.begin(config);

    ds.setup();
	  DS2482DeviceReset::run(ds, nullptr);

    pinMode(D3, INPUT);
}



void loop() {
    /* date formatting not working 
   //String date_time = Time.format(Time.now(), TIME_FORMAT_DEFAULT);
   //String date_time = Time.timeStr().c_str(); */
   
   static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > sampleInterval)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     float sampleValue = analogRead(TdsSensorPin);
     analogBuffer[analogBufferIndex] = sampleValue;    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
    
    turbiditySensorValue = analogRead(turbiditySensorPin);
    if(turbiditySensorValue < turbidityMin) { turbidityMin = turbiditySensorValue; } // Min
    if(turbiditySensorValue > turbidityMax) { turbidityMax = turbiditySensorValue; } // Max
   }
   
   // Fetch GPS data when available
   if (Location.getStatus() != LocationResults::Acquiring) {
      Location.getLocation(point, nullptr);
   }
   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > logInterval)
   {
      /***** TDS *****/
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];

      // -- should read actual ADC ref here -- //
      //averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4095.0; // changed to 4096 (2^12) for Particle M-core 
      tdsMedian = tdsCompensation(averageVoltage);
 
      /***** Turbidity *****/
      float turbidityValue = turbiditySensorValue * (5.0 / 1024.0); // ref volatge / 2^12

      /***** Temp *****/
      DS24821WireAddress addr;
      DS2482GetTemperatureCommand::run(ds, addr, [](DS2482GetTemperatureCommand&, int status, float tempC) {
        if (status != DS2482Command::RESULT_DONE) {
          return;
        }
        tempValue = tempC;
        //Serial.printlnf("temperature: %f C", tempValue);
      });

      
      String date = Time.timeStr();
      
      #ifndef DEBUGGING // LOG
      // choose format
      //String ldata = formatJSON(date, tdsMedian, turbidityValue);
      String ldata = formatCSV(date, tdsValue, turbidityValue, tempValue);
      logToSD(ldata);
      #endif
      
      #ifdef DEBUGGING // SERIAL
      //String sdata = formatJSON(date, tdsMedian, turbidityValue);
      String sdata = formatCSV(date, tdsValue, turbidityValue, tempValue);
      Serial.println(sdata);  
      #endif
      
      //-- reset values --//
      //turbMin
   }

   ds.loop(); // temp sensor

  senseWater = digitalRead(waterPin); 
  if(senseWater == 1) {
    Log.info("WATER!");
   delay(250);
  }
  
}

  
