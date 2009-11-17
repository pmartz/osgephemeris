/**************************************************************************
 * Written by Durk Talsma. Originally started October 1997, for distribution  
 * with the FlightGear project. Version 2 was written in August and 
 * September 1998. This code is based upon algorithms and data kindly 
 * provided by Mr. Paul Schlyter. (pausch@saaf.se). 
 *
 * This code has been repackaged for use with osgEphemeris by Don Burns
 * November 25, 2005 by placing all celestial body classes into one
 * header file and one source file.  
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 **************************************************************************/
#include <osgEphemeris/CelestialBodies.h>
#include <osg/Math>


using namespace osgEphemeris;

/**************************************************************************
 * Written by Durk Talsma. Originally started October 1997, for distribution        
 * with the FlightGear project. Version 2 was written in August and 
 * September 1998. This code is based upon algorithms and data kindly 
 * provided by Mr. Paul Schlyter. (pausch@saaf.se). 
 **************************************************************************/


/**************************************************************************
 * void CelestialBody::updatePosition(double mjd, Sun *ourSun)
 *
 * Basically, this member function provides a general interface for 
 * calculating the right ascension and declinaion. This function is 
 * used for calculating the planetary positions. For the planets, an 
 * overloaded member function is provided to additionally calculate the
 * planet's magnitude. 
 * The sun and moon have their own overloaded updatePosition member, as their
 * position is calculated an a slightly different manner.    
 *
 * arguments:
 * double mjd: provides the modified julian date.
 * Sun *ourSun: the sun's position is needed to convert heliocentric 
 *                             coordinates into geocentric coordinates.
 *
 * return value: none
 *
 *************************************************************************/

void CelestialBody::updatePosition(double mjd, osgEphemeris::Sun *ourSun)
{
    double eccAnom, v, ecl, actTime, 
        xv, yv, xh, yh, zh, xg, yg, zg, xe, ye, ze;

    updateOrbElements(mjd);
    actTime = sgCalcActTime(mjd);

    // calcualate the angle bewteen ecliptic and equatorial coordinate system
    ecl = osg::DegreesToRadians((23.4393 - 3.563E-7 *actTime));
    
    eccAnom = sgCalcEccAnom(M, e);    //calculate the eccentric anomaly
    xv = a * (cos(eccAnom) - e);
    yv = a * (sqrt (1.0 - e*e) * sin(eccAnom));
    v = atan2(yv, xv);                     // the planet's true anomaly
    r = sqrt (xv*xv + yv*yv);        // the planet's distance
    
    // calculate the planet's position in 3D space
    xh = r * (cos(N) * cos(v+w) - sin(N) * sin(v+w) * cos(i));
    yh = r * (sin(N) * cos(v+w) + cos(N) * sin(v+w) * cos(i));
    zh = r * (sin(v+w) * sin(i));

    // calculate the ecliptic longitude and latitude
    xg = xh + ourSun->getxs();
    yg = yh + ourSun->getys();
    zg = zh;

    lonEcl = atan2(yh, xh);
    latEcl = atan2(zh, sqrt(xh*xh+yh*yh));

    xe = xg;
    ye = yg * cos(ecl) - zg * sin(ecl);
    ze = yg * sin(ecl) + zg * cos(ecl);
    rightAscension = atan2(ye, xe);
    declination = atan2(ze, sqrt(xe*xe + ye*ye));
    /* SG_LOG(SG_GENERAL, SG_INFO, "Planet found at : " 
     << rightAscension << " (ra), " << declination << " (dec)" ); */

    //calculate some variables specific to calculating the magnitude 
    //of the planet
    R = sqrt (xg*xg + yg*yg + zg*zg);
    s = ourSun->getDistance();

    // It is possible from these calculations for the argument to acos
    // to exceed the valid range for acos(). So we do a little extra
    // checking.

    double tmp = (r*r + R*R - s*s) / (2*r*R);
    if ( tmp > 1.0) 
    {
        tmp = 1.0;
    } 
    else if ( tmp < -1.0) 
    {
        tmp = -1.0;
    }

    FV = osg::RadiansToDegrees(acos( tmp ));
}

