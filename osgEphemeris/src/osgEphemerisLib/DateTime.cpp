/*
 -------------------------------------------------------------------------------
 | osgEphemeris - Copyright (C) 2007  Don Burns                                |
 |                                                                             |
 | This library is free software; you can redistribute it and/or modify        |
 | it under the terms of the GNU Lesser General Public License as published    |
 | by the Free Software Foundation; either version 3 of the License, or        |
 | (at your option) any later version.                                         |
 |                                                                             |
 | This library is distributed in the hope that it will be useful, but         |
 | WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  |
 | or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public     |
 | License for more details.                                                   |
 |                                                                             |
 | You should have received a copy of the GNU Lesser General Public License    |
 | along with this software; if not, write to the Free Software Foundation,    |
 | Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.               |
 |                                                                             |
 -------------------------------------------------------------------------------
 */

#include <osgEphemeris/DateTime.h>

using namespace osgEphemeris;

const char *DateTime::weekDayNames[7] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};

const char *DateTime::monthNames[12] = {
    "January",
    "Feburary",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

DateTime::DateTime(
            uint32_t year,
            uint32_t month,
            uint32_t day,
            uint32_t hour,
            uint32_t minute,
            uint32_t second  )
{
    _tm.tm_year = year - 1900;
    _tm.tm_mon  = month - 1;
    _tm.tm_mday = day;
    _tm.tm_hour = hour;
    _tm.tm_min  = minute;
    _tm.tm_sec  = second;
    _tm.tm_wday = -1;
    _tm.tm_yday = -1;
    _tm.tm_isdst= -1;

    mktime(&_tm);
}

DateTime::DateTime( const DateTime &dt ):
    _tm(dt._tm)
{
    mktime(&_tm);
}

DateTime::DateTime(bool initialize)
{
    if( initialize )
    {
        now();
    }
}

DateTime::DateTime( const struct tm &tm )
{
    _tm = tm;
    mktime( &_tm );
}

void DateTime::now()
{
    time_t utc = time(0L);
    _tm = *gmtime(&utc);
    _tm.tm_sec += _tzoff;
    mktime(&_tm);
}

void DateTime::setYear( uint32_t year  )
{
    _tm.tm_year = year - 1900;
    mktime(&_tm);
}

uint32_t DateTime::getYear() const
{
    return _tm.tm_year + 1900;
}

void DateTime::setMonth(uint32_t month)
{
    _tm.tm_mon = month - 1;
    mktime(&_tm);
}

uint32_t DateTime::getMonth() const
{
    return _tm.tm_mon + 1;
}

std::string DateTime::getMonthString() const
{
    return std::string( monthNames[_tm.tm_mon] );
}

std::string DateTime::getMonthString(uint32_t month) // Will pass in 1-12
{
    return std::string( monthNames[(month-1)%12] );
}

void DateTime::setDayOfMonth(uint32_t day)
{
    _tm.tm_mday = day;
    mktime(&_tm);
}

uint32_t DateTime::getDayOfMonth() const
{
    return _tm.tm_mday;
}

uint32_t DateTime::getDayOfYear() const
{
    return _tm.tm_yday;
}

uint32_t DateTime::getDayOfWeek() const
{
    return _tm.tm_wday;
}

std::string DateTime::getDayOfWeekString() const
{
    return std::string(weekDayNames[getDayOfWeek()]);
}

std::string DateTime::getDayOfWeekString(uint32_t wday)
{
    return std::string(weekDayNames[wday%7]);
}

void DateTime::setHour( uint32_t hour )
{
    _tm.tm_hour = hour;
    mktime(&_tm);
}

uint32_t DateTime::getHour() const
{
    return _tm.tm_hour;
}

void DateTime::setMinute( uint32_t minute )
{
    _tm.tm_min = minute;
    mktime(&_tm);
}

uint32_t DateTime::getMinute() const
{
    return _tm.tm_min;
}

void  DateTime::setSecond( uint32_t second )
{
    _tm.tm_sec = second;
    mktime(&_tm);
}

uint32_t DateTime::getSecond() const
{
    return _tm.tm_sec;
}

bool DateTime::isDaylightSavingsTime() const
{
    return (_tm.tm_isdst != 0);
}

void DateTime::setTimeZoneOffset( bool useSystemTimeZone, int32_t hours )
{
#ifndef WIN32
    if( useSystemTimeZone )
    {
        _tzoff = _tm.tm_gmtoff;
        return;
    }
#endif

    _tzoff = hours * 3600;
}

int32_t DateTime::getTimeZoneOffset() const
{
    return _tzoff/3600;
}

double DateTime::getModifiedJulianDate() const
{
    struct tm lmt = _tm;
    lmt.tm_sec -= _tzoff;
    mktime(&lmt);

    double day   =  (double(lmt.tm_mday)) +           // Day
                    (double(lmt.tm_hour)/24.0) +      // hour
                    (double(lmt.tm_min)/(24.0*60)) +   // minutes
                    (double(lmt.tm_sec)/(24*3600.0));  // seconds

    int month = lmt.tm_mon + 1;
    int m = month;
    int year = lmt.tm_year + 1900;
    int y = (year < 0) ? year + 1 : year;
    if( month < 3)
    {
        m += 12;
        y -= 1;
    }

    int b = 0;
    if (year < 1582 || (year == 1582 && (month < 10 || (month == 10 && day < 15))))
    {
        b = 0;
    }
    else
    {
        int a;
        a = y/100;
        b = 2 - a + a/4;
    }

    int c = 0;
    if (y < 0)
    {
        c = (long)((365.25*y) - 0.75) - 694025L;
    }
    else
    {
        c = (long)(365.25*y) - 694025L;
    }

    int d = (int)(30.6001*(m+1));

    double mjd = b + c + d + day - 0.5;

    return mjd;
}

DateTime DateTime::getGMT() const
{
    time_t utc = time(0L);
    return DateTime(*gmtime(&utc));
}


