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

#include <string.h>
#include <osg/Math>
#include <osgEphemeris/EphemerisEngine.h>

using namespace osgEphemeris;

EphemerisEngine::EphemerisEngine( EphemerisData *ephemerisData):
    _ephemerisData(ephemerisData),
    _sun     ( new osgEphemeris::Sun ),
    _moon    ( new Moon ),
    _mercury ( new Mercury ),
    _venus   ( new Venus ),
    _mars    ( new Mars ),
    _jupiter ( new Jupiter ),
    _saturn  ( new Saturn ),
    _uranus  ( new Uranus ),
    _neptune ( new Neptune )
{
    if( _ephemerisData != 0L )
    {
        strcpy( _ephemerisData->data[CelestialBodyNames::Sun].name,      "Sun");
        strcpy( _ephemerisData->data[CelestialBodyNames::Moon].name,     "Moon");
        strcpy( _ephemerisData->data[CelestialBodyNames::Mercury].name,  "Mercury");
        strcpy( _ephemerisData->data[CelestialBodyNames::Venus].name,    "Venus");
        strcpy( _ephemerisData->data[CelestialBodyNames::Mars].name,     "Mars");
        strcpy( _ephemerisData->data[CelestialBodyNames::Jupiter].name,  "Jupiter");
        strcpy( _ephemerisData->data[CelestialBodyNames::Saturn].name,   "Saturn");
        strcpy( _ephemerisData->data[CelestialBodyNames::Uranus].name,   "Uranus");
        strcpy( _ephemerisData->data[CelestialBodyNames::Neptune].name,  "Neptune");
        strcpy( _ephemerisData->data[CelestialBodyNames::Pluto].name,    "Pluto");
    }
}

void EphemerisEngine::setLatitude( double latitude )
{
    if( _ephemerisData != 0L )
        _ephemerisData->latitude = latitude;
}

void EphemerisEngine::setLongitude( double longitude )
{
    if( _ephemerisData != 0L )
        _ephemerisData->longitude = longitude;
}


void EphemerisEngine::setLatitudeLongitude( double latitude, double longitude )
{
    if( _ephemerisData != 0L )
    {
        _ephemerisData->latitude  = latitude;
        _ephemerisData->longitude = longitude;
        _ephemerisData->altitude  = 0.0;
    }
}

void EphemerisEngine::setLatitudeLongitudeAltitude( double latitude, double longitude, double altitude )
{
    if( _ephemerisData != 0L )
    {
        _ephemerisData->latitude  = latitude;
        _ephemerisData->longitude = longitude;
        _ephemerisData->altitude  = altitude;
    }
}

void EphemerisEngine::setDateTime()
{
    if( _ephemerisData != 0L )
    {
        _ephemerisData->dateTime.now();
    }
}

void EphemerisEngine::setDateTime( const DateTime &dateTime )
{
    if( _ephemerisData != 0L )
    {
        _ephemerisData->dateTime = dateTime;
    }
}


void EphemerisEngine::update( bool updateTime)
{
    if( _ephemerisData != 0L )
        update( _ephemerisData, updateTime );
}

void  EphemerisEngine::_updateData( 
        const EphemerisData &ephemData, 
        const CelestialBody &cb, 
        double rsn,
        CelestialBodyData &cbd )
{
    cbd.rightAscension  = cb.getRightAscension();
    cbd.declination     = cb.getDeclination();
    cbd.magnitude       = cb.getMagnitude();
    _RADecElevToAzimAlt(
            cbd.rightAscension,
            cbd.declination,
            osg::DegreesToRadians(ephemData.latitude),
            ephemData.localSiderealTime,
            ephemData.altitude,
            rsn,
            cbd.azimuth,
            cbd.alt );
}