/****************************************************************************
 * double CelestialBody::sgCalcEccAnom(double M, double e)
 * this private member calculates the eccentric anomaly of a celestial body, 
 * given its mean anomaly and eccentricity.
 * 
 * -Mean anomaly: the approximate angle between the perihelion and the current
 *    position. this angle increases uniformly with time.
 *
 * True anomaly: the actual angle between perihelion and current position.
 *
 * Eccentric anomaly: this is an auxilary angle, used in calculating the true
 * anomaly from the mean anomaly.
 * 
 * -eccentricity. Indicates the amount in which the orbit deviates from a 
 *    circle (0 = circle, 0-1, is ellipse, 1 = parabola, > 1 = hyperbola).
 *
 * This function is also known as solveKeplersEquation()
 *
 * arguments: 
 * M: the mean anomaly
 * e: the eccentricity
 *
 * return value:
 * the eccentric anomaly
 *
 ****************************************************************************/
double CelestialBody::sgCalcEccAnom(double M, double e)
{
    double eccAnom, E0, E1, diff;

    double epsilon = osg::DegreesToRadians(0.001);
    
    eccAnom = M + e * sin(M) * (1.0 + e * cos (M));
    // iterate to achieve a greater precision for larger eccentricities 
    if (e > 0.05)
    {
        E0 = eccAnom;
        do
        {
             E1 = E0 - (E0 - e * sin(E0) - M) / (1 - e *cos(E0));
             diff = fabs(E0 - E1);
             E0 = E1;
        } while (diff > epsilon );
        return E0;
    }
    return eccAnom;
}

/*****************************************************************************
 * inline CelestialBody::CelestialBody
 * public constructor for a generic celestialBody object.
 * initializes the 6 primary orbital elements. The elements are:
 * N: longitude of the ascending node
 * i: inclination to the ecliptic
 * w: argument of perihelion
 * a: semi-major axis, or mean distance from the sun
 * e: eccenticity
 * M: mean anomaly
 * Each orbital element consists of a constant part and a variable part that 
 * gradually changes over time. 
 *
 * Argumetns:
 * the 13 arguments to the constructor constitute the first, constant 
 * ([NiwaeM]f) and the second variable ([NiwaeM]s) part of the orbital 
 * elements. The 13th argument is the current time. Note that the inclination
 * is written with a capital (If, Is), because 'if' is a reserved word in the 
 * C/C++ programming language.
 ***************************************************************************/ 
CelestialBody::CelestialBody(double Nf, double Ns,
                        double If, double Is,
                        double wf, double ws,
                        double af, double as,
                        double ef, double es,
                        double Mf, double Ms, double mjd)
{
    NFirst = Nf;         NSec = Ns;
    iFirst = If;         iSec = Is;
    wFirst = wf;         wSec = ws;
    aFirst = af;         aSec = as;
    eFirst = ef;         eSec = es;
    MFirst = Mf;         MSec = Ms;
    updateOrbElements(mjd);
}

CelestialBody::CelestialBody(double Nf, double Ns,
                        double If, double Is,
                        double wf, double ws,
                        double af, double as,
                        double ef, double es,
                        double Mf, double Ms)
{
    NFirst = Nf;         NSec = Ns;
    iFirst = If;         iSec = Is;
    wFirst = wf;         wSec = ws;
    aFirst = af;         aSec = as;
    eFirst = ef;         eSec = es;
    MFirst = Mf;         MSec = Ms;
}

/****************************************************************************
 * inline void CelestialBody::updateOrbElements(double mjd)
 * given the current time, this private member calculates the actual 
 * orbital elements
 *
 * Arguments: double mjd: the current modified julian date:
 *
 * return value: none
 ***************************************************************************/
void CelestialBody::updateOrbElements(double mjd)
{
    double actTime = sgCalcActTime(mjd);
     M = osg::DegreesToRadians( (MFirst + (MSec * actTime)) );
     w = osg::DegreesToRadians( (wFirst + (wSec * actTime)) );
     N = osg::DegreesToRadians( (NFirst + (NSec * actTime)) );
     i = osg::DegreesToRadians( (iFirst + (iSec * actTime)) );
     e = eFirst + (eSec * actTime);
     a = aFirst + (aSec * actTime);
}

/*****************************************************************************
 * inline double CelestialBody::sgCalcActTime(double mjd)
 * this private member function returns the offset in days from the epoch for
 * wich the orbital elements are calculated (Jan, 1st, 2000).
 * 
 * Argument: the current time
 *
 * return value: the (fractional) number of days until Jan 1, 2000.
 ****************************************************************************/
