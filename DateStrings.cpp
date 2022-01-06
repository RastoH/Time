/* DateStrings.cpp
 * Definitions for date strings for use with the Time library
 *
 * Updated for Arduino 1.5.7 18 July 2014
 *
 * No memory is consumed in the sketch if your code does not call any of the string methods
 * You can change the text of the strings, make sure the short strings are each exactly 3 characters 
 * the long strings can be any length up to the constant dt_MAX_STRING_LEN defined in TimeLib.h
 * 
 */

#include <Arduino.h>

// Arduino.h should properly define PROGMEM, PGM_P, strcpy_P, pgm_read_byte, pgm_read_ptr
// But not all platforms define these as they should.  If you find a platform needing these
// defined, or if any this becomes unnecessary as platforms improve, please send a pull req.
//#if defined(ESP8266)
//#undef PROGMEM
//#define PROGMEM
//#endif

#include <pgmspace.h> 
#include "sntpTime.h"

 
// the short strings for each day or month must be exactly dt_SHORT_STR_LEN
#define kShortNameLength  3 // the length of short strings
#define kBufferSize  10
#define kNumMonthNames  13
#define kNumDayOfWeekNames  8

//  static char mBuffer[kBufferSize];  // must be big enough for longest string and the terminating null

static const char kError[] PROGMEM = "Error";
static const char kJanuary[] PROGMEM = "January";
static const char kFebruary[] PROGMEM = "February";
static const char kMarch[] PROGMEM = "March";
static const char kApril[] PROGMEM = "April";
static const char kMay[] PROGMEM = "May";
static const char kJune[] PROGMEM = "June";
static const char kJuly[] PROGMEM = "July";
static const char kAugust[] PROGMEM = "August";
static const char kSeptember[] PROGMEM = "September";
static const char kOctober[] PROGMEM = "October";
static const char kNovember[] PROGMEM = "November";
static const char kDecember[] PROGMEM = "December";

// Place pointers into PROGMEM as well, saving 26 bytes of RAM.
// Use getStringAt() to access.
const char* const MonthLongNames_P[] PROGMEM = {
  kError, kJanuary, kFebruary, kMarch, kApril, kMay, kJune,
  kJuly, kAugust, kSeptember, kOctober, kNovember, kDecember
};

static const char kErr[] PROGMEM = "Err";
static const char kJan[] PROGMEM = "Jan";
static const char kFeb[] PROGMEM = "Feb";
static const char kMar[] PROGMEM = "Mar";
static const char kApr[] PROGMEM = "Apr";
static const char kJun[] PROGMEM = "Jun";
static const char kJul[] PROGMEM = "Jul";
static const char kAug[] PROGMEM = "Aug";
static const char kSep[] PROGMEM = "Sep";
static const char kOct[] PROGMEM = "Oct";
static const char kNov[] PROGMEM = "Nov";
static const char kDec[] PROGMEM = "Dec";

// Place pointers into PROGMEM as well, saving 26 bytes of RAM.
const char* const MonthShortNames_P[] PROGMEM = {
  kErr, kJan, kFeb, kMar, kApr, kMay, kJun,
  kJul, kAug, kSep, kOct, kNov, kDec
};

static const char kMonday[] PROGMEM = "Monday";
static const char kTuesday[] PROGMEM = "Tuesday";
static const char kWednesday[] PROGMEM = "Wednesday";
static const char kThursday[] PROGMEM = "Thursday";
static const char kFriday[] PROGMEM = "Friday";
static const char kSaturday[] PROGMEM = "Saturday";
static const char kSunday[] PROGMEM = "Sunday";

// Place pointers into PROGMEM as well, saving 16 bytes of RAM.
// ISO8601 says Monday=1, Sunday=7.
const char* const dayNames_P[] PROGMEM = {
  kError, kMonday, kTuesday, kWednesday, kThursday, kFriday, kSaturday, kSunday
};

static const char kMon[] PROGMEM = "Mon";
static const char kTue[] PROGMEM = "Tue";
static const char kWed[] PROGMEM = "Wed";
static const char kThu[] PROGMEM = "Thu";
static const char kFri[] PROGMEM = "Fri";
static const char kSat[] PROGMEM = "Sat";
static const char kSun[] PROGMEM = "Sun";

// Place pointers into PROGMEM as well, saving 16 bytes of RAM.
// ISO8601 says Monday=1, Sunday=7.
const char* const dayShortNames_P[] PROGMEM = {
  kErr, kMon, kTue, kWed, kThu, kFri, kSat, kSun
};

    /** Return the long month name. 0=Error, 1=January, 12=December. */ 
const char* monthStr(uint8_t month) {
  uint8_t index = (month < kNumMonthNames) ? month : 0;
  return MonthLongNames_P[index];
}

    /** Return the short month name. 0=Err, 1=Jan, 12=Dec. */ 
const char* monthShortStr(uint8_t month) {
  uint8_t index = (month < kNumMonthNames) ? month : 0;
  return MonthShortNames_P[index]; 
}

    /** Return the short dayOfWeek name. 0=Error, 1=Monday, 7=Sunday. */ 
const char * dayStr(uint8_t day) {
  uint8_t index = (day < kNumDayOfWeekNames) ? day : 0;
  return dayNames_P[index];
}

    /** Return the short dayOfWeek name. 0=Err, 1=Mon, 7=Sun. */ 
const char* dayShortStr(uint8_t day) {
  uint8_t index = (day < kNumDayOfWeekNames) ? day : 0;
  return dayShortNames_P[index];
}
