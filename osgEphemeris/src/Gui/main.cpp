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

#include <stdio.h>
#include <time.h>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Value_Output.H>
#include <FL/Fl_Value_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Roller.H>


#include <osgEphemeris/EphemerisData.h>

osgEphemeris::EphemerisData *ephemData = 0L;

static Fl_Value_Input *latInput = 0L;
static Fl_Value_Input *longInput = 0L;
static Fl_Value_Input *turbInput = 0L;
static Fl_Choice *tzchoice = 0L;
static Fl_Choice *month_choice = 0L;
static Fl_Value_Input *monthDayInput = 0L;
static Fl_Value_Input *yearInput = 0L;
static Fl_Roller *droller = 0L;
static Fl_Value_Input *hourInput = 0L;
static Fl_Value_Input *minInput = 0L;
static Fl_Value_Input *secInput = 0L;
static Fl_Roller *troller = 0L;

enum Which {
    E_Latitude,
    E_Longitude,
    E_Turbidity,
    E_TZOffset,
    E_DateByDoy,
    E_MonthDay,
    E_Year,
    E_Hour,
    E_Minute,
    E_Second,
};

void sync()
{
    if( ephemData == 0L )
        return;

    latInput->value(ephemData->latitude);

    longInput->value(ephemData->longitude);

    turbInput->value(ephemData->turbidity);
    tzchoice->value( ephemData->dateTime.getTimeZoneOffset() + 12 );

    month_choice->value( ephemData->dateTime.getMonth() - 1 );
    monthDayInput->value( ephemData->dateTime.getDayOfMonth() );
    yearInput->value( ephemData->dateTime.getYear() );
    droller->value( ephemData->dateTime.getDayOfYear() );

    int hour   = ephemData->dateTime.getHour();
    int minute = ephemData->dateTime.getMinute();
    int second = ephemData->dateTime.getSecond();

    hourInput->value( hour );
    minInput->value( minute );
    secInput->value( second );

    troller->value( hour * 3600 + minute * 60 + second );
}

void deactivateDateTimeInput()
{
    month_choice->deactivate();
    monthDayInput->deactivate();
    yearInput->deactivate();
    droller->deactivate();
    hourInput->deactivate();
    minInput->deactivate();
    secInput->deactivate();
    troller->deactivate();
}

void activateDateTimeInput()
{
    month_choice->activate();
    monthDayInput->activate();
    yearInput->activate();
    droller->activate();
    hourInput->activate();
    minInput->activate();
    secInput->activate();
    troller->activate();
}



static  int mdays[] = {
        31, // jan
        28, // feb
        31, // mar
        30, // apr
        31, // may
        30, // jun
        31, // jul
        31, // aug
        30, // sep
        31, // oct
        30, // nov
        31, // dec
};

static  int ly_mdays[] = {
        31, // jan
        29, // feb
        31, // mar
        30, // apr
        31, // may
        30, // jun
        31, // jul
        31, // aug
        30, // sep
        31, // oct
        30, // nov
        31, // dec
};


unsigned int dateToDayOfYear( unsigned int month, unsigned int day )
{
    if( ephemData == 0L )
        return 0;

    unsigned int n = 0;
    int *md = ephemData->dateTime.getYear() % 4 ? mdays : ly_mdays;

    for( unsigned int i = 0; i < (month-1); i++ )
    {
        n += md[i];
    }
    n += day;

    return n;
}

void dayOfYearToDate( unsigned int dayOfYear, unsigned int &month, unsigned int &day )
{
    if( ephemData == 0L )
        return;

    unsigned int m = 0;
    unsigned int n = 0;
    int *md = ephemData->dateTime.getYear() % 4 ? mdays : ly_mdays;
    while( (n + md[m]) < dayOfYear )
    {
        n += md[m++];
    }

    month = m + 1;
    day = dayOfYear - n;
}