double CelestialBody::sgCalcActTime(double mjd)
{
    return (mjd - 36523.5);
}

/*****************************************************************************
 * inline void CelestialBody::getPos(double* ra, double* dec)
 * gives public access to Right Ascension and declination
 *
 ****************************************************************************/
void CelestialBody::getPos(double* ra, double* dec) const 
{
    *ra    = rightAscension;
    *dec = declination;
}

/*****************************************************************************
 * inline void CelestialBody::getPos(double* ra, double* dec, double* magnitude
 * gives public acces to the current Right ascension, declination, and 
 * magnitude
 ****************************************************************************/
void CelestialBody::getPos(double* ra, double* dec, double* magn) const
{
    *ra = rightAscension;
    *dec = declination;
    *magn = magnitude;
}


/*************************************************************************
 * Sun::Sun(double mjd)
 * Public constructor for class Sun
 * Argument: The current time.
 * the hard coded orbital elements our sun are passed to 
 * CelestialBody::CelestialBody();

 *** (old note)
 *** note that the word sun is avoided, in order to prevent some compilation
 *** problems on sun systems 
 ***

 *
 * DB (November 25, 2005): Note that Star has been returned to 'Sun' and the use of namespace
 * is used to avoid conflict on Sun Microsystem systems
 *
 ************************************************************************/
Sun::Sun(double mjd) :
        CelestialBody (0.000000,        0.0000000000,
   0.0000,                0.00000,
   282.9404,        4.7093500E-5,    
   1.0000000, 0.000000,    
   0.016709,        -1.151E-9,
   356.0470,        0.98560025850, mjd)
{
                distance = 0.0;
}

Sun::Sun() :
                CelestialBody (0.000000,        0.0000000000,
                 0.0000,                0.00000,
                 282.9404,        4.7093500E-5,    
                 1.0000000, 0.000000,    
                 0.016709,        -1.151E-9,
                 356.0470,        0.98560025850)
{
                distance = 0.0;
}

Sun::~Sun()
{
}


/*************************************************************************
 * void Sun::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of our sun.
 *************************************************************************/
void Sun::updatePosition(double mjd)
{
        double 
                actTime, eccAnom, 
                xv, yv, v, r,
                xe, ye, ze, ecl;

        updateOrbElements(mjd);
        
        actTime = sgCalcActTime(mjd);
        ecl = osg::DegreesToRadians((23.4393 - 3.563E-7 * actTime)); // Angle in Radians
        eccAnom = sgCalcEccAnom(M, e);        // Calculate the eccentric Anomaly (also known as solving Kepler's equation)
        
        xv = cos(eccAnom) - e;
        yv = sqrt (1.0 - e*e) * sin(eccAnom);
        v = atan2 (yv, xv);                                                                         // the sun's true anomaly
        distance = r = sqrt (xv*xv + yv*yv);        // and its distance

        lonEcl = v + w; // the sun's true longitude
        latEcl = 0;

        // convert the sun's true longitude to ecliptic rectangular 
        // geocentric coordinates (xs, ys)
        xs = r * cos (lonEcl);
        ys = r * sin (lonEcl);

        // convert ecliptic coordinates to equatorial rectangular
        // geocentric coordinates

        xe = xs;
        ye = ys * cos (ecl);
        ze = ys * sin (ecl);

        // And finally, calculate right ascension and declination
        rightAscension = atan2 (ye, xe);
        declination = atan2 (ze, sqrt (xe*xe + ye*ye));
}


/*************************************************************************
 * Moon::Moon(double mjd)
 * Public constructor for class Moon. Initializes the orbital elements 
 * Argument: The current time.
 * the hard coded orbital elements for Moon are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Moon::Moon(double mjd) :
    CelestialBody(125.1228, -0.0529538083,
        5.1454,        0.00000,
        318.0634,    0.1643573223,
        60.266600, 0.000000,
        0.054900,    0.000000,
        115.3654,    13.0649929509, mjd)
{
}

Moon::Moon() :
    CelestialBody(125.1228, -0.0529538083,
        5.1454,        0.00000,
        318.0634,    0.1643573223,
        60.266600, 0.000000,
        0.054900,    0.000000,
        115.3654,    13.0649929509)
{
}


Moon::~Moon()
{
}


/*****************************************************************************
 * void Moon::updatePosition(double mjd, Star *ourSun)
 * this member function calculates the actual topocentric position (i.e.) 
 * the position of the moon as seen from the current position on the surface
 * of the moon. 
 ****************************************************************************/
