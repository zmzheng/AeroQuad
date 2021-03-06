/*
  AeroQuad v3.0.1 - February 2012
  www.AeroQuad.com
  Copyright (c) 2012 Ted Carancho.  All rights reserved.
  An Open Source Arduino based multicopter.
 
  This program is free software: you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version. 

  This program is distributed in the hope that it will be useful, 
  but WITHOUT ANY WARRANTY; without even the implied warranty of 
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details. 

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <http://www.gnu.org/licenses/>. 
*/

#include "Ultrasonic.h"
#define TRIGGER_PIN  34
#define ECHO_PIN     35
Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);


#ifndef _AEROQUAD_MAX_SONAR_RANGE_FINDER_H_
#define _AEROQUAD_MAX_SONAR_RANGE_FINDER_H_

// @see http://www.arduino.cc/playground/Main/MaxSonar

#if defined (__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(BOARD_aeroquad32)

#include "RangeFinder.h"

#define MB1000 0 // Maxbotix LV-MaxSonar-EZ*
#define MB1200 1 // Maxbotix XL-MaxSonar-EZ*

#define SPIKE_FILTER_MARGIN 500 // mm ; changes bigger than this need two samples to take effect

struct rangeFinder {
    byte target;      // {ALTITUDE,FRONT,RIGHT,REAR,LEFT}_RANGE_FINDER_INDEX
    byte pin;
    byte triggerpin;
    byte type;
  } rangeFinders[] = {
     // Define your rangers here
     // First ranger is given priority so it should be used for altitude
     // If using more than one ranger you should connect the 'trigger' to the 'RX' pin on the ranger.
     //
  //    { ALTITUDE_RANGE_FINDER_INDEX, A1, 24, MB1200}, 
    { ALTITUDE_RANGE_FINDER_INDEX, A1, 0, MB1000},
//	  { FRONT_RANGE_FINDER_INDEX,    A2, 25, MB1000},
//	  { RIGHT_RANGE_FINDER_INDEX,    A3, 26, MB1000},
//	  { REAR_RANGE_FINDER_INDEX,     A4, 27, MB1000},
//	  { LEFT_RANGE_FINDER_INDEX,     A5, 28, MB1000}
	};

// theoretical range at AIN=VCC
short rangerScale[] = { 
  13005, // MB10xx series
  10240, // MB12xx series
  };

// 50Hz cycles needed to wait for ranging
byte rangerWait[] = {
  2, // MB1000 needs 50ms i.e. wait 2 cycles (60ms)
  4, // MB1200 needs 100ms i.e. wait 5 cycles (100ms)
};

#define RANGER_COUNT ((sizeof(rangeFinders) / sizeof(struct rangeFinder)))

// last reading used for 'spike' filtter
short lastRange[RANGER_COUNT];

byte rangerWaitCycles = 0;

byte rangerSchedule = 0;

void inititalizeRangeFinders() {

  for (byte i = 0; i < RANGER_COUNT; i++) {

    rangeFinderRange[rangeFinders[i].target] = -1;
    if (rangeFinders[i].triggerpin) {
      digitalWrite(rangeFinders[i].triggerpin, LOW);
      pinMode(rangeFinders[i].triggerpin, OUTPUT);
    }
    lastRange[i] = 32000; 
    //    pinMode(rangeFinders[i].pin, INPUT);
  }
  rangerWaitCycles = 10; // allow to initialize
}

int a = 100;
short range =0;
void updateRangeFinders() {

  byte rangerToRead = 0;
  byte rangerToTrigger = 0;

  if (rangerWaitCycles) {
    rangerWaitCycles--;
    return;
  }

  if (RANGER_COUNT > 1) {
    if ((rangerSchedule & 1) == 0) {
      rangerToRead = 0;
      rangerToTrigger = (rangerSchedule >> 1) + 1;
    }
    else {
      rangerToRead = (rangerSchedule >> 1) + 1;
      rangerToTrigger = 0;
    }
    rangerSchedule++;
    if (((rangerSchedule>>1) + 1) >= (byte)RANGER_COUNT) {
      rangerSchedule = 0;
    }
  }

//test code//
  // step 1: instruct sensor to read echoes
  Wire.beginTransmission(112); // transmit to device #112 (0x70)
                               // the address specified in the datasheet is 224 (0xE0)
                               // but i2c adressing uses the high 7 bits so it's 112
  Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)  
  Wire.write(byte(0x50));      // command sensor to measure in "inches" (0x50) 
                               // use 0x51 for centimeters
                               // use 0x52 for ping microseconds
  Wire.endTransmission();      // stop transmitting

  // step 2: wait for readings to happen
  delay(70);                   // datasheet suggests at least 65 milliseconds

  // step 3: instruct sensor to return a particular echo reading
  Wire.beginTransmission(112); // transmit to device #112
  Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)
  Wire.endTransmission();      // stop transmitting

  // step 4: request reading from sensor
  Wire.requestFrom(112, 2);    // request 2 bytes from slave device #112

  int reading = 0;
  // step 5: receive reading from sensor
  if(2 <= Wire.available())    // if two bytes were received
  {
    reading = Wire.read();  // receive high byte (overwrites previous reading)
    reading = reading << 8;    // shift high byte to be high 8 bits
    reading |= Wire.read(); // receive low byte as lower 8 bits
  }
  
  range = reading*25.4;
  
  if ((abs(range - lastRange[rangerToRead]) < SPIKE_FILTER_MARGIN) ||
      (abs(range * 1000.0 - rangeFinderRange[rangeFinders[rangerToRead].target]) < SPIKE_FILTER_MARGIN)) {
    rangeFinderRange[rangeFinders[rangerToRead].target] = (float)range / 1000.0;
  }
  lastRange[rangerToRead] = range;
 
  rangerWaitCycles = rangerWait[rangeFinders[rangerToRead].type];

  if (rangeFinders[rangerToTrigger].triggerpin) {
    digitalWrite(rangeFinders[rangerToTrigger].triggerpin, LOW);
  }
}

#endif 
#endif