void updateDateByDoy( double doy )
{
    unsigned int mm = (unsigned int)(doy);

    unsigned int month, day;
    dayOfYearToDate( mm, month, day );

    unsigned int year = (unsigned int)(yearInput->value());


    if( ephemData != 0L )
    {
        ephemData->dateTime.setYear(year) ;
        ephemData->dateTime.setMonth( month );
        ephemData->dateTime.setDayOfMonth( day );
        sync();
    }
}

void updateTime( double t )
{
    unsigned int hr = (unsigned int)(t/3600);
    /*
    t -= double(hr * 3600);
    unsigned int min = (unsigned int)(t)/60;
    unsigned int sec = (unsigned int)(t)%60;
    */
    unsigned int min = ((unsigned int)(t)%3600)/60;
    unsigned int sec = (unsigned int)(t)%60;

    if( ephemData != 0L )
    {
        ephemData->dateTime.setHour (hr);
        ephemData->dateTime.setMinute( min );
        ephemData->dateTime.setSecond( sec );
        sync();
    }
}

void s_updateTimeBySec( Fl_Widget *w, void * )
{
    Fl_Roller *roller = dynamic_cast<Fl_Roller *>(w);
    if( roller != 0L )
        updateTime( roller->value() );
}


static void timerUpdate( void * data )
{
    time_t utc = time(0L);
    struct tm lt  = *localtime( &utc);

    unsigned int doy = dateToDayOfYear( lt.tm_mon+1, lt.tm_mday );

    yearInput->value( lt.tm_year + 1900 );
    updateDateByDoy( double(doy) );

    double t = lt.tm_hour * 3600.0 +
               lt.tm_min  * 60.0 + 
               lt.tm_sec;
    updateTime( t );

    Fl::repeat_timeout( 0.25, timerUpdate );
}

static void s_toggleCallback( Fl_Widget *w, void *data )
{
    Fl_Button *toggle = dynamic_cast<Fl_Button *>(w);

    if( toggle->value() )
    {
        deactivateDateTimeInput();
        Fl::add_timeout( 0.25, timerUpdate );
    }
    else
    {
        activateDateTimeInput();
        Fl::remove_timeout( timerUpdate );
    }
}

static void s_updateDateByDoy( Fl_Widget *w, void *data )
{
    Fl_Roller *roller = dynamic_cast<Fl_Roller *>(w);
    if( roller != 0L )
        updateDateByDoy( roller->value() );
}

static void s_setMonthCallback( Fl_Widget *w, void *data )
{
    Fl_Choice *choice = dynamic_cast<Fl_Choice *>(w);
    if( choice != 0L )
    {
        if( ephemData != 0L )
        {
            ephemData->dateTime.setMonth( choice->value() + 1 );
            sync();
        }
    }
}

static void s_setTimeZoneCallback( Fl_Widget *w, void *data )
{
    Fl_Choice *choice = dynamic_cast<Fl_Choice *>(w);
    if( choice != 0L )
    {
        if( ephemData != 0L )
        {
            ephemData->dateTime.setTimeZoneOffset( false, choice->value() - 12 );
            sync();
        }
    }
}

static void s_valueInputCallback( Fl_Widget *w, void *data )
{
    Fl_Value_Input *s = dynamic_cast<Fl_Value_Input *>(w);
    Which which = static_cast<Which>( (unsigned long)(data) );

    if( s != 0L )
    {
        if( ephemData != 0L )
        {
            switch( which )
            {
                case E_Latitude:
                    if( ephemData != 0L )
                        ephemData->latitude = s->value();
                    break;

                case E_Longitude:
                    if( ephemData != 0L )
                        ephemData->longitude = s->value();
                    break;

                case E_Turbidity:
                    if( ephemData != 0L )
                        ephemData->turbidity = s->value();
                    break;

                case E_TZOffset:
                    if( ephemData != 0L )
                        ephemData->dateTime.setTimeZoneOffset( false, (int32_t)(s->value()) );
                    break;

                case E_MonthDay:
                    if( ephemData != 0L )
                        ephemData->dateTime.setDayOfMonth( (int)(s->value()) );
                    break;

                case E_Year:
                    if( ephemData != 0L )
                        ephemData->dateTime.setYear( (int)(s->value()) );
                    break;

                case E_Hour:
                    if( ephemData != 0L )
                        ephemData->dateTime.setHour( (int)(s->value()) );
                    break;

                case E_Minute:
                    if( ephemData != 0L )
                        ephemData->dateTime.setMinute( (int)(s->value()) );
                    break;

                case E_Second:
                    if( ephemData != 0L )
                        ephemData->dateTime.setSecond( (int)(s->value()) );
                    break;

                default:
                    break;
            }

            sync();
        }
    }
}