void Moon::updatePosition(double mjd, double lst, double lat, Sun *ourSun)
{
    double 
        eccAnom, ecl, actTime,
        xv, yv, v, r, xh, yh, zh, xg, yg, zg, xe, ye, ze,
        Ls, Lm, D, F, mpar, gclat, rho, HA, g,
        geoRa, geoDec;
    
    updateOrbElements(mjd);
    actTime = sgCalcActTime(mjd);

    // calculate the angle between ecliptic and equatorial coordinate system
    // in Radians
    ecl = ((osg::DegreesToRadians(23.4393)) - (osg::DegreesToRadians(3.563E-7) * actTime));    
    eccAnom = sgCalcEccAnom(M, e);    // Calculate the eccentric anomaly
    xv = a * (cos(eccAnom) - e);
    yv = a * (sqrt(1.0 - e*e) * sin(eccAnom));
    v = atan2(yv, xv);                             // the moon's true anomaly
    r = sqrt (xv*xv + yv*yv);             // and its distance
    
    // estimate the geocentric rectangular coordinates here
    xh = r * (cos(N) * cos (v+w) - sin (N) * sin(v+w) * cos(i));
    yh = r * (sin(N) * cos (v+w) + cos (N) * sin(v+w) * cos(i));
    zh = r * (sin(v+w) * sin(i));

    // calculate the ecliptic latitude and longitude here
    lonEcl = atan2 (yh, xh);
    latEcl = atan2(zh, sqrt(xh*xh + yh*yh));

    /* Calculate a number of perturbatioin, i.e. disturbances caused by the 
     * gravitational infuence of the sun and the other major planets.
     * The largest of these even have a name */
    Ls = ourSun->getM() + ourSun->getw();
    Lm = M + w + N;
    D = Lm - Ls;
    F = Lm - N;
    
    lonEcl += osg::DegreesToRadians((-1.274 * sin (M - 2*D)
                +0.658 * sin (2*D)
                -0.186 * sin(ourSun->getM())
                -0.059 * sin(2*M - 2*D)
                -0.057 * sin(M - 2*D + ourSun->getM())
                +0.053 * sin(M + 2*D)
                +0.046 * sin(2*D - ourSun->getM())
                +0.041 * sin(M - ourSun->getM())
                -0.035 * sin(D)
                -0.031 * sin(M + ourSun->getM())
                -0.015 * sin(2*F - 2*D)
                +0.011 * sin(M - 4*D)
                ));
    latEcl += osg::DegreesToRadians( (-0.173 * sin(F-2*D)
                -0.055 * sin(M - F - 2*D)
                -0.046 * sin(M + F - 2*D)
                +0.033 * sin(F + 2*D)
                +0.017 * sin(2*M + F)
                ) );
    r += (-0.58 * cos(M - 2*D)
    -0.46 * cos(2*D)
    );
    xg = r * cos(lonEcl) * cos(latEcl);
    yg = r * sin(lonEcl) * cos(latEcl);
    zg = r *                             sin(latEcl);
    
    xe = xg;
    ye = yg * cos(ecl) -zg * sin(ecl);
    ze = yg * sin(ecl) +zg * cos(ecl);

    geoRa    = atan2(ye, xe);
    geoDec = atan2(ze, sqrt(xe*xe + ye*ye));


    // Given the moon's geocentric ra and dec, calculate its 
    // topocentric ra and dec. i.e. the position as seen from the
    // surface of the earth, instead of the center of the earth

    // First calculate the moon's parrallax, that is, the apparent size of the 
    // (equatorial) radius of the earth, as seen from the moon 
    mpar = asin ( 1 / r);

    gclat = lat - 0.003358 * 
            sin (2 * osg::DegreesToRadians( lat ) );

    rho = 0.99883 + 0.00167 * cos(2 * osg::DegreesToRadians(lat));
    
    if (geoRa < 0)
        geoRa += (2*osg::PI);
    
    HA = lst - (3.8197186 * geoRa);

    g = atan (tan(gclat) / cos ((HA / 3.8197186)));

    rightAscension = geoRa - mpar * rho * cos(gclat) * sin(HA) / cos (geoDec);
    if (fabs(lat) > 0) 
    {
        declination = geoDec - mpar * rho * sin (gclat) * sin (g - geoDec) / sin(g);
    } 
    else 
    {
        declination = geoDec;
    }
}

