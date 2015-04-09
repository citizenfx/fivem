/*
* Calendar Functions
* (C) 1999-2009 Jack Lloyd
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CALENDAR_H__
#define BOTAN_CALENDAR_H__

#include <botan/types.h>
#include <chrono>

namespace Botan {

/**
* Struct representing a particular date and time
*/
struct BOTAN_DLL calendar_point
   {
   /** The year */
   u32bit year;

   /** The month, 1 through 12 for Jan to Dec */
   byte month;

   /** The day of the month, 1 through 31 (or 28 or 30 based on month */
   byte day;

   /** Hour in 24-hour form, 0 to 23 */
   byte hour;

   /** Minutes in the hour, 0 to 60 */
   byte minutes;

   /** Seconds in the minute, 0 to 60, but might be slightly
       larger to deal with leap seconds on some systems
   */
   byte seconds;

   /**
   * Initialize a calendar_point
   * @param y the year
   * @param mon the month
   * @param d the day
   * @param h the hour
   * @param min the minute
   * @param sec the second
   */
   calendar_point(u32bit y, byte mon, byte d, byte h, byte min, byte sec) :
      year(y), month(mon), day(d), hour(h), minutes(min), seconds(sec) {}
   };

/*
* @param time_point a time point from the system clock
* @return calendar_point object representing this time point
*/
BOTAN_DLL calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point);

}

#endif
