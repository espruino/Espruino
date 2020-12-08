// Simple test from http://test262.ecmascript.org/#
// Just checking whether Espruino is capable of running the tests or not
// NOTE: RegEx checks have been removed here and 'result=1' was put at the end

var strict_mode = false; 
function testRun(id, path, description, codeString, result, error) {
  if (result!=="pass") {
      throw new Error("Test '" + path + "'failed: " + error);
  }
}

function testFinished() {
    //no-op
}
function compareArray(aExpected, aActual) {
    if (aActual.length != aExpected.length) {
        return false;
    }

    aExpected.sort();
    aActual.sort();

    var s;
    for (var i = 0; i < aExpected.length; i++) {
        if (aActual[i] !== aExpected[i]) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
function arrayContains(arr, expected) {
    var found;
    for (var i = 0; i < expected.length; i++) {
        found = false;
        for (var j = 0; j < arr.length; j++) {
            if (expected[i] === arr[j]) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
var supportsArrayIndexGettersOnArrays = undefined;
function fnSupportsArrayIndexGettersOnArrays() {
    if (typeof supportsArrayIndexGettersOnArrays !== "undefined") {
        return supportsArrayIndexGettersOnArrays;
    }

    supportsArrayIndexGettersOnArrays = false;

    if (fnExists(Object.defineProperty)) {
        var arr = [];
        Object.defineProperty(arr, "0", {
            get: function() {
                supportsArrayIndexGettersOnArrays = true;
                return 0;
            }
        });
        var res = arr[0];
    }

    return supportsArrayIndexGettersOnArrays;
}

//-----------------------------------------------------------------------------
var supportsArrayIndexGettersOnObjects = undefined;
function fnSupportsArrayIndexGettersOnObjects() {
    if (typeof supportsArrayIndexGettersOnObjects !== "undefined")
        return supportsArrayIndexGettersOnObjects;

    supportsArrayIndexGettersOnObjects = false;

    if (fnExists(Object.defineProperty)) {
        var obj = {};
        Object.defineProperty(obj, "0", {
            get: function() {
                supportsArrayIndexGettersOnObjects = true;
                return 0;
            }
        });
        var res = obj[0];
    }

    return supportsArrayIndexGettersOnObjects;
}

//-----------------------------------------------------------------------------
function ConvertToFileUrl(pathStr) {
    return "file:" + pathStr.replace(/\\/g, "/");
}

//-----------------------------------------------------------------------------
function fnExists(/*arguments*/) {
    for (var i = 0; i < arguments.length; i++) {
        if (typeof (arguments[i]) !== "function") return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
var __globalObject = Function("return this;")();
function fnGlobalObject() {
     return __globalObject;
}

//-----------------------------------------------------------------------------
function fnSupportsStrict() {
    "use strict";
    try {
        eval('with ({}) {}');
        return false;
    } catch (e) {
        return true;
    }
}

//-----------------------------------------------------------------------------
//Verify all attributes specified data property of given object:
//value, writable, enumerable, configurable
//If all attribute values are expected, return true, otherwise, return false
function dataPropertyAttributesAreCorrect(obj,
                                          name,
                                          value,
                                          writable,
                                          enumerable,
                                          configurable) {
    var attributesCorrect = true;

    if (obj[name] !== value) {
        if (typeof obj[name] === "number" &&
            isNaN(obj[name]) &&
            typeof value === "number" &&
            isNaN(value)) {
            // keep empty
        } else {
            attributesCorrect = false;
        }
    }

    try {
        if (obj[name] === "oldValue") {
            obj[name] = "newValue";
        } else {
            obj[name] = "OldValue";
        }
    } catch (we) {
    }

    var overwrited = false;
    if (obj[name] !== value) {
        if (typeof obj[name] === "number" &&
            isNaN(obj[name]) &&
            typeof value === "number" &&
            isNaN(value)) {
            // keep empty
        } else {
            overwrited = true;
        }
    }
    if (overwrited !== writable) {
        attributesCorrect = false;
    }

    var enumerated = false;
    for (var prop in obj) {
        if (obj.hasOwnProperty(prop) && prop === name) {
            enumerated = true;
        }
    }

    if (enumerated !== enumerable) {
        attributesCorrect = false;
    }


    var deleted = false;

    try {
        delete obj[name];
    } catch (de) {
    }
    if (!obj.hasOwnProperty(name)) {
        deleted = true;
    }
    if (deleted !== configurable) {
        attributesCorrect = false;
    }

    return attributesCorrect;
}

//-----------------------------------------------------------------------------
//Verify all attributes specified accessor property of given object:
//get, set, enumerable, configurable
//If all attribute values are expected, return true, otherwise, return false
function accessorPropertyAttributesAreCorrect(obj,
                                              name,
                                              get,
                                              set,
                                              setVerifyHelpProp,
                                              enumerable,
                                              configurable) {
    var attributesCorrect = true;

    if (get !== undefined) {
        if (obj[name] !== get()) {
            if (typeof obj[name] === "number" &&
                isNaN(obj[name]) &&
                typeof get() === "number" &&
                isNaN(get())) {
                // keep empty
            } else {
                attributesCorrect = false;
            }
        }
    } else {
        if (obj[name] !== undefined) {
            attributesCorrect = false;
        }
    }

    try {
        var desc = Object.getOwnPropertyDescriptor(obj, name);
        if (typeof desc.set === "undefined") {
            if (typeof set !== "undefined") {
                attributesCorrect = false;
            }
        } else {
            obj[name] = "toBeSetValue";
            if (obj[setVerifyHelpProp] !== "toBeSetValue") {
                attributesCorrect = false;
            }
        }
    } catch (se) {
        throw se;
    }


    var enumerated = false;
    for (var prop in obj) {
        if (obj.hasOwnProperty(prop) && prop === name) {
            enumerated = true;
        }
    }

    if (enumerated !== enumerable) {
        attributesCorrect = false;
    }


    var deleted = false;
    try {
        delete obj[name];
    } catch (de) {
        throw de;
    }
    if (!obj.hasOwnProperty(name)) {
        deleted = true;
    }
    if (deleted !== configurable) {
        attributesCorrect = false;
    }

    return attributesCorrect;
}

//-----------------------------------------------------------------------------
var NotEarlyErrorString = "NotEarlyError";
var EarlyErrorRePat = "^((?!" + NotEarlyErrorString + ").)*$";
var NotEarlyError = new Error(NotEarlyErrorString);

//-----------------------------------------------------------------------------
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

function Test262Error(message) {
    if (message) this.message = message;
}

Test262Error.prototype.toString = function () {
    return "Test262 Error: " + this.message;
};

function testFailed(message) {
    throw new Test262Error(message);
}


function testPrint(message) {

}


//adaptors for Test262 framework
function $PRINT(message) {

}

function $INCLUDE(message) { }
function $ERROR(message) {
    testFailed(message);
}

function $FAIL(message) {
    testFailed(message);
}



//Sputnik library definitions
//Ultimately these should be namespaced some how and only made
//available to tests that explicitly include them.
//For now, we just define the globally

//math_precision.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

function getPrecision(num) {
    //TODO: Create a table of prec's,
    //      because using Math for testing Math isn't that correct.

    var log2num = Math.log(Math.abs(num)) / Math.LN2;
    var pernum = Math.ceil(log2num);
    return (2 * Math.pow(2, -52 + pernum));
    //return(0);
}


//math_isequal.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

var prec;
function isEqual(num1, num2) {
    if ((num1 === Infinity) && (num2 === Infinity)) {
        return (true);
    }
    if ((num1 === -Infinity) && (num2 === -Infinity)) {
        return (true);
    }
    prec = getPrecision(Math.min(Math.abs(num1), Math.abs(num2)));
    return (Math.abs(num1 - num2) <= prec);
    //return(num1 === num2);
}

//numeric_conversion.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

function ToInteger(p) {
    var x = Number(p);

    if (isNaN(x)) {
        return +0;
    }

    if ((x === +0)
  || (x === -0)
  || (x === Number.POSITIVE_INFINITY)
  || (x === Number.NEGATIVE_INFINITY)) {
        return x;
    }

    var sign = (x < 0) ? -1 : 1;

    return (sign * Math.floor(Math.abs(x)));
}

//Date_constants.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

var HoursPerDay = 24;
var MinutesPerHour = 60;
var SecondsPerMinute = 60;

var msPerDay = 86400000;
var msPerSecond = 1000;
var msPerMinute = 60000;
var msPerHour = 3600000;

var date_1899_end = -2208988800001;
var date_1900_start = -2208988800000;
var date_1969_end = -1;
var date_1970_start = 0;
var date_1999_end = 946684799999;
var date_2000_start = 946684800000;
var date_2099_end = 4102444799999;
var date_2100_start = 4102444800000;

// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

//the following values are normally generated by the sputnik.py driver
var $LocalTZ,
    $DST_start_month,
    $DST_start_sunday,
    $DST_start_hour,
    $DST_start_minutes,
    $DST_end_month,
    $DST_end_sunday,
    $DST_end_hour,
    $DST_end_minutes;

(function () {
    /**
      * Finds the first date, starting from |start|, where |predicate|
      * holds.
      */
    var findNearestDateBefore = function(start, predicate) {
        var current = start;
        var month = 1000 * 60 * 60 * 24 * 30;
        for (var step = month; step > 0; step = Math.floor(step / 3)) {
            if (!predicate(current)) {
                while (!predicate(current))
                    current = new Date(current.getTime() + step);
                    current = new Date(current.getTime() - step);
                }
        }
        while (!predicate(current)) {
            current = new Date(current.getTime() + 1);
        }
        return current;
    };

    var juneDate = new Date(2000, 5, 20, 0, 0, 0, 0);
    var decemberDate = new Date(2000, 11, 20, 0, 0, 0, 0);
    var juneOffset = juneDate.getTimezoneOffset();
    var decemberOffset = decemberDate.getTimezoneOffset();
    var isSouthernHemisphere = (juneOffset > decemberOffset);
    var winterTime = isSouthernHemisphere ? juneDate : decemberDate;
    var summerTime = isSouthernHemisphere ? decemberDate : juneDate;

    var dstStart = findNearestDateBefore(winterTime, function (date) {
        return date.getTimezoneOffset() == summerTime.getTimezoneOffset();
    });
    $DST_start_month = dstStart.getMonth();
    $DST_start_sunday = dstStart.getDate() > 15 ? '"last"' : '"first"';
    $DST_start_hour = dstStart.getHours();
    $DST_start_minutes = dstStart.getMinutes();

    var dstEnd = findNearestDateBefore(summerTime, function (date) {
        return date.getTimezoneOffset() == winterTime.getTimezoneOffset();
    });
    $DST_end_month = dstEnd.getMonth();
    $DST_end_sunday = dstEnd.getDate() > 15 ? '"last"' : '"first"';
    $DST_end_hour = dstEnd.getHours();
    $DST_end_minutes = dstEnd.getMinutes();

    return;
})();


//Date.library.js
// Copyright 2009 the Sputnik authors.  All rights reserved.
// This code is governed by the BSD license found in the LICENSE file.

//15.9.1.2 Day Number and Time within Day
function Day(t) {
  return Math.floor(t/msPerDay);
}

function TimeWithinDay(t) {
  return t%msPerDay;
}

//15.9.1.3 Year Number
function DaysInYear(y){
  if(y%4 != 0) return 365;
  if(y%4 == 0 && y%100 != 0) return 366;
  if(y%100 == 0 && y%400 != 0) return 365;
  if(y%400 == 0) return 366;
}

function DayFromYear(y) {
  return (365*(y-1970)
          + Math.floor((y-1969)/4)
          - Math.floor((y-1901)/100)
          + Math.floor((y-1601)/400));
}

function TimeFromYear(y){
  return msPerDay*DayFromYear(y);
}

function YearFromTime(t) {
  t = Number(t);

  var sign = ( t < 0 ) ? -1 : 1;
  var year = ( sign < 0 ) ? 1969 : 1970;
  for(var time = 0;;year += sign){
    time = TimeFromYear(year);

    if(sign > 0 && time > t){
      year -= sign;
      break;
    }
    else if(sign < 0 && time <= t){
      break;
    }
  };
  return year;
}

function InLeapYear(t){
  if(DaysInYear(YearFromTime(t)) == 365)
    return 0;

  if(DaysInYear(YearFromTime(t)) == 366)
    return 1;
}

function DayWithinYear(t) {
  return Day(t)-DayFromYear(YearFromTime(t));
}

//15.9.1.4 Month Number
function MonthFromTime(t){
  var day = DayWithinYear(t);
  var leap = InLeapYear(t);

  if((0 <= day) && (day < 31)) return 0;
  if((31 <= day) && (day < (59+leap))) return 1;
  if(((59+leap) <= day) && (day < (90+leap))) return 2;
  if(((90+leap) <= day) && (day < (120+leap))) return 3;
  if(((120+leap) <= day) && (day < (151+leap))) return 4;
  if(((151+leap) <= day) && (day < (181+leap))) return 5;
  if(((181+leap) <= day) && (day < (212+leap))) return 6;
  if(((212+leap) <= day) && (day < (243+leap))) return 7;
  if(((243+leap) <= day) && (day < (273+leap))) return 8;
  if(((273+leap) <= day) && (day < (304+leap))) return 9;
  if(((304+leap) <= day) && (day < (334+leap))) return 10;
  if(((334+leap) <= day) && (day < (365+leap))) return 11;
}

//15.9.1.5 Date Number
function DateFromTime(t) {
  var day = DayWithinYear(t);
  var month = MonthFromTime(t);
  var leap = InLeapYear(t);

  if(month == 0) return day+1;
  if(month == 1) return day-30;
  if(month == 2) return day-58-leap;
  if(month == 3) return day-89-leap;
  if(month == 4) return day-119-leap;
  if(month == 5) return day-150-leap;
  if(month == 6) return day-180-leap;
  if(month == 7) return day-211-leap;
  if(month == 8) return day-242-leap;
  if(month == 9) return day-272-leap;
  if(month == 10) return day-303-leap;
  if(month == 11) return day-333-leap;
}

//15.9.1.6 Week Day
function WeekDay(t) {
  var weekday = (Day(t)+4)%7;
  return (weekday < 0 ? 7+weekday : weekday);
}

//15.9.1.9 Daylight Saving Time Adjustment
$LocalTZ = (new Date()).getTimezoneOffset() / -60;
if (DaylightSavingTA((new Date()).valueOf()) !== 0) {
   $LocalTZ -= 1;
}
var LocalTZA = $LocalTZ*msPerHour;

function DaysInMonth(m, leap) {
  m = m%12;

  //April, June, Sept, Nov
  if(m == 3 || m == 5 || m == 8 || m == 10 ) {
    return 30;
  }

  //Jan, March, May, July, Aug, Oct, Dec
  if(m == 0 || m == 2 || m == 4 || m == 6 || m == 7 || m == 9 || m == 11){
    return 31;
  }

  //Feb
  return 28+leap;
}

function GetSundayInMonth(t, m, count){
    var year = YearFromTime(t);
    var tempDate;

    if (count==='"first"') {
        for (var d=1; d <= DaysInMonth(m, InLeapYear(t)); d++) {
            tempDate = new Date(year, m, d);
            if (tempDate.getDay()===0) {
                return tempDate.valueOf();
            }
        }
    } else if(count==='"last"') {
        for (var d=DaysInMonth(m, InLeapYear(t)); d>0; d--) {
            tempDate = new Date(year, m, d);
            if (tempDate.getDay()===0) {
                return tempDate.valueOf();
            }
        }
    }
    throw new Error("Unsupported 'count' arg:" + count);
}
/*
function GetSundayInMonth(t, m, count){
  var year = YearFromTime(t);
  var leap = InLeapYear(t);
  var day = 0;

  if(m >= 1) day += DaysInMonth(0, leap);
  if(m >= 2) day += DaysInMonth(1, leap);
  if(m >= 3) day += DaysInMonth(2, leap);
  if(m >= 4) day += DaysInMonth(3, leap);
  if(m >= 5) day += DaysInMonth(4, leap);
  if(m >= 6) day += DaysInMonth(5, leap);
  if(m >= 7) day += DaysInMonth(6, leap);
  if(m >= 8) day += DaysInMonth(7, leap);
  if(m >= 9) day += DaysInMonth(8, leap);
  if(m >= 10) day += DaysInMonth(9, leap);
  if(m >= 11) day += DaysInMonth(10, leap);

  var month_start = TimeFromYear(year)+day*msPerDay;
  var sunday = 0;

  if(count === "last"){
    for(var last_sunday = month_start+DaysInMonth(m, leap)*msPerDay;
      WeekDay(last_sunday)>0;
      last_sunday -= msPerDay
    ){};
    sunday = last_sunday;
  }
  else {
    for(var first_sunday = month_start;
      WeekDay(first_sunday)>0;
      first_sunday += msPerDay
    ){};
    sunday = first_sunday+7*msPerDay*(count-1);
  }

  return sunday;
}*/

function DaylightSavingTA(t) {
//  t = t-LocalTZA;

  var DST_start = GetSundayInMonth(t, $DST_start_month, $DST_start_sunday) +
                  $DST_start_hour*msPerHour +
                  $DST_start_minutes*msPerMinute;

  var k = new Date(DST_start);

  var DST_end   = GetSundayInMonth(t, $DST_end_month, $DST_end_sunday) +
                  $DST_end_hour*msPerHour +
                  $DST_end_minutes*msPerMinute;

  if ( t >= DST_start && t < DST_end ) {
    return msPerHour;
  } else {
    return 0;
  }
}

//15.9.1.9 Local Time
function LocalTime(t){
  return t+LocalTZA+DaylightSavingTA(t);
}

function UTC(t) {
  return t-LocalTZA-DaylightSavingTA(t-LocalTZA);
}

//15.9.1.10 Hours, Minutes, Second, and Milliseconds
function HourFromTime(t){
  return Math.floor(t/msPerHour)%HoursPerDay;
}

function MinFromTime(t){
  return Math.floor(t/msPerMinute)%MinutesPerHour;
}

function SecFromTime(t){
  return Math.floor(t/msPerSecond)%SecondsPerMinute;
}

function msFromTime(t){
  return t%msPerSecond;
}

//15.9.1.11 MakeTime (hour, min, sec, ms)
function MakeTime(hour, min, sec, ms){
  if ( !isFinite(hour) || !isFinite(min) || !isFinite(sec) || !isFinite(ms)) {
    return Number.NaN;
  }

  hour = ToInteger(hour);
  min  = ToInteger(min);
  sec  = ToInteger(sec);
  ms   = ToInteger(ms);

  return ((hour*msPerHour) + (min*msPerMinute) + (sec*msPerSecond) + ms);
}

//15.9.1.12 MakeDay (year, month, date)
function MakeDay(year, month, date) {
  if ( !isFinite(year) || !isFinite(month) || !isFinite(date)) {
    return Number.NaN;
  }

  year = ToInteger(year);
  month = ToInteger(month);
  date = ToInteger(date );

  var result5 = year + Math.floor(month/12);
  var result6 = month%12;

  var sign = ( year < 1970 ) ? -1 : 1;
  var t =    ( year < 1970 ) ? 1 :  0;
  var y =    ( year < 1970 ) ? 1969 : 1970;

  if( sign == -1 ){
    for ( y = 1969; y >= year; y += sign ) {
      t += sign * DaysInYear(y)*msPerDay;
    }
  } else {
    for ( y = 1970 ; y < year; y += sign ) {
      t += sign * DaysInYear(y)*msPerDay;
    }
  }

  var leap = 0;
  for ( var m = 0; m < month; m++ ) {
    //if year is changed, than we need to recalculate leep
    leap = InLeapYear(t);
    t += DaysInMonth(m, leap)*msPerDay;
  }

  if ( YearFromTime(t) != result5 ) {
    return Number.NaN;
  }
  if ( MonthFromTime(t) != result6 ) {
    return Number.NaN;
  }
  if ( DateFromTime(t) != 1 ) {
    return Number.NaN;
  }

  return Day(t)+date-1;
}

//15.9.1.13 MakeDate (day, time)
function MakeDate( day, time ) {
  if(!isFinite(day) || !isFinite(time)) {
    return Number.NaN;
  }

  return day*msPerDay+time;
}

//15.9.1.14 TimeClip (time)
function TimeClip(time) {
  if(!isFinite(time) || Math.abs(time) > 8.64e15){
    return Number.NaN;
  }

  return ToInteger(time);
}

//Test Functions
//ConstructDate is considered deprecated, and should not be used directly from
//test262 tests as it's incredibly sensitive to DST start/end dates that 
//vary with geographic location.
function ConstructDate(year, month, date, hours, minutes, seconds, ms){
  /*
   * 1. Call ToNumber(year)
   * 2. Call ToNumber(month)
   * 3. If date is supplied use ToNumber(date); else use 1
   * 4. If hours is supplied use ToNumber(hours); else use 0
   * 5. If minutes is supplied use ToNumber(minutes); else use 0
   * 6. If seconds is supplied use ToNumber(seconds); else use 0
   * 7. If ms is supplied use ToNumber(ms); else use 0
   * 8. If Result(1) is not NaN and 0 <= ToInteger(Result(1)) <= 99, Result(8) is
   * 1900+ToInteger(Result(1)); otherwise, Result(8) is Result(1)
   * 9. Compute MakeDay(Result(8), Result(2), Result(3))
   * 10. Compute MakeTime(Result(4), Result(5), Result(6), Result(7))
   * 11. Compute MakeDate(Result(9), Result(10))
   * 12. Set the [[Value]] property of the newly constructed object to TimeClip(UTC(Result(11)))
   */
  var r1 = Number(year);
  var r2 = Number(month);
  var r3 = ((date && arguments.length > 2) ? Number(date) : 1);
  var r4 = ((hours && arguments.length > 3) ? Number(hours) : 0);
  var r5 = ((minutes && arguments.length > 4) ? Number(minutes) : 0);
  var r6 = ((seconds && arguments.length > 5) ? Number(seconds) : 0);
  var r7 = ((ms && arguments.length > 6) ? Number(ms) : 0);

  var r8 = r1;

  if(!isNaN(r1) && (0 <= ToInteger(r1)) && (ToInteger(r1) <= 99))
    r8 = 1900+r1;

  var r9 = MakeDay(r8, r2, r3);
  var r10 = MakeTime(r4, r5, r6, r7);
  var r11 = MakeDate(r9, r10);

  var retVal = TimeClip(UTC(r11));
  return retVal;
}



/**** Python code for initialize the above constants
// We may want to replicate the following in JavaScript.
// However, using JS date operations to generate parameters that are then used to
// test those some date operations seems unsound.  However, it isn't clear if there
//is a good interoperable alternative.

# Copyright 2009 the Sputnik authors.  All rights reserved.
# This code is governed by the BSD license found in the LICENSE file.

def GetDaylightSavingsTimes():
# Is the given floating-point time in DST?
def IsDst(t):
return time.localtime(t)[-1]
# Binary search to find an interval between the two times no greater than
# delta where DST switches, returning the midpoint.
def FindBetween(start, end, delta):
while end - start > delta:
middle = (end + start) / 2
if IsDst(middle) == IsDst(start):
start = middle
else:
end = middle
return (start + end) / 2
now = time.time()
one_month = (30 * 24 * 60 * 60)
# First find a date with different daylight savings.  To avoid corner cases
# we try four months before and after today.
after = now + 4 * one_month
before = now - 4 * one_month
if IsDst(now) == IsDst(before) and IsDst(now) == IsDst(after):
logger.warning("Was unable to determine DST info.")
return None
# Determine when the change occurs between now and the date we just found
# in a different DST.
if IsDst(now) != IsDst(before):
first = FindBetween(before, now, 1)
else:
first = FindBetween(now, after, 1)
# Determine when the change occurs between three and nine months from the
# first.
second = FindBetween(first + 3 * one_month, first + 9 * one_month, 1)
# Find out which switch is into and which if out of DST
if IsDst(first - 1) and not IsDst(first + 1):
start = second
end = first
else:
start = first
end = second
return (start, end)


def GetDaylightSavingsAttribs():
times = GetDaylightSavingsTimes()
if not times:
return None
(start, end) = times
def DstMonth(t):
return time.localtime(t)[1] - 1
def DstHour(t):
return time.localtime(t - 1)[3] + 1
def DstSunday(t):
if time.localtime(t)[2] > 15:
return "'last'"
else:
return "'first'"
def DstMinutes(t):
return (time.localtime(t - 1)[4] + 1) % 60
attribs = { }
attribs['start_month'] = DstMonth(start)
attribs['end_month'] = DstMonth(end)
attribs['start_sunday'] = DstSunday(start)
attribs['end_sunday'] = DstSunday(end)
attribs['start_hour'] = DstHour(start)
attribs['end_hour'] = DstHour(end)
attribs['start_minutes'] = DstMinutes(start)
attribs['end_minutes'] = DstMinutes(end)
return attribs

*********/

//--Test case registration-----------------------------------------------------
function runTestCase(testcase) {
    if (testcase() !== true) {
        $ERROR("Test case returned non-true value!");
    }
}

if (this.window!==undefined) {  //for console support
    this.window.onerror = function(errorMsg, url, lineNumber) {
        this.window.iframeError = errorMsg;
    };
}

//This doesn't work with early errors in current versions of Opera
/*
if (/opera/i.test(navigator.userAgent)) {
    (function() {
        var origError = window.Error;
        window.Error = function() {
            if (arguments.length>0) {
                try {
                    window.onerror(arguments[0]);
                } catch(e) {
                    alert("Failed to invoke window.onerror (from ed.js)");
                }
            }
            return origError.apply(this, arguments);
        }
    })();
}*/
/**
 * @description Tests that obj meets the requirements for built-in objects
 *     defined by the introduction of chapter 15 of the ECMAScript Language Specification.
 * @param {Object} obj the object to be tested.
 * @param {boolean} isFunction whether the specification describes obj as a function.
 * @param {boolean} isConstructor whether the specification describes obj as a constructor.
 * @param {String[]} properties an array with the names of the built-in properties of obj,
 *     excluding length, prototype, or properties with non-default attributes.
 * @param {number} length for functions only: the length specified for the function
 *     or derived from the argument list.
 * @author Norbert Lindenberg
 */

function testBuiltInObject(obj, isFunction, isConstructor, properties, length) {

    if (obj === undefined) {
        $ERROR("Object being tested is undefined.");
    }

    var objString = Object.prototype.toString.call(obj);
    if (isFunction) {
        if (objString !== "[object Function]") {
            $ERROR("The [[Class]] internal property of a built-in function must be " +
                    "\"Function\", but toString() returns " + objString);
        }
    } else {
        if (objString !== "[object Object]") {
            $ERROR("The [[Class]] internal property of a built-in non-function object must be " +
                    "\"Object\", but toString() returns " + objString);
        }
    }

    if (!Object.isExtensible(obj)) {
        $ERROR("Built-in objects must be extensible.");
    }

    if (isFunction && Object.getPrototypeOf(obj) !== Function.prototype) {
        $ERROR("Built-in functions must have Function.prototype as their prototype.");
    }

    if (isConstructor && Object.getPrototypeOf(obj.prototype) !== Object.prototype) {
        $ERROR("Built-in prototype objects must have Object.prototype as their prototype.");
    }

    // verification of the absence of the [[Construct]] internal property has
    // been moved to the end of the test
    
    // verification of the absence of the prototype property has
    // been moved to the end of the test

    if (isFunction) {
        
        if (typeof obj.length !== "number" || obj.length !== Math.floor(obj.length)) {
            $ERROR("Built-in functions must have a length property with an integer value.");
        }
    
        if (obj.length !== length) {
            $ERROR("Function's length property doesn't have specified value; expected " +
                length + ", got " + obj.length + ".");
        }

        var desc = Object.getOwnPropertyDescriptor(obj, "length");
        if (desc.writable) {
            $ERROR("The length property of a built-in function must not be writable.");
        }
        if (desc.enumerable) {
            $ERROR("The length property of a built-in function must not be enumerable.");
        }
        if (desc.configurable) {
            $ERROR("The length property of a built-in function must not be configurable.");
        }
    }

    properties.forEach(function(prop) {
        var desc = Object.getOwnPropertyDescriptor(obj, prop);
        if (desc === undefined) {
            $ERROR("Missing property " + prop + ".");
        }
        // accessor properties don't have writable attribute
        if (desc.hasOwnProperty("writable") && !desc.writable) {
            $ERROR("The " + prop + " property of this built-in function must be writable.");
        }
        if (desc.enumerable) {
            $ERROR("The " + prop + " property of this built-in function must not be enumerable.");
        }
        if (!desc.configurable) {
            $ERROR("The " + prop + " property of this built-in function must be configurable.");
        }
    });

    // The remaining sections have been moved to the end of the test because
    // unbound non-constructor functions written in JavaScript cannot possibly
    // pass them, and we still want to test JavaScript implementations as much
    // as possible.
    
    var exception;
    if (isFunction && !isConstructor) {
        // this is not a complete test for the presence of [[Construct]]:
        // if it's absent, the exception must be thrown, but it may also
        // be thrown if it's present and just has preconditions related to
        // arguments or the this value that this statement doesn't meet.
        try {
            /*jshint newcap:false*/
            var instance = new obj();
        } catch (e) {
            exception = e;
        }
        if (exception === undefined || exception.name !== "TypeError") {
            $ERROR("Built-in functions that aren't constructors must throw TypeError when " +
                "used in a \"new\" statement.");
        }
    }

    if (isFunction && !isConstructor && obj.hasOwnProperty("prototype")) {
        $ERROR("Built-in functions that aren't constructors must not have a prototype property.");
    }

    // passed the complete test!
    return true;
}


/**
 * This file contains shared functions for the tests in the conformance test
 * suite for the ECMAScript Internationalization API.
 * @author Norbert Lindenberg
 */


/**
 * @description Calls the provided function for every service constructor in
 * the Intl object, until f returns a falsy value. It returns the result of the
 * last call to f, mapped to a boolean.
 * @param {Function} f the function to call for each service constructor in
 *     the Intl object.
 *     @param {Function} Constructor the constructor object to test with.
 * @result {Boolean} whether the test succeeded.
 */
function testWithIntlConstructors(f) {
    var constructors = ["Collator", "NumberFormat", "DateTimeFormat"];
    return constructors.every(function (constructor) {
        var Constructor = Intl[constructor];
        var result;
        try {
            result = f(Constructor);
        } catch (e) {
            e.message += " (Testing with " + constructor + ".)";
            throw e;
        }
        return result;
    });
}


/**
 * Returns the name of the given constructor object, which must be one of
 * Intl.Collator, Intl.NumberFormat, or Intl.DateTimeFormat.
 * @param {object} Constructor a constructor
 * @return {string} the name of the constructor
 */
function getConstructorName(Constructor) {
    switch (Constructor) {
        case Intl.Collator:
            return "Collator";
        case Intl.NumberFormat:
            return "NumberFormat";
        case Intl.DateTimeFormat:
            return "DateTimeFormat";
        default:
            $ERROR("test internal error: unknown Constructor");
    }
}


/**
 * Taints a named data property of the given object by installing
 * a setter that throws an exception.
 * @param {object} obj the object whose data property to taint
 * @param {string} property the property to taint
 */
function taintDataProperty(obj, property) {
    Object.defineProperty(obj, property, {
        set: function(value) {
            $ERROR("Client code can adversely affect behavior: setter for " + property + ".");
        },
        enumerable: false,
        configurable: true
    });
}


/**
 * Taints a named method of the given object by replacing it with a function
 * that throws an exception.
 * @param {object} obj the object whose method to taint
 * @param {string} property the name of the method to taint
 */
function taintMethod(obj, property) {
    Object.defineProperty(obj, property, {
        value: function() {
            $ERROR("Client code can adversely affect behavior: method " + property + ".");
        },
        writable: true,
        enumerable: false,
        configurable: true
    });
}


/**
 * Taints the given properties (and similarly named properties) by installing
 * setters on Object.prototype that throw exceptions.
 * @param {Array} properties an array of property names to taint
 */
function taintProperties(properties) {
    properties.forEach(function (property) {
        var adaptedProperties = [property, "__" + property, "_" + property, property + "_", property + "__"];
        adaptedProperties.forEach(function (property) {
            taintDataProperty(Object.prototype, property);
        });
    });
}


/**
 * Taints the Array object by creating a setter for the property "0" and
 * replacing some key methods with functions that throw exceptions.
 */
function taintArray() {
    taintDataProperty(Array.prototype, "0");
    taintMethod(Array.prototype, "indexOf");
    taintMethod(Array.prototype, "join");
    taintMethod(Array.prototype, "push");
    taintMethod(Array.prototype, "slice");
    taintMethod(Array.prototype, "sort");
}


// auxiliary data for getLocaleSupportInfo
var languages = ["zh", "es", "en", "hi", "ur", "ar", "ja", "pa"];
var scripts = ["Latn", "Hans", "Deva", "Arab", "Jpan", "Hant"];
var countries = ["CN", "IN", "US", "PK", "JP", "TW", "HK", "SG"];
var localeSupportInfo = {};


/**
 * Gets locale support info for the given constructor object, which must be one
 * of Intl.Collator, Intl.NumberFormat, Intl.DateTimeFormat.
 * @param {object} Constructor the constructor for which to get locale support info
 * @return {object} locale support info with the following properties:
 *     supported: array of fully supported language tags
 *     byFallback: array of language tags that are supported through fallbacks
 *     unsupported: array of unsupported language tags
 */
function getLocaleSupportInfo(Constructor) {
    var constructorName = getConstructorName(Constructor);
    if (localeSupportInfo[constructorName] !== undefined) {
        return localeSupportInfo[constructorName];
    }

    var allTags = [];
    var i, j, k;
    var language, script, country;
    for (i = 0; i < languages.length; i++) {
        language = languages[i];
        allTags.push(language);
        for (j = 0; j < scripts.length; j++) {
            script = scripts[j];
            allTags.push(language + "-" + script);
            for (k = 0; k < countries.length; k++) {
                country = countries[k];
                allTags.push(language + "-" + script + "-" + country);
            }
        }
        for (k = 0; k < countries.length; k++) {
            country = countries[k];
            allTags.push(language + "-" + country);
        }
    }
    
    var supported = [];
    var byFallback = [];
    var unsupported = [];
    for (i = 0; i < allTags.length; i++) {
        var request = allTags[i];
        var result = new Constructor([request], {localeMatcher: "lookup"}).resolvedOptions().locale;
         if (request === result) {
            supported.push(request);
        } else if (request.indexOf(result) === 0) {
            byFallback.push(request);
        } else {
            unsupported.push(request);
        }
    }
    
    localeSupportInfo[constructorName] = {
        supported: supported,
        byFallback: byFallback,
        unsupported: unsupported
    };
    
    return localeSupportInfo[constructorName];
}
        

/**
 * @description Tests whether locale is a String value representing a
 * structurally valid and canonicalized BCP 47 language tag, as defined in
 * sections 6.2.2 and 6.2.3 of the ECMAScript Internationalization API
 * Specification.
 * @param {String} locale the string to be tested.
 * @result {Boolean} whether the test succeeded.
 */
function isCanonicalizedStructurallyValidLanguageTag(locale) {

    /**
     * Regular expression defining BCP 47 language tags.
     *
     * Spec: RFC 5646 section 2.1.
     */
    var alpha = "[a-zA-Z]",
        digit = "[0-9]",
        alphanum = "(" + alpha + "|" + digit + ")",
        regular = "(art-lojban|cel-gaulish|no-bok|no-nyn|zh-guoyu|zh-hakka|zh-min|zh-min-nan|zh-xiang)",
        irregular = "(en-GB-oed|i-ami|i-bnn|i-default|i-enochian|i-hak|i-klingon|i-lux|i-mingo|i-navajo|i-pwn|i-tao|i-tay|i-tsu|sgn-BE-FR|sgn-BE-NL|sgn-CH-DE)",
        grandfathered = "(" + irregular + "|" + regular + ")",
        privateuse = "(x(-[a-z0-9]{1,8})+)",
        singleton = "(" + digit + "|[A-WY-Za-wy-z])",
        extension = "(" + singleton + "(-" + alphanum + "{2,8})+)",
        variant = "(" + alphanum + "{5,8}|(" + digit + alphanum + "{3}))",
        region = "(" + alpha + "{2}|" + digit + "{3})",
        script = "(" + alpha + "{4})",
        extlang = "(" + alpha + "{3}(-" + alpha + "{3}){0,2})",
        language = "(" + alpha + "{2,3}(-" + extlang + ")?|" + alpha + "{4}|" + alpha + "{5,8})",
        langtag = language + "(-" + script + ")?(-" + region + ")?(-" + variant + ")*(-" + extension + ")*(-" + privateuse + ")?",
        languageTag = "^(" + langtag + "|" + privateuse + "|" + grandfathered + ")$",
        languageTagRE = new RegExp(languageTag, "i");
    var duplicateSingleton = "-" + singleton + "-(.*-)?\\1(?!" + alphanum + ")",
        duplicateSingletonRE = new RegExp(duplicateSingleton, "i"),
        duplicateVariant = "(" + alphanum + "{2,8}-)+" + variant + "-(" + alphanum + "{2,8}-)*\\3(?!" + alphanum + ")",
        duplicateVariantRE = new RegExp(duplicateVariant, "i");


    /**
     * Verifies that the given string is a well-formed BCP 47 language tag
     * with no duplicate variant or singleton subtags.
     *
     * Spec: ECMAScript Internationalization API Specification, draft, 6.2.2.
     */
    function isStructurallyValidLanguageTag(locale) {
        if (!languageTagRE.test(locale)) {
            return false;
        }
        locale = locale.split(/-x-/)[0];
        return !duplicateSingletonRE.test(locale) && !duplicateVariantRE.test(locale);
    }


    /**
     * Mappings from complete tags to preferred values.
     *
     * Spec: IANA Language Subtag Registry.
     */
    var __tagMappings = {
        // property names must be in lower case; values in canonical form

        // grandfathered tags from IANA language subtag registry, file date 2011-08-25
        "art-lojban": "jbo",
        "cel-gaulish": "cel-gaulish",
        "en-gb-oed": "en-GB-oed",
        "i-ami": "ami",
        "i-bnn": "bnn",
        "i-default": "i-default",
        "i-enochian": "i-enochian",
        "i-hak": "hak",
        "i-klingon": "tlh",
        "i-lux": "lb",
        "i-mingo": "i-mingo",
        "i-navajo": "nv",
        "i-pwn": "pwn",
        "i-tao": "tao",
        "i-tay": "tay",
        "i-tsu": "tsu",
        "no-bok": "nb",
        "no-nyn": "nn",
        "sgn-be-fr": "sfb",
        "sgn-be-nl": "vgt",
        "sgn-ch-de": "sgg",
        "zh-guoyu": "cmn",
        "zh-hakka": "hak",
        "zh-min": "zh-min",
        "zh-min-nan": "nan",
        "zh-xiang": "hsn",
        // deprecated redundant tags from IANA language subtag registry, file date 2011-08-25
        "sgn-br": "bzs",
        "sgn-co": "csn",
        "sgn-de": "gsg",
        "sgn-dk": "dsl",
        "sgn-es": "ssp",
        "sgn-fr": "fsl",
        "sgn-gb": "bfi",
        "sgn-gr": "gss",
        "sgn-ie": "isg",
        "sgn-it": "ise",
        "sgn-jp": "jsl",
        "sgn-mx": "mfs",
        "sgn-ni": "ncs",
        "sgn-nl": "dse",
        "sgn-no": "nsl",
        "sgn-pt": "psr",
        "sgn-se": "swl",
        "sgn-us": "ase",
        "sgn-za": "sfs",
        "zh-cmn": "cmn",
        "zh-cmn-hans": "cmn-Hans",
        "zh-cmn-hant": "cmn-Hant",
        "zh-gan": "gan",
        "zh-wuu": "wuu",
        "zh-yue": "yue",
        // deprecated variant with prefix from IANA language subtag registry, file date 2011-08-25
        "ja-latn-hepburn-heploc": "ja-Latn-alalc97"
    };


    /**
     * Mappings from non-extlang subtags to preferred values.
     *
     * Spec: IANA Language Subtag Registry.
     */
    var __subtagMappings = {
        // property names and values must be in canonical case
        // language subtags with Preferred-Value mappings from IANA language subtag registry, file date 2011-08-25
        "in": "id",
        "iw": "he",
        "ji": "yi",
        "jw": "jv",
        "mo": "ro",
        "ayx": "nun",
        "cjr": "mom",
        "cmk": "xch",
        "drh": "khk",
        "drw": "prs",
        "gav": "dev",
        "mst": "mry",
        "myt": "mry",
        "tie": "ras",
        "tkk": "twm",
        "tnf": "prs",
        // region subtags with Preferred-Value mappings from IANA language subtag registry, file date 2011-08-25
        "BU": "MM",
        "DD": "DE",
        "FX": "FR",
        "TP": "TL",
        "YD": "YE",
        "ZR": "CD"
    };


    /**
     * Mappings from extlang subtags to preferred values.
     *
     * Spec: IANA Language Subtag Registry.
     */
    var __extlangMappings = {
        // extlang subtags with Preferred-Value mappings from IANA language subtag registry, file date 2011-08-25
        // values are arrays with [0] the replacement value, [1] (if present) the prefix to be removed
        "aao": ["aao", "ar"],
        "abh": ["abh", "ar"],
        "abv": ["abv", "ar"],
        "acm": ["acm", "ar"],
        "acq": ["acq", "ar"],
        "acw": ["acw", "ar"],
        "acx": ["acx", "ar"],
        "acy": ["acy", "ar"],
        "adf": ["adf", "ar"],
        "ads": ["ads", "sgn"],
        "aeb": ["aeb", "ar"],
        "aec": ["aec", "ar"],
        "aed": ["aed", "sgn"],
        "aen": ["aen", "sgn"],
        "afb": ["afb", "ar"],
        "afg": ["afg", "sgn"],
        "ajp": ["ajp", "ar"],
        "apc": ["apc", "ar"],
        "apd": ["apd", "ar"],
        "arb": ["arb", "ar"],
        "arq": ["arq", "ar"],
        "ars": ["ars", "ar"],
        "ary": ["ary", "ar"],
        "arz": ["arz", "ar"],
        "ase": ["ase", "sgn"],
        "asf": ["asf", "sgn"],
        "asp": ["asp", "sgn"],
        "asq": ["asq", "sgn"],
        "asw": ["asw", "sgn"],
        "auz": ["auz", "ar"],
        "avl": ["avl", "ar"],
        "ayh": ["ayh", "ar"],
        "ayl": ["ayl", "ar"],
        "ayn": ["ayn", "ar"],
        "ayp": ["ayp", "ar"],
        "bbz": ["bbz", "ar"],
        "bfi": ["bfi", "sgn"],
        "bfk": ["bfk", "sgn"],
        "bjn": ["bjn", "ms"],
        "bog": ["bog", "sgn"],
        "bqn": ["bqn", "sgn"],
        "bqy": ["bqy", "sgn"],
        "btj": ["btj", "ms"],
        "bve": ["bve", "ms"],
        "bvl": ["bvl", "sgn"],
        "bvu": ["bvu", "ms"],
        "bzs": ["bzs", "sgn"],
        "cdo": ["cdo", "zh"],
        "cds": ["cds", "sgn"],
        "cjy": ["cjy", "zh"],
        "cmn": ["cmn", "zh"],
        "coa": ["coa", "ms"],
        "cpx": ["cpx", "zh"],
        "csc": ["csc", "sgn"],
        "csd": ["csd", "sgn"],
        "cse": ["cse", "sgn"],
        "csf": ["csf", "sgn"],
        "csg": ["csg", "sgn"],
        "csl": ["csl", "sgn"],
        "csn": ["csn", "sgn"],
        "csq": ["csq", "sgn"],
        "csr": ["csr", "sgn"],
        "czh": ["czh", "zh"],
        "czo": ["czo", "zh"],
        "doq": ["doq", "sgn"],
        "dse": ["dse", "sgn"],
        "dsl": ["dsl", "sgn"],
        "dup": ["dup", "ms"],
        "ecs": ["ecs", "sgn"],
        "esl": ["esl", "sgn"],
        "esn": ["esn", "sgn"],
        "eso": ["eso", "sgn"],
        "eth": ["eth", "sgn"],
        "fcs": ["fcs", "sgn"],
        "fse": ["fse", "sgn"],
        "fsl": ["fsl", "sgn"],
        "fss": ["fss", "sgn"],
        "gan": ["gan", "zh"],
        "gom": ["gom", "kok"],
        "gse": ["gse", "sgn"],
        "gsg": ["gsg", "sgn"],
        "gsm": ["gsm", "sgn"],
        "gss": ["gss", "sgn"],
        "gus": ["gus", "sgn"],
        "hab": ["hab", "sgn"],
        "haf": ["haf", "sgn"],
        "hak": ["hak", "zh"],
        "hds": ["hds", "sgn"],
        "hji": ["hji", "ms"],
        "hks": ["hks", "sgn"],
        "hos": ["hos", "sgn"],
        "hps": ["hps", "sgn"],
        "hsh": ["hsh", "sgn"],
        "hsl": ["hsl", "sgn"],
        "hsn": ["hsn", "zh"],
        "icl": ["icl", "sgn"],
        "ils": ["ils", "sgn"],
        "inl": ["inl", "sgn"],
        "ins": ["ins", "sgn"],
        "ise": ["ise", "sgn"],
        "isg": ["isg", "sgn"],
        "isr": ["isr", "sgn"],
        "jak": ["jak", "ms"],
        "jax": ["jax", "ms"],
        "jcs": ["jcs", "sgn"],
        "jhs": ["jhs", "sgn"],
        "jls": ["jls", "sgn"],
        "jos": ["jos", "sgn"],
        "jsl": ["jsl", "sgn"],
        "jus": ["jus", "sgn"],
        "kgi": ["kgi", "sgn"],
        "knn": ["knn", "kok"],
        "kvb": ["kvb", "ms"],
        "kvk": ["kvk", "sgn"],
        "kvr": ["kvr", "ms"],
        "kxd": ["kxd", "ms"],
        "lbs": ["lbs", "sgn"],
        "lce": ["lce", "ms"],
        "lcf": ["lcf", "ms"],
        "liw": ["liw", "ms"],
        "lls": ["lls", "sgn"],
        "lsg": ["lsg", "sgn"],
        "lsl": ["lsl", "sgn"],
        "lso": ["lso", "sgn"],
        "lsp": ["lsp", "sgn"],
        "lst": ["lst", "sgn"],
        "lsy": ["lsy", "sgn"],
        "ltg": ["ltg", "lv"],
        "lvs": ["lvs", "lv"],
        "lzh": ["lzh", "zh"],
        "max": ["max", "ms"],
        "mdl": ["mdl", "sgn"],
        "meo": ["meo", "ms"],
        "mfa": ["mfa", "ms"],
        "mfb": ["mfb", "ms"],
        "mfs": ["mfs", "sgn"],
        "min": ["min", "ms"],
        "mnp": ["mnp", "zh"],
        "mqg": ["mqg", "ms"],
        "mre": ["mre", "sgn"],
        "msd": ["msd", "sgn"],
        "msi": ["msi", "ms"],
        "msr": ["msr", "sgn"],
        "mui": ["mui", "ms"],
        "mzc": ["mzc", "sgn"],
        "mzg": ["mzg", "sgn"],
        "mzy": ["mzy", "sgn"],
        "nan": ["nan", "zh"],
        "nbs": ["nbs", "sgn"],
        "ncs": ["ncs", "sgn"],
        "nsi": ["nsi", "sgn"],
        "nsl": ["nsl", "sgn"],
        "nsp": ["nsp", "sgn"],
        "nsr": ["nsr", "sgn"],
        "nzs": ["nzs", "sgn"],
        "okl": ["okl", "sgn"],
        "orn": ["orn", "ms"],
        "ors": ["ors", "ms"],
        "pel": ["pel", "ms"],
        "pga": ["pga", "ar"],
        "pks": ["pks", "sgn"],
        "prl": ["prl", "sgn"],
        "prz": ["prz", "sgn"],
        "psc": ["psc", "sgn"],
        "psd": ["psd", "sgn"],
        "pse": ["pse", "ms"],
        "psg": ["psg", "sgn"],
        "psl": ["psl", "sgn"],
        "pso": ["pso", "sgn"],
        "psp": ["psp", "sgn"],
        "psr": ["psr", "sgn"],
        "pys": ["pys", "sgn"],
        "rms": ["rms", "sgn"],
        "rsi": ["rsi", "sgn"],
        "rsl": ["rsl", "sgn"],
        "sdl": ["sdl", "sgn"],
        "sfb": ["sfb", "sgn"],
        "sfs": ["sfs", "sgn"],
        "sgg": ["sgg", "sgn"],
        "sgx": ["sgx", "sgn"],
        "shu": ["shu", "ar"],
        "slf": ["slf", "sgn"],
        "sls": ["sls", "sgn"],
        "sqs": ["sqs", "sgn"],
        "ssh": ["ssh", "ar"],
        "ssp": ["ssp", "sgn"],
        "ssr": ["ssr", "sgn"],
        "svk": ["svk", "sgn"],
        "swc": ["swc", "sw"],
        "swh": ["swh", "sw"],
        "swl": ["swl", "sgn"],
        "syy": ["syy", "sgn"],
        "tmw": ["tmw", "ms"],
        "tse": ["tse", "sgn"],
        "tsm": ["tsm", "sgn"],
        "tsq": ["tsq", "sgn"],
        "tss": ["tss", "sgn"],
        "tsy": ["tsy", "sgn"],
        "tza": ["tza", "sgn"],
        "ugn": ["ugn", "sgn"],
        "ugy": ["ugy", "sgn"],
        "ukl": ["ukl", "sgn"],
        "uks": ["uks", "sgn"],
        "urk": ["urk", "ms"],
        "uzn": ["uzn", "uz"],
        "uzs": ["uzs", "uz"],
        "vgt": ["vgt", "sgn"],
        "vkk": ["vkk", "ms"],
        "vkt": ["vkt", "ms"],
        "vsi": ["vsi", "sgn"],
        "vsl": ["vsl", "sgn"],
        "vsv": ["vsv", "sgn"],
        "wuu": ["wuu", "zh"],
        "xki": ["xki", "sgn"],
        "xml": ["xml", "sgn"],
        "xmm": ["xmm", "ms"],
        "xms": ["xms", "sgn"],
        "yds": ["yds", "sgn"],
        "ysl": ["ysl", "sgn"],
        "yue": ["yue", "zh"],
        "zib": ["zib", "sgn"],
        "zlm": ["zlm", "ms"],
        "zmi": ["zmi", "ms"],
        "zsl": ["zsl", "sgn"],
        "zsm": ["zsm", "ms"]
    };


    /**
     * Canonicalizes the given well-formed BCP 47 language tag, including regularized case of subtags.
     *
     * Spec: ECMAScript Internationalization API Specification, draft, 6.2.3.
     * Spec: RFC 5646, section 4.5.
     */
    function canonicalizeLanguageTag(locale) {

        // start with lower case for easier processing, and because most subtags will need to be lower case anyway
        locale = locale.toLowerCase();

        // handle mappings for complete tags
        if (__tagMappings.hasOwnProperty(locale)) {
            return __tagMappings[locale];
        }

        var subtags = locale.split("-");
        var i = 0;

        // handle standard part: all subtags before first singleton or "x"
        while (i < subtags.length) {
            var subtag = subtags[i];
            if (subtag.length === 1 && (i > 0 || subtag === "x")) {
                break;
            } else if (i !== 0 && subtag.length === 2) {
                subtag = subtag.toUpperCase();
            } else if (subtag.length === 4) {
                subtag = subtag[0].toUpperCase() + subtag.substring(1).toLowerCase();
            }
            if (__subtagMappings.hasOwnProperty(subtag)) {
                subtag = __subtagMappings[subtag];
            } else if (__extlangMappings.hasOwnProperty(subtag)) {
                subtag = __extlangMappings[subtag][0];
                if (i === 1 && __extlangMappings[subtag][1] === subtags[0]) {
                    subtags.shift();
                    i--;
                }
            }
            subtags[i] = subtag;
            i++;
        }
        var normal = subtags.slice(0, i).join("-");

        // handle extensions
        var extensions = [];
        while (i < subtags.length && subtags[i] !== "x") {
            var extensionStart = i;
            i++;
            while (i < subtags.length && subtags[i].length > 1) {
                i++;
            }
            var extension = subtags.slice(extensionStart, i).join("-");
            extensions.push(extension);
        }
        extensions.sort();

        // handle private use
        var privateUse;
        if (i < subtags.length) {
            privateUse = subtags.slice(i).join("-");
        }

        // put everything back together
        var canonical = normal;
        if (extensions.length > 0) {
            canonical += "-" + extensions.join("-");
        }
        if (privateUse !== undefined) {
            if (canonical.length > 0) {
                canonical += "-" + privateUse;
            } else {
                canonical = privateUse;
            }
        }

        return canonical;
    }

    return typeof locale === "string" && isStructurallyValidLanguageTag(locale) &&
            canonicalizeLanguageTag(locale) === locale;
}


/**
 * Tests whether the named options property is correctly handled by the given constructor.
 * @param {object} Constructor the constructor to test.
 * @param {string} property the name of the options property to test.
 * @param {string} type the type that values of the property are expected to have
 * @param {Array} [values] an array of allowed values for the property. Not needed for boolean.
 * @param {any} fallback the fallback value that the property assumes if not provided.
 * @param {object} testOptions additional options:
 *     @param {boolean} isOptional whether support for this property is optional for implementations.
 *     @param {boolean} noReturn whether the resulting value of the property is not returned.
 *     @param {boolean} isILD whether the resulting value of the property is implementation and locale dependent.
 *     @param {object} extra additional option to pass along, properties are value -> {option: value}.
 * @return {boolean} whether the test succeeded.
 */
function testOption(Constructor, property, type, values, fallback, testOptions) {
    var isOptional = testOptions !== undefined && testOptions.isOptional === true;
    var noReturn = testOptions !== undefined && testOptions.noReturn === true;
    var isILD = testOptions !== undefined && testOptions.isILD === true;
    
    function addExtraOptions(options, value, testOptions) {
        if (testOptions !== undefined && testOptions.extra !== undefined) {
            var extra;
            if (value !== undefined && testOptions.extra[value] !== undefined) {
                extra = testOptions.extra[value];
            } else if (testOptions.extra.any !== undefined) {
                extra = testOptions.extra.any;
            }
            if (extra !== undefined) {
                Object.getOwnPropertyNames(extra).forEach(function (prop) {
                    options[prop] = extra[prop];
                });
            }
        }
    }

    var testValues, options, obj, expected, actual, error;

    // test that the specified values are accepted. Also add values that convert to specified values.
    if (type === "boolean") {
        if (values === undefined) {
            values = [true, false];
        }
        testValues = values.slice(0);
        testValues.push(888);
        testValues.push(0);
    } else if (type === "string") {
        testValues = values.slice(0);
        testValues.push({toString: function () { return values[0]; }});
    }
    testValues.forEach(function (value) {
        options = {};
        options[property] = value;
        addExtraOptions(options, value, testOptions);
        obj = new Constructor(undefined, options);
        if (noReturn) {
            if (obj.resolvedOptions().hasOwnProperty(property)) {
                $ERROR("Option property " + property + " is returned, but shouldn't be.");
            }
        } else {
            actual = obj.resolvedOptions()[property];
            if (isILD) {
                if (actual !== undefined && values.indexOf(actual) === -1) {
                    $ERROR("Invalid value " + actual + " returned for property " + property + ".");
                }
            } else {
                if (type === "boolean") {
                    expected = Boolean(value);
                } else if (type === "string") {
                    expected = String(value);
                }
                if (actual !== expected && !(isOptional && actual === undefined)) {
                    $ERROR("Option value " + value + " for property " + property +
                        " was not accepted; got " + actual + " instead.");
                }
            }
        }
    });

    // test that invalid values are rejected
    if (type === "string") {
        var invalidValues = ["invalidValue", -1, null];
        // assume that we won't have values in caseless scripts
        if (values[0].toUpperCase() !== values[0]) {
            invalidValues.push(values[0].toUpperCase());
        } else {
            invalidValues.push(values[0].toLowerCase());
        }
        invalidValues.forEach(function (value) {
            options = {};
            options[property] = value;
            addExtraOptions(options, value, testOptions);
            error = undefined;
            try {
                obj = new Constructor(undefined, options);
            } catch (e) {
                error = e;
            }
            if (error === undefined) {
                $ERROR("Invalid option value " + value + " for property " + property + " was not rejected.");
            } else if (error.name !== "RangeError") {
                $ERROR("Invalid option value " + value + " for property " + property + " was rejected with wrong error " + error.name + ".");
            }
        });
    }

    // test that fallback value or another valid value is used if no options value is provided
    if (!noReturn) {
        options = {};
        addExtraOptions(options, undefined, testOptions);
        obj = new Constructor(undefined, options);
        actual = obj.resolvedOptions()[property];
        if (!(isOptional && actual === undefined)) {
            if (fallback !== undefined) {
                if (actual !== fallback) {
                    $ERROR("Option fallback value " + fallback + " for property " + property +
                        " was not used; got " + actual + " instead.");
                }
            } else {
                if (values.indexOf(actual) === -1 && !(isILD && actual === undefined)) {
                    $ERROR("Invalid value " + actual + " returned for property " + property + ".");
                }
            }
        }
    }

    return true;
}


/**
 * Tests whether the named property of the given object has a valid value
 * and the default attributes of the properties of an object literal.
 * @param {Object} obj the object to be tested.
 * @param {string} property the name of the property
 * @param {Function|Array} valid either a function that tests value for validity and returns a boolean,
 *     an array of valid values.
 * @exception if the property has an invalid value.
 */
function testProperty(obj, property, valid) {
    var desc = Object.getOwnPropertyDescriptor(obj, property);
    if (!desc.writable) {
        $ERROR("Property " + property + " must be writable.");
    }
    if (!desc.enumerable) {
        $ERROR("Property " + property + " must be enumerable.");
    }
    if (!desc.configurable) {
        $ERROR("Property " + property + " must be configurable.");
    }
    var value = desc.value;
    var isValid = (typeof valid === "function") ? valid(value) : (valid.indexOf(value) !== -1);
    if (!isValid) {
        $ERROR("Property value " + value + " is not allowed for property " + property + ".");
    }
}


/**
 * Tests whether the named property of the given object, if present at all, has a valid value
 * and the default attributes of the properties of an object literal.
 * @param {Object} obj the object to be tested.
 * @param {string} property the name of the property
 * @param {Function|Array} valid either a function that tests value for validity and returns a boolean,
 *     an array of valid values.
 * @exception if the property is present and has an invalid value.
 */
function mayHaveProperty(obj, property, valid) {
    if (obj.hasOwnProperty(property)) {
        testProperty(obj, property, valid);
    }
}


/**
 * Tests whether the given object has the named property with a valid value
 * and the default attributes of the properties of an object literal.
 * @param {Object} obj the object to be tested.
 * @param {string} property the name of the property
 * @param {Function|Array} valid either a function that tests value for validity and returns a boolean,
 *     an array of valid values.
 * @exception if the property is missing or has an invalid value.
 */
function mustHaveProperty(obj, property, valid) {
    if (!obj.hasOwnProperty(property)) {
        $ERROR("Object is missing property " + property + ".");
    }
    testProperty(obj, property, valid);
}


/**
 * Tests whether the given object does not have the named property.
 * @param {Object} obj the object to be tested.
 * @param {string} property the name of the property
 * @exception if the property is present.
 */
function mustNotHaveProperty(obj, property) {
    if (obj.hasOwnProperty(property)) {
        $ERROR("Object has property it mustn't have: " + property + ".");
    }
}


/**
 * Properties of the RegExp constructor that may be affected by use of regular
 * expressions, and the default values of these properties. Properties are from
 * https://developer.mozilla.org/en-US/docs/JavaScript/Reference/Deprecated_and_obsolete_features#RegExp_Properties
 */
// CHANGE GW
/*
var regExpProperties = ["$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8", "$9",
    "$_", "$*", "$&", "$+", "$`", "$'",
    "input", "lastMatch", "lastParen", "leftContext", "rightContext"
];

var regExpPropertiesDefaultValues = (function () {
    var values = Object.create(null);
    regExpProperties.forEach(function (property) {
        values[property] = RegExp[property];
    });
    return values;
}());
*/

/**
 * Tests that executing the provided function (which may use regular expressions
 * in its implementation) does not create or modify unwanted properties on the
 * RegExp constructor.
 */
/*
function testForUnwantedRegExpChanges(testFunc) {
    regExpProperties.forEach(function (property) {
        RegExp[property] = regExpPropertiesDefaultValues[property];
    });
    testFunc();
    regExpProperties.forEach(function (property) {
        if (RegExp[property] !== regExpPropertiesDefaultValues[property]) {
            $ERROR("RegExp has unexpected property " + property + " with value " +
                RegExp[property] + ".");
        }
    });
}
*/

/**
 * Tests whether name is a valid BCP 47 numbering system name
 * and not excluded from use in the ECMAScript Internationalization API.
 * @param {string} name the name to be tested.
 * @return {boolean} whether name is a valid BCP 47 numbering system name and
 *     allowed for use in the ECMAScript Internationalization API.
 */

function isValidNumberingSystem(name) {
    
    // source: CLDR file common/bcp47/number.xml; version CLDR 21.
    var numberingSystems = [
        "arab",
        "arabext",
        "armn",
        "armnlow",
        "bali",
        "beng",
        "brah",
        "cakm",
        "cham",
        "deva",
        "ethi",
        "finance",
        "fullwide",
        "geor",
        "grek",
        "greklow",
        "gujr",
        "guru",
        "hanidec",
        "hans",
        "hansfin",
        "hant",
        "hantfin",
        "hebr",
        "java",
        "jpan",
        "jpanfin",
        "kali",
        "khmr",
        "knda",
        "osma",            
        "lana",
        "lanatham",
        "laoo",
        "latn",
        "lepc",
        "limb",
        "mlym",
        "mong",
        "mtei",
        "mymr",
        "mymrshan",
        "native",
        "nkoo",
        "olck",
        "orya",
        "roman",
        "romanlow",
        "saur",
        "shrd",
        "sora",
        "sund",
        "talu",
        "takr",
        "taml",
        "tamldec",
        "telu",
        "thai",
        "tibt",
        "traditio",
        "vaii"
    ];
    
    var excluded = [
        "finance",
        "native",
        "traditio"
    ];
        
    
    return numberingSystems.indexOf(name) !== -1 && excluded.indexOf(name) === -1;
}


/**
 * Provides the digits of numbering systems with simple digit mappings,
 * as specified in 11.3.2.
 */

var numberingSystemDigits = {
    arab: "٠١٢٣٤٥٦٧٨٩",
    arabext: "۰۱۲۳۴۵۶۷۸۹",
    beng: "০১২৩৪৫৬৭৮৯",
    deva: "०१२३४५६७८९",
    fullwide: "０１２３４５６７８９",
    gujr: "૦૧૨૩૪૫૬૭૮૯",
    guru: "੦੧੨੩੪੫੬੭੮੯",
    hanidec: "〇一二三四五六七八九",
    khmr: "០១២៣៤៥៦៧៨៩",
    knda: "೦೧೨೩೪೫೬೭೮೯",
    laoo: "໐໑໒໓໔໕໖໗໘໙",
    latn: "0123456789",
    mlym: "൦൧൨൩൪൫൬൭൮൯",
    mong: "᠐᠑᠒᠓᠔᠕᠖᠗᠘᠙",
    mymr: "၀၁၂၃၄၅၆၇၈၉",
    orya: "୦୧୨୩୪୫୬୭୮୯",
    tamldec: "௦௧௨௩௪௫௬௭௮௯",
    telu: "౦౧౨౩౪౫౬౭౮౯",
    thai: "๐๑๒๓๔๕๖๗๘๙",
    tibt: "༠༡༢༣༤༥༦༧༨༩"
};


/**
 * Tests that number formatting is handled correctly. The function checks that the
 * digit sequences in formatted output are as specified, converted to the
 * selected numbering system, and embedded in consistent localized patterns.
 * @param {Array} locales the locales to be tested.
 * @param {Array} numberingSystems the numbering systems to be tested.
 * @param {Object} options the options to pass to Intl.NumberFormat. Options
 *     must include {useGrouping: false}, and must cause 1.1 to be formatted
 *     pre- and post-decimal digits.
 * @param {Object} testData maps input data (in ES5 9.3.1 format) to expected output strings
 *     in unlocalized format with Western digits.
 */

function testNumberFormat(locales, numberingSystems, options, testData) {
    locales.forEach(function (locale) {
        numberingSystems.forEach(function (numbering) {
            var digits = numberingSystemDigits[numbering];
            var format = new Intl.NumberFormat([locale + "-u-nu-" + numbering], options);
    
            function getPatternParts(positive) {
                var n = positive ? 1.1 : -1.1;
                var formatted = format.format(n);
                var oneoneRE = "([^" + digits + "]*)[" + digits + "]+([^" + digits + "]+)[" + digits + "]+([^" + digits + "]*)";
                var match = formatted.match(new RegExp(oneoneRE));
                if (match === null) {
                    $ERROR("Unexpected formatted " + n + " for " +
                        format.resolvedOptions().locale + " and options " +
                        JSON.stringify(options) + ": " + formatted);
                }
                return match;
            }
            
            function toNumbering(raw) {
                return raw.replace(/[0-9]/g, function (digit) {
                    return digits[digit.charCodeAt(0) - "0".charCodeAt(0)];
                });
            }
            
            function buildExpected(raw, patternParts) {
                var period = raw.indexOf(".");
                if (period === -1) {
                    return patternParts[1] + toNumbering(raw) + patternParts[3];
                } else {
                    return patternParts[1] + 
                        toNumbering(raw.substring(0, period)) +
                        patternParts[2] +
                        toNumbering(raw.substring(period + 1)) +
                        patternParts[3];
                }
            }
            
            if (format.resolvedOptions().numberingSystem === numbering) {
                // figure out prefixes, infixes, suffixes for positive and negative values
                var posPatternParts = getPatternParts(true);
                var negPatternParts = getPatternParts(false);
                
                Object.getOwnPropertyNames(testData).forEach(function (input) {
                    var rawExpected = testData[input];
                    var patternParts;
                    if (rawExpected[0] === "-") {
                        patternParts = negPatternParts;
                        rawExpected = rawExpected.substring(1);
                    } else {
                        patternParts = posPatternParts;
                    }
                    var expected = buildExpected(rawExpected, patternParts);
                    var actual = format.format(input);
                    if (actual !== expected) {
                        $ERROR("Formatted value for " + input + ", " +
                        format.resolvedOptions().locale + " and options " +
                        JSON.stringify(options) + " is " + actual + "; expected " + expected + ".");
                    }
                });
            }
        });
    });
}


/**
 * Return the components of date-time formats.
 * @return {Array} an array with all date-time components.
 */

function getDateTimeComponents() {
    return ["weekday", "era", "year", "month", "day", "hour", "minute", "second", "timeZoneName"];
}


/**
 * Return the valid values for the given date-time component, as specified
 * by the table in section 12.1.1.
 * @param {string} component a date-time component.
 * @return {Array} an array with the valid values for the component.
 */

function getDateTimeComponentValues(component) {
    
    var components = {
        weekday: ["narrow", "short", "long"],
        era: ["narrow", "short", "long"],
        year: ["2-digit", "numeric"],
        month: ["2-digit", "numeric", "narrow", "short", "long"],
        day: ["2-digit", "numeric"],
        hour: ["2-digit", "numeric"],
        minute: ["2-digit", "numeric"],
        second: ["2-digit", "numeric"],
        timeZoneName: ["short", "long"]
    };
    
    var result = components[component];
    if (result === undefined) {
        $ERROR("Internal error: No values defined for date-time component " + component + ".");
    }
    return result;
}


/**
 * Tests that the given value is valid for the given date-time component.
 * @param {string} component a date-time component.
 * @param {string} value the value to be tested.
 * @return {boolean} true if the test succeeds.
 * @exception if the test fails.
 */

function testValidDateTimeComponentValue(component, value) {
    if (getDateTimeComponentValues(component).indexOf(value) === -1) {
        $ERROR("Invalid value " + value + " for date-time component " + component + ".");
    }
    return true;
}


/**
 * Verifies that the actual array matches the expected one in length, elements,
 * and element order.
 * @param {Array} expected the expected array.
 * @param {Array} actual the actual array.
 * @return {boolean} true if the test succeeds.
 * @exception if the test fails.
 */
function testArraysAreSame(expected, actual) {
    for (i = 0; i < Math.max(actual.length, expected.length); i++) {
        if (actual[i] !== expected[i]) {
            $ERROR("Result array element at index " + i + " should be \"" +
                expected[i] + "\" but is \"" + actual[i] + "\".");
        }
    }
    return true;
}


function testcase() {
        /*MultiLine
        Comments 
        \u2028 var = ;
        */
        return true;
    }
runTestCase(testcase);

// CHANGE GW
result = 1;