void EphemerisEngine::update( EphemerisData *ephemData, bool updateTime )
{
    if( updateTime == true )
        ephemData->dateTime.now();

    ephemData->modifiedJulianDate = ephemData->dateTime.getModifiedJulianDate();
    double rsn, lsn;
    _getLsnRsn( ephemData->modifiedJulianDate, lsn, rsn );

    ephemData->localSiderealTime = 
        getLocalSiderealTimePrecise( ephemData->modifiedJulianDate, ephemData->longitude ); // in degrees

    _sun    ->updatePosition( ephemData->modifiedJulianDate );
    _updateData( *ephemData, *_sun.get(), rsn, ephemData->data[CelestialBodyNames::Sun]   );

    _moon   ->updatePosition( ephemData->modifiedJulianDate, ephemData->localSiderealTime, 
            osg::DegreesToRadians(ephemData->latitude), _sun.get() );
    _updateData( *ephemData, *_moon.get(), rsn, ephemData->data[CelestialBodyNames::Moon]   );

    _mercury->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_mercury.get(), rsn, ephemData->data[CelestialBodyNames::Mercury]   );

    _venus  ->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_venus.get(), rsn, ephemData->data[CelestialBodyNames::Venus]   );

    _mars   ->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_mars.get(), rsn, ephemData->data[CelestialBodyNames::Mars]   );

    _jupiter->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_jupiter.get(), rsn, ephemData->data[CelestialBodyNames::Jupiter]   );

    _saturn ->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_saturn.get(), rsn, ephemData->data[CelestialBodyNames::Saturn]   );

    _uranus ->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_uranus.get(), rsn, ephemData->data[CelestialBodyNames::Uranus]   );

    _neptune->updatePosition( ephemData->modifiedJulianDate, _sun.get() );
    _updateData( *ephemData, *_neptune.get(), rsn, ephemData->data[CelestialBodyNames::Neptune]   );


#if 0
#define rad2deg(x)  ((x)*180.0/osg::PI)
    printf("\033[0;0H\n" );
    printf( "%16s %16s %16s %16s %16s %16s\n", "Body", "Right Ascension", "Declination", "Magnitude", "Azimuth", "Alt" );
    printf( "%16s %16s %16s %16s %16s %16s\n", "----", "---------------", "-----------", "---------", "-------", "---" );
    printf( "%16s %16.4lf %16.4lf %16lf %16.4lf %16.4lf\n",
                ephemData->data[osgEphemeris::CelestialBodyNames::Sun].name,
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Sun].rightAscension ),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Sun].declination ),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Sun].magnitude ),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Sun].azimuth),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Sun].alt) );


    printf( "%16s %16.4lf %16.4lf %16lf %16.4lf %16.4lf\n",
                ephemData->data[osgEphemeris::CelestialBodyNames::Moon].name,
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Moon].rightAscension ),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Moon].declination ),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Moon].magnitude ),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Moon].azimuth),
                rad2deg(ephemData->data[osgEphemeris::CelestialBodyNames::Moon].alt) );
#endif

}



/* given the modified JD, mjd, return the true geocentric ecliptic longitude
*   of the sun for the mean equinox of the date, *lsn, in radians, and the
*   sun-earth distance, *rsn, in AU. (the true ecliptic latitude is never more
*   than 1.2 arc seconds and so may be taken to be a constant 0.)
* if the APPARENT ecliptic longitude is required, correct the longitude for
*   nutation to the true equinox of date and for aberration (light travel time,
*   approximately  -9.27e7/186000/(3600*24*365)*2*pi = -9.93e-5 radians).
*/
void EphemerisEngine::_getLsnRsn( double mjd, double &lsn, double &rsn)
{
   double t, t2;
   double ls, ms;    /* mean longitude and mean anomaly */
   double s, nu, ea; /* eccentricity, true anomaly, eccentric anomaly */
   double a, b, a1, b1, c1, d1, e1, h1, dl, dr;

#define range(x, r)     ((x) -= (r)*floor((x)/(r)))

   t = mjd/36525.;
   t2 = t*t;
   a = 100.0021359 * t;
   b = 360. * ( a - (int)a );
   ls = 279.69668f + (.0003025 * t2) + b;
   a = 99.99736042000039 * t;
   b = 360. * ( a - (int)a);
   ms = 358.47583 - (.00015+.0000033 * t * t2) +b;
   s = .016751 - (.0000418*t) - (1.26e-07 * t2);

   _getAnomaly(osg::DegreesToRadians(ms), s, nu, ea);

   a = 62.55209472000015 * t;
   b = 360. * (a - (int)a);
   a1 = osg::DegreesToRadians(153.23 + b);

   a = 125.1041894 * t;
   b = 360*(a - (int)a);
   b1 = osg::DegreesToRadians(216.57 + b);

   a = 91.56766028 * t;
   b = 360 * (a - (int)a);
   c1 = osg::DegreesToRadians(312.69 + b);

   a = 1236.853095*t;
   b = 360*(a-(int)a);
   d1 = osg::DegreesToRadians(350.74 - (.00144 * t2) + b);
   e1 = osg::DegreesToRadians(231.19 + (20.2*t));

   a = 183.1353208 * t;
   b = 360 * (a - (int)a);
   h1 = osg::DegreesToRadians(353.4 + b);
   dl = .00134 * (cos(a1) + .00154) * (cos(b1) + .002) * cos(c1) + (.00179 * sin(d1)) + (.00178 * sin(e1));
   dr = (5.43e-06 * sin(a1)) + (1.575e-05 * sin(b1)) + (1.627e-05 * sin(c1)) + (3.076e-05 * cos(d1)) + (9.27e-06 * sin(h1));
   lsn = nu + osg::DegreesToRadians(ls - ms + dl);
   range(lsn, 2.0*osg::PI);
   rsn = 1.0000002 * (1 - s * cos(ea)) + dr;
}