int main( int argc, char **argv )
{
    ephemData = new (osgEphemeris::EphemerisData::getDefaultShmemFileName()) osgEphemeris::EphemerisData;

    Fl_Window *window = new Fl_Window(  200, 250, "Lat/Lon, Date and Time Control" );

    window->begin();

    latInput = new Fl_Value_Input( 80, 10, 75, 20, "Latitude" );
    latInput->box( FL_EMBOSSED_BOX );
    latInput->callback( s_valueInputCallback, (void *)E_Latitude );
    latInput->minimum( -90.0 );
    latInput->maximum(  90.0 );
    latInput->step( 0.1 );
    latInput->value(ephemData->latitude);

    longInput = new Fl_Value_Input( 80, 35, 75, 20, "Longitude" );
    longInput->box( FL_EMBOSSED_BOX );
    longInput->callback( s_valueInputCallback, (void *)E_Longitude );
    longInput->minimum( -180.0 );
    longInput->maximum(  180.0 );
    longInput->step( 0.1 );
    longInput->value(ephemData->longitude);

    turbInput = new Fl_Value_Input( 80, 65, 75, 20, "Turbidity" );
    turbInput->box( FL_EMBOSSED_BOX );
    turbInput->callback( s_valueInputCallback, (void *)E_Turbidity );
    turbInput->minimum( 1.0 );
    turbInput->maximum( 60.0 );
    turbInput->step( 0.1 );
    turbInput->value(ephemData->turbidity);

    static Fl_Menu_Item tzchoices[] = {
      {"-12",  0, 0L, 0L},
      {"-11",  0, 0L, 0L},
      {"-10",  0, 0L, 0L},
      {"-9",  0, 0L, 0L},
      {"-8",  0, 0L, 0L},
      {"-7",  0, 0L, 0L},
      {"-6",  0, 0L, 0L},
      {"-5",  0, 0L, 0L},
      {"-4",  0, 0L, 0L},
      {"-3",  0, 0L, 0L},
      {"-2",  0, 0L, 0L},
      {"-1",  0, 0L, 0L},
      {"0",  0, 0L, 0L},
      {"1",  0, 0L, 0L},
      {"2",  0, 0L, 0L},
      {"3",  0, 0L, 0L},
      {"4",  0, 0L, 0L},
      {"5",  0, 0L, 0L},
      {"6",  0, 0L, 0L},
      {"7",  0, 0L, 0L},
      {"8",  0, 0L, 0L},
      {"9",  0, 0L, 0L},
      {"10",  0, 0L, 0L},
      {"11",  0, 0L, 0L},
      {"12",  0, 0L, 0L},
    };

    tzchoice = new Fl_Choice( 80, 65, 75, 20, "TimeZone" );
    tzchoice->menu( tzchoices );
    tzchoice->value( ephemData->dateTime.getTimeZoneOffset() + 12 );
    tzchoice->callback( s_setTimeZoneCallback, 0L );

    turbInput = new Fl_Value_Input( 80, 95, 75, 20, "Turbidity" );
    turbInput->box( FL_EMBOSSED_BOX );
    turbInput->callback( s_valueInputCallback, (void *)E_Turbidity );
    turbInput->minimum( 1.0 );
    turbInput->maximum( 60.0 );
    turbInput->step( 0.1 );
    turbInput->value(ephemData->turbidity);

    static Fl_Menu_Item phase_choices[] = {
      {"January",  0, 0L, 0L},
      {"Feburary", 0, 0L, 0L},
      {"March",    0, 0L, 0L},
      {"April",    0, 0L, 0L},
      {"May",      0, 0L, 0L},
      {"June",     0, 0L, 0L},
      {"July",     0, 0L, 0L},
      {"August",   0, 0L, 0L},
      {"September",0, 0L, 0L},
      {"October",  0, 0L, 0L},
      {"November", 0, 0L, 0L},
      {"December", 0, 0L, 0L},
      {0}
    };

    month_choice = new Fl_Choice( 10, 120, 100, 25, "" );
    month_choice->menu( phase_choices );
    month_choice->value( ephemData->dateTime.getMonth() - 1 );
    month_choice->callback( s_setMonthCallback, 0L );

    monthDayInput = new Fl_Value_Input( 115, 120, 25, 20, "" );
    monthDayInput->box( FL_EMBOSSED_BOX );
    monthDayInput->minimum( 1 );
    monthDayInput->maximum( 31 );
    monthDayInput->step( 1 );
    monthDayInput->value( ephemData->dateTime.getDayOfMonth());
    monthDayInput->callback( s_valueInputCallback, (void *)E_MonthDay );

    yearInput = new Fl_Value_Input( 145, 120, 40, 20, "" );
    yearInput->box( FL_EMBOSSED_BOX );
    yearInput->minimum( 0 );
    yearInput->maximum( 9999 );
    yearInput->step( 1 );
    yearInput->value(ephemData->dateTime.getYear());
    yearInput->callback( s_valueInputCallback, (void *)E_Year );

    droller = new Fl_Roller( 50, 150, 100, 15 );
    droller->minimum(1);
    droller->maximum(365);
    droller->step(0.25);
    droller->type( FL_HORIZONTAL );
    droller->callback( s_updateDateByDoy, (void *)0L );

    hourInput = new Fl_Value_Input( 50, 175, 25, 20, "" );
    hourInput->box( FL_EMBOSSED_BOX );
    hourInput->minimum( 0 );
    hourInput->maximum( 23 );
    hourInput->step( 1 );
    hourInput->value(12);
    hourInput->callback( s_valueInputCallback, (void *)E_Hour );

    new Fl_Box( 73, 178, 10, 10, ":" );

    minInput = new Fl_Value_Input( 80, 175, 25, 20, "" );
    minInput->box( FL_EMBOSSED_BOX );
    minInput->minimum( 0 );
    minInput->maximum( 59 );
    minInput->step( 1 );
    minInput->value(12);
    minInput->callback( s_valueInputCallback, (void *)E_Minute );

    new Fl_Box( 103, 178, 10, 10, ":" );

    secInput = new Fl_Value_Input( 110, 175, 25, 20, "" );
    secInput->box( FL_EMBOSSED_BOX );
    secInput->minimum( 0 );
    secInput->maximum( 59 );
    secInput->step( 1 );
    secInput->value(0);
    secInput->callback( s_valueInputCallback, (void *)E_Second );

    troller = new Fl_Roller( 50, 200, 100, 15 );
    troller->type( FL_HORIZONTAL );
    troller->minimum(1);
    troller->maximum(86399);
    troller->step(60);
    troller->callback( s_updateTimeBySec, (void *)0L );

    Fl_Button *nowToggle = new Fl_Button( 80, 220, 40, 20, "Now" );
    nowToggle->type( FL_TOGGLE_BUTTON );
    nowToggle->callback( s_toggleCallback, 0L );

    sync();

    window->end();
    window->show();

    return Fl::run();
}