/*************************************************************************
 * Mercury::Mercury(double mjd)
 * Public constructor for class Mercury
 * Argument: The current time.
 * the hard coded orbital elements for Mercury are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Mercury::Mercury(double mjd) :
        CelestialBody (48.33130,         3.2458700E-5,
            7.0047,                5.00E-8,
            29.12410,        1.0144400E-5,
            0.3870980, 0.000000,
            0.205635,        5.59E-10,
            168.6562,        4.09233443680, mjd)
{
}
Mercury::Mercury() :
        CelestialBody (48.33130,         3.2458700E-5,
            7.0047,                5.00E-8,
            29.12410,        1.0144400E-5,
            0.3870980, 0.000000,
            0.205635,        5.59E-10,
            168.6562,        4.09233443680)
{
}

/*************************************************************************
 * void Mercury::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Mercury, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Mercury specific equation
 *************************************************************************/
void Mercury::updatePosition(double mjd, Sun *ourSun)
{
        CelestialBody::updatePosition(mjd, ourSun);
        magnitude = -0.36 + 5*log10( r*R ) + 0.027 * FV + 2.2E-13 * pow(FV, 6); 
}

/*************************************************************************
 * Venus::Venus(double mjd)
 * Public constructor for class Venus
 * Argument: The current time.
 * the hard coded orbital elements for Venus are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Venus::Venus(double mjd) :
        CelestialBody(76.67990,        2.4659000E-5, 
        3.3946,                2.75E-8,
        54.89100,        1.3837400E-5,
        0.7233300, 0.000000,
        0.006773, -1.302E-9,
        48.00520,        1.60213022440, mjd)
{
}
Venus::Venus() :
        CelestialBody(76.67990,        2.4659000E-5, 
        3.3946,                2.75E-8,
        54.89100,        1.3837400E-5,
        0.7233300, 0.000000,
        0.006773, -1.302E-9,
        48.00520,        1.60213022440)
{
}

/*************************************************************************
 * void Venus::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Venus, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Venus specific equation
 *************************************************************************/
void Venus::updatePosition(double mjd, Sun *ourSun)
{
        CelestialBody::updatePosition(mjd, ourSun);
        magnitude = -4.34 + 5*log10( r*R ) + 0.013 * FV + 4.2E-07 * pow(FV,3);
}

/*************************************************************************
 * Mars::Mars(double mjd)
 * Public constructor for class Mars
 * Argument: The current time.
 * the hard coded orbital elements for Mars are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Mars::Mars(double mjd) :
        CelestialBody(49.55740,        2.1108100E-5,
        1.8497,         -1.78E-8,
        286.5016,        2.9296100E-5,
        1.5236880, 0.000000,
        0.093405,        2.516E-9,
        18.60210,        0.52402077660, mjd)
{
}

Mars::Mars() :
        CelestialBody(49.55740,        2.1108100E-5,
        1.8497,         -1.78E-8,
        286.5016,        2.9296100E-5,
        1.5236880, 0.000000,
        0.093405,        2.516E-9,
        18.60210,        0.52402077660)
{
}

/*************************************************************************
 * void Mars::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Mars, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Mars specific equation
 *************************************************************************/
void Mars::updatePosition(double mjd, Sun *ourSun)
{
        CelestialBody::updatePosition(mjd, ourSun);
        magnitude = -1.51 + 5*log10( r*R ) + 0.016 * FV;
}

/*************************************************************************
 * Jupiter::Jupiter(double mjd)
 * Public constructor for class Jupiter
 * Argument: The current time.
 * the hard coded orbital elements for Jupiter are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Jupiter::Jupiter(double mjd) :
        CelestialBody(100.4542,        2.7685400E-5,    
        1.3030,         -1.557E-7,
        273.8777,        1.6450500E-5,
        5.2025600, 0.000000,
        0.048498,        4.469E-9,
        19.89500,        0.08308530010, mjd)
{
}

Jupiter::Jupiter() :
        CelestialBody(100.4542,        2.7685400E-5,    
        1.3030,         -1.557E-7,
        273.8777,        1.6450500E-5,
        5.2025600, 0.000000,
        0.048498,        4.469E-9,
        19.89500,        0.08308530010)
{
}

/*************************************************************************
 * void Jupiter::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Jupiter, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Jupiter specific equation
 *************************************************************************/