/* given the mean anomaly, ma, and the eccentricity, s, of elliptical motion,
* find the true anomaly, *nu, and the eccentric anomaly, *ea.
* all angles in radians.
*/
void EphemerisEngine::_getAnomaly( double ma, double s, double &nu, double &ea)
{
    double m, fea;

    m = ma-(2.f*osg::PI)*(int)(ma/(2.f*osg::PI));

    if (m > osg::PI) 
        m -= (2.0*osg::PI);

    if (m < -osg::PI) 
        m += (2.0*osg::PI);

    fea = m;

    if (s < 1.0) 
    {
      /* elliptical */
        double dla;
        for (;;) 
        {
            dla = fea-(s*sin(fea))-m;
            if (fabs(dla)<1e-6)
                break;
            dla /= 1-(s*cos(fea));
            fea -= dla;
        }
        nu = 2*atan((double)(sqrt((1+s)/(1-s))*tan(fea/2)));
    } 
    else 
    {
      /* hyperbolic */
        double corr = 1;
        while (fabs(corr) > 0.000001) 
        {
            corr = (m - s * sinh(fea) + fea) / (s*cosh(fea) - 1);
            fea += corr;
        }
        nu = 2*atan((double)(sqrt((s+1)/(s-1))*tanh(fea/2)));
   }
   ea = fea;
}


#if 0 // ORIGINAL VERSION BEFORE CHANGES - will test

double EphemerisEngine::getLocalSiderealTimePrecise( double mjd, double lng )
{
#define deghr(x)        ((x)/15.)
#define radhr(x)        deghr(osg::RadiansToDegrees(x))

    static const double MJD0    = 2415020.0;
    static const double J2000   = 2451545.0 - MJD0;
    static const double SIDRATE = 0.9972695677;

    // convert to required internal units
    lng = osg::DegreesToRadians(lng);

    double day = floor(mjd - 0.5) + 0.5;
    double hr = (mjd - day) * 24.0;
    double T, x;

    T = ((int)(mjd - 0.5) + 0.5 - J2000)/36525.0;
    x = 24110.54841 + (8640184.812866 + (0.093104 - 6.2e-6 * T) * T) * T;
    x /= 3600.0;
    double gst = (1.0/SIDRATE) * hr + x;
    double lst = gst - radhr( lng );

    lst -= 24.0 * floor( lst / 24.0 );

#undef deghr
#undef radhr
    return lst;
}

#else // Changed version of this function with Paul's suggested changes

