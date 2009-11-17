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

#include <time.h>
#include <unistd.h>
#include <osgEphemeris/EphemerisData.h>

int main( int argc, char **argv )
{
    time_t seconds = 0;

    // Attached to the default shared memory segment
    osgEphemeris::EphemerisData *data = new(osgEphemeris::EphemerisData::getDefaultShmemFileName())osgEphemeris::EphemerisData;

    for( ;; )
    {
         seconds += 60;
         struct tm *_tm = localtime(&seconds);

         data->dateTime.setYear( _tm->tm_year + 1900 );
         data->dateTime.setMonth( _tm->tm_mon + 1 );
         data->dateTime.setDayOfMonth( _tm->tm_mday + 1 );
         data->dateTime.setHour( _tm->tm_hour );
         data->dateTime.setMinute( _tm->tm_min );
         data->dateTime.setSecond( _tm->tm_sec );

         // Sleep 16 milliseconds
         usleep(16667);
    }

    return 0;
}

