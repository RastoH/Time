/*
  time.c - low level time and date functions
  Copyright (c) Michael Margolis 2009-2014

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  1.0  6  Jan 2010 - initial release
  1.1  12 Feb 2010 - fixed leap year calculation error
  1.2  1  Nov 2010 - fixed setTime bug (thanks to Korman for this)
  1.3  24 Mar 2012 - many edits by Paul Stoffregen: fixed timeStatus() to update
                     status, updated examples for Arduino 1.0, fixed ARM
                     compatibility issues, added TimeArduinoDue and TimeTeensy3
                     examples, add error checking and messages to RTC examples,
                     add examples to DS1307RTC library.
  1.4  5  Sep 2014 - compatibility with Arduino 1.5.7
*/

#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif

#include "sntpTime.h"
#include <lwip/ip_addr.h> 
#include "IPAddress.h" 
#include "sntp.h"
#include "coredecls.h"

static tmElements_t tm;          // a cache of time elements
static time_t cacheTime;   // the time the cache was updated
static uint32_t syncInterval = 300;  // time sync will be attempted after this many seconds

void refreshCache(time_t t) {
  if (t == cacheTime) return;
  breakTime(t, tm); 
  cacheTime = t; 
}

int hour() { // the hour now 
  return hour(now()); 
}

int hour(time_t t) { // the hour for the given time
  refreshCache(t);
  return tm.Hour;  
}

int hourFormat12() { // the hour now in 12 hour format
  return hourFormat12(now()); 
}

int hourFormat12(time_t t) { // the hour for the given time in 12 hour format
  refreshCache(t);
  if( tm.Hour == 0 )
    return 12; // 12 midnight
  else if( tm.Hour  > 12)
    return tm.Hour - 12 ;
  else
    return tm.Hour ;
}

uint8_t isAM() { // returns true if time now is AM
  return !isPM(now()); 
}

uint8_t isAM(time_t t) { // returns true if given time is AM
  return !isPM(t);  
}

uint8_t isPM() { // returns true if PM
  return isPM(now()); 
}

uint8_t isPM(time_t t) { // returns true if PM
  return (hour(t) >= 12); 
}

int minute() {
  return minute(now()); 
}

int minute(time_t t) { // the minute for the given time
  refreshCache(t);
  return tm.Minute;  
}

int second() {
  return second(now()); 
}

int second(time_t t) {  // the second for the given time
  refreshCache(t);
  return tm.Second;
}

int day(){
  return(day(now())); 
}

int day(time_t t) { // the day for the given time (0-6)
  refreshCache(t);
  return tm.Day;
}

int weekday() {   // Sunday is day 1
  return  weekday(now()); 
}

int weekday(time_t t) {
  refreshCache(t);
  return tm.Wday;
}
   
int month(){
  return month(now()); 
}

int month(time_t t) {  // the month for the given time
  refreshCache(t);
  return tm.Month;
}

int year() {  // as in Processing, the full four digit year: (2009, 2010 etc) 
  return year(now()); 
}

int year(time_t t) { // the year for the given time
  refreshCache(t);
  return tmYearToCalendar(tm.Year);
}

/*============================================================================*/	
/* functions to convert to and from system time */
/* These are for interfacing with time services and are not normally needed in a sketch */

// leap year calculator expects year argument as years offset from 1970
#define LEAP_YEAR(Y)     ( ((1970+(Y))>0) && !((1970+(Y))%4) && ( ((1970+(Y))%100) || !((1970+(Y))%400) ) )

static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0
 
void breakTime(time_t timeInput, tmElements_t &tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  uint32_t time;
  unsigned long days;

  time = (uint32_t)timeInput;
  tm.Second = time % 60;
  time /= 60; // now it is minutes
  tm.Minute = time % 60;
  time /= 60; // now it is hours
  tm.Hour = time % 24;
  time /= 24; // now it is days
  //  tm.days = time;
  tm.Wday = ((time + 4) % 7) + 1;  // Sunday is day 1  // day of week 
  
  year = 0;  
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm.Year = year; // year is offset from 1970 
  
  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0
  //  tm.day_of_year = time;  

  //  days=0;
  //  month=0;
  //  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }
    
    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm.Month = month + 1;  // jan is month 1  
  tm.Day = time + 1;     // day of month
}

time_t makeTime(const tmElements_t &tm){   
// assemble time elements into time_t 
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  
  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm.Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm.Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds += SECS_PER_DAY;   // add extra days for leap years
    }
  }
  
  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++) {
    if ( (2 == i) && LEAP_YEAR(tm.Year)) { 
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm.Day-1) * SECS_PER_DAY;
  seconds+= tm.Hour * SECS_PER_HOUR;
  seconds+= tm.Minute * SECS_PER_MIN;
  seconds+= tm.Second;
  return (time_t)seconds; 
}
/*=====================================================*/	
/* Low level system time functions  */

//  static uint32_t sysTime = 0;
//  static uint32_t prevMillis = 0;
static time_t nextSyncTime = 0;
static timeStatus_t Status = timeNotSet;

getExternalTime getTimePtr = NULL;  // pointer to external sync function
setExternalTime setTimePtr = NULL;  // not used in this version

//  static SetExternalTime SetTimePtr; 

#define RTC_UTC_TEST 1640859659 // = Thu Dec 30 10:20:59 2021

