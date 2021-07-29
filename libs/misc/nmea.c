/*
 * This file is part of Espruino, a JavaScript interpreter for Microcontrollers
 *
 * Copyright (C) 2019 Gordon Williams <gw@pur3.co.uk>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * ----------------------------------------------------------------------------
 * NMEA decoder
 * ----------------------------------------------------------------------------
 */

#include "nmea.h"
#include "jswrap_date.h"

char *nmea_next_comma(char *nmea) {
  while (*nmea && *nmea!=',') nmea++; // find the comma
  return nmea;
}
double nmea_decode_latlon(char *nmea, char *comma) {
  if (*nmea==',') return NAN; // no reading
  char *dp = nmea;
  while (*dp && *dp!='.' && *dp!=',') dp++; // find decimal pt
  *comma = 0;
  double minutes = stringToFloatWithRadix(&dp[-2], 10, NULL);
  *comma = ',';
  dp[-2] = 0;
  int x = stringToIntWithRadix(nmea, 10, NULL, NULL);
  return x+(minutes/60);
}
double nmea_decode_float(char *nmea, char *comma) {
  *comma = 0;
  double r = stringToFloat(nmea);
  *comma = ',';
  return r;
}
uint8_t nmea_decode_1(char *nmea) {
  return chtod(nmea[0]);
}
uint8_t nmea_decode_2(char *nmea) {
  return chtod(nmea[0])*10 + chtod(nmea[1]);
}
bool nmea_decode(NMEAFixInfo *gpsFix, const char *nmeaLine) {
  char buf[NMEA_MAX_SIZE];
  strcpy(buf, nmeaLine);
  char *nmea = buf, *nextComma;
  bool lastWasGSV = gpsFix->lastWasGSV;
  bool thisIsGSV = false;
  gpsFix->lastWasGSV = false;
  if (nmea[0]=='$' && nmea[1]=='G') {
    if (nmea[3]=='R' && nmea[4]=='M' && nmea[5]=='C') {
      // $GNRMC,161945.00,A,5139.11397,N,00116.07202,W,1.530,,190919,,,A*7E
      nmea = nmea_next_comma(nmea)+1;
      nextComma = nmea_next_comma(nmea);
      // time
      gpsFix->hour = nmea_decode_2(&nmea[0]);
      gpsFix->min = nmea_decode_2(&nmea[2]);
      gpsFix->sec = nmea_decode_2(&nmea[4]);
      gpsFix->ms = nmea_decode_2(&nmea[7]);
      // status
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);//?
      // lat + NS
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // lon + EW
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // speed
      gpsFix->speed = nmea_decode_float(nmea, nextComma)*1.852; // knots to km/h
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // course
      gpsFix->course = nmea_decode_float(nmea, nextComma);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // date
      if (nmea[0]==',') {
        gpsFix->day = 0;
        gpsFix->month = 0;
        gpsFix->year = 0;
      } else {
        gpsFix->day = nmea_decode_2(&nmea[0]);
        gpsFix->month = nmea_decode_2(&nmea[2]);
        gpsFix->year = nmea_decode_2(&nmea[4]);
      }
      // ....
    }
    if (nmea[3]=='G' && nmea[4]=='G' && nmea[5]=='A') {
      // $GNGGA,161945.00,5139.11397,N,00116.07202,W,1,06,1.29,71.1,M,47.0,M,,*64
      nmea = nmea_next_comma(nmea)+1;
      nextComma = nmea_next_comma(nmea);
      // time
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // LAT
      gpsFix->lat = nmea_decode_latlon(nmea, nextComma);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      if (*nmea=='S') gpsFix->lat=-gpsFix->lat;
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // LON
      gpsFix->lon = nmea_decode_latlon(nmea, nextComma);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      if (*nmea=='W') gpsFix->lon=-gpsFix->lon;
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // quality
      gpsFix->quality = nmea_decode_1(nmea);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // num satellites
      gpsFix->satellites = nmea_decode_2(nmea);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // dilution of precision
      gpsFix->hdop = nmea_decode_float(nmea, nextComma);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // altitude
      gpsFix->alt = nmea_decode_float(nmea, nextComma);
      nmea = nextComma+1; nextComma = nmea_next_comma(nmea);
      // ....
    }
    if (nmea[3]=='G' && nmea[4]=='S' && nmea[5]=='V') {
      // loads of cool data about what satellites we have and signal strength...
      thisIsGSV = true;
      gpsFix->lastWasGSV = true;
    }
  }
  /* When to create GPS data event?
  F18 (UBlox) GPS gives a bunch of data ending in GLL
    No fix:
      $GNRMC,...
      $GNVTG....
      $GNGGA,...
      $GNGSA,A,1....
      $GPGSV,1,1....
      $GNGLL....
    With fix:
      $GNRMC,....
      $GNVTG,....
      $GNGGA,....
      $GNGSA,A,2,08,...
      $GPGSV,2,1,08,...
      $GPGSV,2,2,08,...
      $GNGLL,...

  SMA Q3 gives data ending , or on some devices BDGSV:
    No fix:
      $GNGGA,....
      $GPGSV,1,1,00,0*65
      $BDGSV,1,1,00,0*74
    With fix:
      $GNGGA,....
      $GPGSV,3,1,...
      $GPGSV,3,2,...
      $GPGSV,3,3,..
      $BDGSV,1,1,00,0*74

  The thing they have in common is they have GSV, then some stuff after that
  we don't care about. So when that happens, trigger success
  */
  if (lastWasGSV && !thisIsGSV) {
    // Complete set of data received
    return true;
  }

  return false;
}


JsVar *nmea_to_jsVar(NMEAFixInfo *gpsFix) {
  JsVar *o = jsvNewObject();
  if (o) {
    jsvObjectSetChildAndUnLock(o, "lat", jsvNewFromFloat(gpsFix->lat));
    jsvObjectSetChildAndUnLock(o, "lon", jsvNewFromFloat(gpsFix->lon));
    jsvObjectSetChildAndUnLock(o, "alt", jsvNewFromFloat(gpsFix->alt));
    jsvObjectSetChildAndUnLock(o, "speed", jsvNewFromFloat(gpsFix->speed));
    jsvObjectSetChildAndUnLock(o, "course", jsvNewFromFloat(gpsFix->course));
    if (gpsFix->day) {
      CalendarDate date;
      date.day = gpsFix->day;
      date.month = gpsFix->month-1; // 1 based to 0 based
      date.year = 2000+gpsFix->year;
      TimeInDay td;
      td.daysSinceEpoch = fromCalenderDate(&date);
      td.hour = gpsFix->hour;
      td.min = gpsFix->min;
      td.sec = gpsFix->sec;
      td.ms = gpsFix->ms;
      td.zone = 0; // jsdGetTimeZone(); - no! GPS time is always in UTC :)
      jsvObjectSetChildAndUnLock(o, "time", jswrap_date_from_milliseconds(fromTimeInDay(&td)));
    } else {
      jsvObjectSetChildAndUnLock(o, "time", 0);
    }
    jsvObjectSetChildAndUnLock(o, "satellites", jsvNewFromInteger(gpsFix->satellites));
    jsvObjectSetChildAndUnLock(o, "fix", jsvNewFromInteger(gpsFix->quality));
    jsvObjectSetChildAndUnLock(o, "hdop", jsvNewFromFloat(gpsFix->hdop));
  }
  return o;
}