double EphemerisEngine::getLocalSiderealTimePrecise( double mjd, double lng )
{
    static const double MJD0    = 2415020.0;
    static const double J2000   = 2451545.0 - MJD0;
    static const double SIDRATE = 0.9972695677;

    double day = floor(mjd - 0.5) + 0.5;
    double hr = (mjd - day) * 24.0;
    double T, x;

    T = ((int)(mjd - 0.5) + 0.5 - J2000)/36525.0;
    x = 24110.54841 + (8640184.812866 + (0.093104 - 6.2e-6 * T) * T) * T;
    x /= 3600.0;
    double gst = (1.0/SIDRATE) * hr + x;

    /// NOTE THE SIGN CHANGE IN THE NEXT LINE
    // 
    double lst = gst + lng/15.0; 
    //               ^
    // here ---------|

    lst -= 24.0 * floor( lst / 24.0 );

#undef deghr
#undef radhr
    return lst;
}
#endif


void EphemerisEngine::_RADecElevToAzimAlt( 
        double rightAscension,
        double declination,
        double latitude,
        double localSiderealTime,
        double elevation, // In meters above sea level
        double rsn,
        double &azim,
        double &alt )
{
    static const double meanRadiusOfEarthInMeters = 6378160.0;
    range(rightAscension, 2*osg::PI );
    double ha = (osg::DegreesToRadians(localSiderealTime) * 15.0) - rightAscension;
    double elev = elevation/meanRadiusOfEarthInMeters;

    static const double ddes = (2.0 * 6378.0 / 146.0e6);
    double ehp = ddes/rsn;

    double aha; // Apparent HA
    double adec;// Apparent declination
    _calcParallax(ha, declination, latitude, elev, ehp, aha, adec);
    _aaha_aux( latitude, aha, adec, &azim, &alt );
}

/* given true ha and dec, tha and tdec, the geographical latitude, phi, the
* height above sea-level (as a fraction of the earths radius, 6378.16km),
* ht, and the equatorial horizontal parallax, ehp, find the apparent
* ha and dec, aha and adec allowing for parallax.
* all angles in radians. ehp is the angle subtended at the body by the
* earth's equator.
*/
void EphemerisEngine::_calcParallax ( 
                    double tha, double tdec,    // True right ascension and declination
                    double phi, double ht,      // geographical latitude, height abouve sealevel
                    double ehp,                 // Equatorial horizontal parallax
                    double &aha, double &adec)  // output: aparent right ascencion and declination
{
    double cphi = cos(phi);
    double sphi = sin(phi);
    double u = atan(9.96647e-1*sphi/cphi);
    double rsp = (9.96647e-1*sin(u))+(ht*sphi);
    double rcp = cos(u)+(ht*cphi);

   double rp = 1/sin(ehp);  /* distance to object in Earth radii */
   double ctha = cos(tha);
   double stdec = sin(tdec);
   double ctdec = cos(tdec);
   double tdtha = (rcp*sin(tha))/((rp*ctdec)-(rcp*ctha));
   double dtha = atan(tdtha);

   aha = tha+dtha;
   double caha = cos(aha);
   range(aha, 2*osg::PI);
   adec = atan(caha*(rp*stdec-rsp)/(rp*ctdec*ctha-rcp));
}


/* the actual formula is the same for both transformation directions so
* do it here once for each way.
* N.B. all arguments are in radians.
//   lat = latitude, x = aparent right ascension, y = Declination, p = azimuth, q = altitude
*/
void EphemerisEngine::_aaha_aux (double lat, double x, double y, double *p, double *q)
{
   double sinlat = sin (lat);
   double coslat = cos (lat);

   double sy = sin (y);
   double cy = cos (y);
   double sx = sin (x);
   double cx = cos (x);

#define EPS (1e-20)
   double sq = (sy*sinlat) + (cy*coslat*cx);
   *q = asin (sq);
   double cq = cos (*q);
   double a = coslat*cq;
   if (a > -EPS && a < EPS)
      a = a < 0 ? -EPS : EPS; /* avoid / 0 */
   double cp = (sy - (sinlat*sq))/a;
   if (cp >= 1.0)   /* the /a can be slightly > 1 */
      *p = 0.0;
   else if (cp <= -1.0)
      *p = osg::PI;
   else
      *p = acos ((sy - (sinlat*sq))/a);
   if (sx>0) *p = 2.0*osg::PI - *p;
}