void Jupiter::updatePosition(double mjd, Sun *ourSun)
{
        CelestialBody::updatePosition(mjd, ourSun);
        magnitude = -9.25 + 5*log10( r*R ) + 0.014 * FV;
}

/*************************************************************************
 * Saturn::Saturn(double mjd)
 * Public constructor for class Saturn
 * Argument: The current time.
 * the hard coded orbital elements for Saturn are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Saturn::Saturn(double mjd) :
        CelestialBody(113.6634,         2.3898000E-5,
        2.4886,             -1.081E-7,
        339.3939,         2.9766100E-5,
        9.5547500,        0.000000,
        0.055546,        -9.499E-9,
        316.9670,         0.03344422820, mjd)
{
}

Saturn::Saturn() :
        CelestialBody(113.6634,         2.3898000E-5,
        2.4886,             -1.081E-7,
        339.3939,         2.9766100E-5,
        9.5547500,        0.000000,
        0.055546,        -9.499E-9,
        316.9670,         0.03344422820)
{
}

/*************************************************************************
 * void Saturn::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Saturn, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Saturn specific equation
 *************************************************************************/
void Saturn::updatePosition(double mjd, Sun *ourSun)
{
        CelestialBody::updatePosition(mjd, ourSun);
        
        double actTime = sgCalcActTime(mjd);
        double ir = 0.4897394;
        double Nr = 2.9585076 + 6.6672E-7*actTime;
        double B = asin (sin(declination) * cos(ir) - 
                 cos(declination) * sin(ir) *
                 sin(rightAscension - Nr));
        double ring_magn = -2.6 * sin(fabs(B)) + 1.2 * pow(sin(B), 2);
        magnitude = -9.0 + 5*log10(r*R) + 0.044 * FV + ring_magn;
}

/*************************************************************************
 * Uranus::Uranus(double mjd)
 * Public constructor for class Uranus
 * Argument: The current time.
 * the hard coded orbital elements for Uranus are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Uranus::Uranus(double mjd) :
        CelestialBody(74.00050,         1.3978000E-5,
        0.7733,                 1.900E-8,
        96.66120,         3.0565000E-5,
        19.181710, -1.55E-8,
        0.047318,         7.450E-9,
        142.5905,         0.01172580600, mjd)
{
}

Uranus::Uranus() :
        CelestialBody(74.00050,         1.3978000E-5,
        0.7733,                 1.900E-8,
        96.66120,         3.0565000E-5,
        19.181710, -1.55E-8,
        0.047318,         7.450E-9,
        142.5905,         0.01172580600)
{
}

/*************************************************************************
 * void Uranus::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Uranus, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Uranus specific equation
 *************************************************************************/
void Uranus::updatePosition(double mjd, Sun *ourSun)
{
        CelestialBody::updatePosition(mjd, ourSun);
        magnitude = -7.15 + 5*log10( r*R) + 0.001 * FV;
}

/*************************************************************************
 * Neptune::Neptune(double mjd)
 * Public constructor for class Neptune
 * Argument: The current time.
 * the hard coded orbital elements for Neptune are passed to 
 * CelestialBody::CelestialBody();
 ************************************************************************/
Neptune::Neptune(double mjd) :
        CelestialBody(131.7806,         3.0173000E-5,
        1.7700,             -2.550E-7,
        272.8461,        -6.027000E-6,    
        30.058260,        3.313E-8,
        0.008606,         2.150E-9,
        260.2471,         0.00599514700, mjd)
{
}

Neptune::Neptune() :
        CelestialBody(131.7806,         3.0173000E-5,
        1.7700,             -2.550E-7,
        272.8461,        -6.027000E-6,    
        30.058260,        3.313E-8,
        0.008606,         2.150E-9,
        260.2471,         0.00599514700)
{
}

/*************************************************************************
 * void Neptune::updatePosition(double mjd, Sun *ourSun)
 * 
 * calculates the current position of Neptune, by calling the base class,
 * CelestialBody::updatePosition(); The current magnitude is calculated using 
 * a Neptune specific equation
 *************************************************************************/
void Neptune::updatePosition(double mjd, Sun *ourSun)
{
    CelestialBody::updatePosition(mjd, ourSun);
    magnitude = -6.90 + 5*log10 (r*R) + 0.001 *FV;
}