time_t now() {
  time_t current_stamp;
  current_stamp = (time_t)sntp_get_current_timestamp();

  if (nextSyncTime <= current_stamp) {
    if (getTimePtr != 0) {
      time_t t = getTimePtr();
      if (t > RTC_UTC_TEST) {
        setTime(t);
        current_stamp = t;
      } else {
        nextSyncTime = current_stamp + syncInterval;
        Status = (Status == timeNotSet) ?  timeNotSet : timeNeedsSync;
      }
    }
  }  
  return (time_t)current_stamp;
}

time_t Now() {
  return sntp_get_current_timestamp();
}

void setTime(time_t ts) { 
  timeval tv { ts, 0 };
  //  timezone tz { 0, 0 };
  settimeofday(&tv, nullptr);  //   settimeofday(&tv, &tz); 

  Status = timeSet;
} 

void setTime(int hr,int min,int sec,int dy, int mnth, int yr){
 // year can be given as full four digit year or two digts (2010 or 10 for 2010);  
 //it is converted to years since 1970
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;  
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  setTime(makeTime(tm));
}

// indicates if time has been set and recently synchronized
timeStatus_t timeStatus() {
  //  now(); // required to actually update the status
  return Status;
}

void new_timeStatus(timeStatus_t new_status) {
  Status = new_status;
}

void setSyncProvider(getExternalTime getTimeFunction){
  getTimePtr = getTimeFunction;  
  nextSyncTime = Now();
  now(); // this will sync the clock
}

void setSyncInterval(time_t interval){ // set the number of seconds between re-sync
  syncInterval = (uint32_t)interval;
  nextSyncTime = Now() + syncInterval;
}

void setSyncReceiver(setExternalTime setTimeFunction) {  //  for RTC sync?  RTC.set(time_t) 
  setTimePtr = setTimeFunction;  
}

void IRAM_ATTR sntpSetTimeOfDayCallback(bool from_sntp /* <= this parameter is optional */) {  //  should be in iram (#7713)
 // time_t current_stamp;
 // current_stamp = sntp_get_current_timestamp();
  if (from_sntp) {
    Status = timeSet;
    if (setTimePtr) {
        setTimePtr(); 
    }
  }
}

//void sntpSetup(const char *server1, const char *server2, const char *server3) {
void sntpSetup(const char* server1, const char* server2, const char* server3) {
  if (sntp_enabled())
    sntp_stop(); 

  // will be called every time after ntp syncs AND loop() finishes
  settimeofday_cb(sntpSetTimeOfDayCallback);  //  sntp_set_time_sync_notification_cb()

  sntp_set_timezone(0);

  //  sntp_servermode_dhcp(0);

  sntp_setservername(0, server1); 
  sntp_setservername(1, server2);
  sntp_setservername(2, server3); 

  sntp_init();
}

//void sntpSetup(const char *server1) {
//  sntpSetup(server1, nullptr, nullptr);
//}

void sntpStart() {
  sntp_init();
}

void sntpStop() {
  sntp_stop(); 
}

bool sntpEnabled() {
  return (1 == sntp_enabled());
}

/**
 * Initialize one of the NTP servers by name
 *
 * @param numdns the index of the NTP server to set must be < SNTP_MAX_SERVERS
 * @param dnsserver DNS name of the NTP server to set, to be resolved at contact time
 */
//void ICACHE_FLASH_ATTR setservername(u8_t idx, const char *server) {
void sntpServerName(u8_t idx, const char *server){
  if (idx < SNTP_MAX_SERVERS) {
    sntp_setservername(idx, server);
  }
}

/**
 * Obtain one of the currently configured by name NTP servers.
 *
 * @param numdns the index of the NTP server
 * @return IP address of the indexed NTP server or nullptr if the NTP
 *         server has not been configured by name (or at all)
 */
 //const char * ICACHE_FLASH_ATTR getservername(uint8_t idx) {
const char * sntpServerName(uint8_t idx) {
  if (idx < SNTP_MAX_SERVERS) {
    return sntp_getservername(idx);
  }
  return nullptr;
} 

/**
 * Initialize one of the NTP servers by IP address
 *
 * @param numdns the index of the NTP server to set must be < SNTP_MAX_SERVERS
 * @param dnsserver IP address of the NTP server to set
 */
//void ICACHE_FLASH_ATTR

void sntpServer(uint8_t idx, IPAddress server) {
  if (idx < SNTP_MAX_SERVERS) {
    sntp_setserver(idx, server);
  }
} 

/**
 * Obtain one of the currently configured by IP address (or DHCP) NTP servers 
 *
 * @param numdns the index of the NTP server
 * @return IP address of the indexed NTP server or "ip_addr_any" if the NTP
 *         server has not been configured by address (or at all).
 */
//ip_addr_t ICACHE_FLASH_ATTR

IPAddress sntpServer(uint8_t idx) {
  if (idx < SNTP_MAX_SERVERS) {
    return sntp_getserver(idx);
  }
  return IP_ADDR_ANY;
} 


String sntpGetServer(uint8_t idx) {
  String server;

  if (idx < SNTP_MAX_SERVERS) {
    server = sntp_getservername(idx);
    if (!server.length()) {
      auto ip = IPAddress(sntp_getserver(idx));
      if (ip) {
        server = ip.toString();
      }
    } 
    if (!server.length()) {
      server = F("sntp server #");
      server += idx;
      server += F(" not set");
    }
  }
  return server;
}

bool sntpSetTimeZone(int8_t tz) {
  return sntp_set_timezone(tz);
}

int8_t sntpGetTimeZone() {
  return sntp_get_timezone();
}
