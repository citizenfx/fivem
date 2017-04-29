/*
* Calendar Functions
* (C) 1999-2009,2015 Jack Lloyd
* (C) 2015 Simon Warta (Kullo GmbH)
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#ifndef BOTAN_CALENDAR_H__
#define BOTAN_CALENDAR_H__

#include <botan/types.h>
#include <chrono>
#include <string>

namespace Botan {

/**
* Struct representing a particular date and time
*/
struct BOTAN_DLL calendar_point
   {
   /** The year */
   uint32_t year;

   /** The month, 1 through 12 for Jan to Dec */
   uint32_t month;

   /** The day of the month, 1 through 31 (or 28 or 30 based on month */
   uint32_t day;

   /** Hour in 24-hour form, 0 to 23 */
   uint32_t hour;

   /** Minutes in the hour, 0 to 60 */
   uint32_t minutes;

   /** Seconds in the minute, 0 to 60, but might be slightly
       larger to deal with leap seconds on some systems
   */
   uint32_t seconds;

   /**
   * Initialize a calendar_point
   * @param y the year
   * @param mon the month
   * @param d the day
   * @param h the hour
   * @param min the minute
   * @param sec the second
   */
   calendar_point(uint32_t y, uint32_t mon, uint32_t d, uint32_t h, uint32_t min, uint32_t sec) :
      year(y), month(mon), day(d), hour(h), minutes(min), seconds(sec) {}

   /**
   * Returns an STL timepoint object
   */
   std::chrono::system_clock::time_point to_std_timepoint() const;

   /**
   * Returns a human readable string of the struct's components.
   * Formatting might change over time. Currently it is RFC339 'iso-date-time'.
   */
   std::string to_string() const;
   };

/**
* Convert a time_point to a calendar_point
* @param time_point a time point from the system clock
* @return calendar_point object representing this time point
*/
BOTAN_DLL calendar_point calendar_value(
   const std::chrono::system_clock::time_point& time_point);

}

#endif
