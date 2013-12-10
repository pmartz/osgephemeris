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

#include <osg/MatrixTransform>
#include <osg/LightSource>
#include <osgEphemeris/EphemerisModel.h>

#include <osg/StateSet>
#include <osg/CullFace>
#include <osg/ComputeBoundsVisitor>

#include <osgUtil/Optimizer>

using namespace osgEphemeris;


EphemerisModel::EphemerisModel(const EphemerisModel& /*copy*/,
                              const osg::CopyOp& /*copyop*/ ):
    osg::Group()
{
}

EphemerisModel::EphemerisModel():
    _inited(false),
    _members(DEFAULT_MEMBERS),
    _scale(1.0),
    _center(0,0,0),
    _autoDateTime(false),
    _sunLightNum(0),
    _moonLightNum(1),
    _skyDomeUseSouthernHemisphere(true),
    _skyDomeMirrorSouthernHemisphere( true ),
    _sunFudgeScale(1.0),
    _moonFudgeScale(1.0)
{

    _ephemerisData   = new (EphemerisData::getDefaultShmemFileName()) EphemerisData;
    _ephemerisEngine = new EphemerisEngine(_ephemerisData);

    _skyTx = new osg::MatrixTransform;
     osg::StateSet* ssSky = _skyTx->getOrCreateStateSet();
     ssSky->setAttributeAndModes( new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON );

    _memberGroup = new osg::Group;

    // Set up clipping planes for reflections
    _clipPlaneTop = new osg::ClipPlane(0, 0.0, 0.0, 1.0, 0.0 );
    _clipNodeTop = new osg::ClipNode;
    _clipNodeTop->addClipPlane( _clipPlaneTop.get() );
    _clipNodeTop->addChild( _memberGroup.get() );
    _clipPlaneBottom = new osg::ClipPlane(1, 0.0, 0.0, -1.0, 0.0);
    _clipNodeBottom = new osg::ClipNode;
    _clipNodeBottom->addClipPlane( _clipPlaneBottom.get() );
    _clipReverseTx = new osg::MatrixTransform;
    _clipReverseTx->preMult( osg::Matrix::scale(1.0f, 1.0f, -1.0f) );
    osg::StateSet* ssReverse = _clipReverseTx->getOrCreateStateSet();
    ssReverse->setAttributeAndModes( new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON );
    _clipReverseTx->addChild( _memberGroup.get() );
    _clipNodeBottom->addChild( _clipReverseTx.get() );

    _ttx = new MoveWithEyePointTransform;
    _ttx->addChild( _skyTx.get() );

    addChild( _ttx.get() );

    setUpdateCallback( new UpdateCallback(*this) );
}

void EphemerisModel::setEphemerisData( EphemerisData *data )
{
    _ephemerisData = data;
}

EphemerisData *EphemerisModel::getEphemerisData()
{
    return _ephemerisData;
}


void EphemerisModel::setMembers( unsigned int members )
{
    _members = members & ALL_MEMBERS;
}

unsigned int EphemerisModel::getMembers() const
{
    return _members;
}

void EphemerisModel::setSkyDomeRadius( double radius )
{
    _scale = radius/SkyDome::getMeanDistanceToMoon();
    _skyTx->setMatrix( osg::Matrix::scale( _scale, _scale, _scale ));

}

double EphemerisModel::getSkyDomeRadius()  const
{
    return SkyDome::getMeanDistanceToMoon() * _scale;
}

void EphemerisModel::setSkyDomeCenter( osg::Vec3 center )
{
    _center = center;
    _ttx->setCenter(_center);
}

osg::Vec3 EphemerisModel::getSkyDomeCenter()
{
    return _center;
}

void EphemerisModel::setSunFudgeScale( double scale )
{
    _sunFudgeScale = scale;
    if( _skyDome.valid() )
        _skyDome->setSunFudgeScale( _sunFudgeScale );
}

void EphemerisModel::setMoonFudgeScale( double scale )
{
    _moonFudgeScale = scale;
}

bool EphemerisModel::_init()
{
    if( _inited ) return  _inited;
    // Sun Light Source
    if( _members & SUN_LIGHT_SOURCE )
    {
        _createSunLightSource();
        if (_sunLightSource.valid())
        {
            _sunLightSource->getLight()->setLightNum(_sunLightNum);
            addChild( _sunLightSource.get() );
        }
    }

    // Moon Light Source
    if( _members & MOON_LIGHT_SOURCE )
    {
        _createMoonLightSource();
        if (_moonLightSource.valid())
        {
            _moonLightSource->getLight()->setLightNum(_moonLightNum);
            addChild( _moonLightSource.get() );
        }
    }

    // Sky
    _skyTx->setMatrix( osg::Matrix::scale( _scale, _scale, _scale ) *
                       osg::Matrix::translate( _center ));

    // Sky Dome
    if( _members & SKY_DOME )
    {
        _createSkyDome();
        if (_skyDome.valid())
        {
            _skyDome->setSunFudgeScale( _sunFudgeScale );
            // Add this to _skyTx instead of _memberGroup.  _memberGroup is affected
            // by clipping planes, which cause an ugly seam in the sky dome.  But since
            // reflections are already handled internally by SkyDome, clipping planes
            // are not required.  (Terry cannot figure out why ugly seam appears in SkyDome
            // but not in other geometry.)
            _skyTx->addChild(_skyDome.get());
        }
    }

    // Ground Plane
    if( _members & GROUND_PLANE )
    {
        _createGroundPlane();
        if (_groundPlane.valid())
        {
            _memberGroup->addChild( _groundPlane.get());
        }
    }

    // Moon
    if( _members & MOON )
    {
        _createMoon();
        if (_moon.valid())
        {
            _moonTx = new osg::MatrixTransform;
            _moonTx->addChild( _moon.get() );
            _memberGroup->addChild( _moonTx.get() );
        }
    }

    // Planets
    if( _members & PLANETS )
    {
        _createPlanets();
        if (_planets.valid())
        {
            _memberGroup->addChild( _planets.get() );
        }
    }

    // StarField
    if( _members & STAR_FIELD )
    {
        _createStarField();
        if (_starField.valid())
        {
            _starFieldTx = new osg::MatrixTransform;
            _starFieldTx->addChild( _starField.get() );
            _memberGroup->addChild( _starFieldTx.get() );
        }
    }

    _makeConnections();

    return _inited = true;
}

void EphemerisModel::_createSunLightSource()
{
    _sunLightSource = new osg::LightSource;
}

void EphemerisModel::_createMoonLightSource()
{
    _moonLightSource = new osg::LightSource;
}

void EphemerisModel::_createSkyDome()
{
    _skyDome = new SkyDome( _skyDomeUseSouthernHemisphere, _skyDomeMirrorSouthernHemisphere );
}

void EphemerisModel::_createGroundPlane()
{
    _groundPlane = new GroundPlane(SkyDome::getMeanDistanceToMoon());
}

void EphemerisModel::_createMoon()
{
    _moon = new MoonModel;
}

void EphemerisModel::_createPlanets()
{
    _planets = new Planets;
}

void EphemerisModel::_createStarField()
{
    _starField  = new StarField;
}


void EphemerisModel::_makeConnections()
{
    if(_members & GROUND_PLANE)
    {
        _skyTx->removeChild( _clipNodeTop.get() );
        _skyTx->removeChild( _clipNodeBottom.get() );
        _skyTx->addChild( _memberGroup.get() );
    }
    else
    {
        _skyTx->removeChild( _memberGroup.get() );
        _skyTx->addChild( _clipNodeBottom.get() );
        _skyTx->addChild( _clipNodeTop.get() );
    }
}

void EphemerisModel::setSunLightNum( unsigned int lightNum )
{
    if( _sunLightSource.valid() )
    {
        _sunLightNum = lightNum;
        _sunLightSource->getLight()->setLightNum(_sunLightNum);
    }
}

unsigned int EphemerisModel::getSunLightNum() const
{
    return _sunLightNum;
}

void EphemerisModel::setMoonLightNum( unsigned int lightNum )
{
    if( _moonLightSource.valid() )
    {
        _moonLightNum = lightNum;
        _moonLightSource->getLight()->setLightNum(_moonLightNum);
    }
}

unsigned int EphemerisModel::getMoonLightNum() const
{
    return _moonLightNum;
}

void EphemerisModel::setUseEphemerisEngine(bool flag )
{
    if( flag == true )
    {
        if( !_ephemerisEngine.valid() )
            _ephemerisEngine = new EphemerisEngine(_ephemerisData);
    }
    else
        _ephemerisEngine = 0L;
}

bool EphemerisModel::getUseEphemerisEngine() const
{
    return _ephemerisEngine.valid();
}


void EphemerisModel::setMoveWithEyePoint(bool flag)
{
    if( flag )
        _ttx->enable();
    else
        _ttx->disable();
}

bool EphemerisModel::getMoveWithEyePoint() const
{
    return _ttx->isEnabled();
}

void EphemerisModel::setLatitude( double latitude)
{
    if( _ephemerisEngine.valid() )
        _ephemerisEngine->setLatitude( latitude );
}

double EphemerisModel::getLatitude() const
{
    if( _ephemerisData != 0L )
        return _ephemerisData->latitude;

    return 0.0;
}

double EphemerisModel::getLongitude() const
{
    if( _ephemerisData != 0L )
        return _ephemerisData->longitude;
    return 0.0;
}

GroundPlane* EphemerisModel::getGroundPlane()
{
    if( _groundPlane.valid() )
    {
        return _groundPlane.get();
    }
    return 0;
}

void EphemerisModel::setLongitude( double longitude)
{
    if( _ephemerisEngine.valid() )
        _ephemerisEngine->setLongitude( longitude );
}

float EphemerisModel::getTurbidity() const
{
    if( _ephemerisData != 0L )
        return _ephemerisData->turbidity;
    return 0.0;
}

void EphemerisModel::setTurbidity( float turbidity )
{
    if( _ephemerisData != 0L )
        _ephemerisData->turbidity = turbidity;
}

void EphemerisModel::setLatitudeLongitude( double latitude, double longitude )
{
    if( _ephemerisEngine.valid() )
        _ephemerisEngine->setLatitudeLongitude( latitude, longitude );
}

void EphemerisModel::getLatitudeLongitude( double &latitude, double &longitude ) const
{
    if( _ephemerisData != 0L )
    {
        latitude  = _ephemerisData->latitude;
        longitude = _ephemerisData->longitude;
    }
}

void EphemerisModel::setLatitudeLongitudeAltitude( double latitude, double longitude, double altitude )
{
    if( _ephemerisEngine.valid() )
        _ephemerisEngine->setLatitudeLongitudeAltitude( latitude, longitude, altitude );
}

void EphemerisModel::getLatitudeLongitudeAltitude( double &latitude, double &longitude, double &altitude ) const
{
    if( _ephemerisData != 0L )
    {
        latitude  = _ephemerisData->latitude;
        longitude = _ephemerisData->longitude;
        altitude  = _ephemerisData->altitude;
    }
}

void EphemerisModel::setDateTime( const DateTime &dateTime )
{
    if( _ephemerisEngine.valid() )
        _ephemerisEngine->setDateTime( dateTime );
}

DateTime EphemerisModel::getDateTime() const
{
    DateTime dt;
    if( _ephemerisData != 0L )
        dt =  _ephemerisData->dateTime;

    return dt;
}

void EphemerisModel::setAutoDateTime(bool flag)
{
    _autoDateTime = flag;
}

bool EphemerisModel::getAutoDateTime() const
{
    return _autoDateTime;
}


void EphemerisModel::update()
{
    //if( !_inited )
    //    _init();

    if( _ephemerisUpdateCallback.valid() )
        (*_ephemerisUpdateCallback.get())(_ephemerisData);

    if( _ephemerisEngine.valid() )
        _ephemerisEngine->update( _ephemerisData, _autoDateTime);

    _updateSun();
    if( _moon.valid() )
        _updateMoon();
    if( _planets.valid() )
        _planets->update( _ephemerisData );
    if( _starField.valid() )
        _updateStars();
}

void EphemerisModel::_updateStars()
{
    if( _starFieldTx.valid() )
    {
        _starFieldTx->setMatrix(
        osg::Matrix::rotate( -(-1.0 + (_ephemerisData->localSiderealTime/12.0)) * osg::PI,     osg::Vec3(0, 0, 1)) *
        osg::Matrix::rotate( -osg::DegreesToRadians((90.0 - _ephemerisData->latitude)), osg::Vec3(1, 0, 0))
        );
    }
}

void EphemerisModel::_updateMoon()
{
    osg::Matrix mat =
        osg::Matrix::scale( _moonFudgeScale, _moonFudgeScale, _moonFudgeScale ) *
        osg::Matrix::translate( 0.0, SkyDome::getMeanDistanceToMoon() + MoonModel::getMoonRadius() * 1.1 * _moonFudgeScale, 0.0 ) *
        osg::Matrix::rotate( _ephemerisData->data[CelestialBodyNames::Moon].alt, 1, 0, 0 ) *
        osg::Matrix::rotate( _ephemerisData->data[CelestialBodyNames::Moon].azimuth, 0, 0, -1 );

    if( _moonTx.valid() )
        _moonTx->setMatrix( mat );

    // Phase of the moon
    osg::Matrix mati;
    mati.invert(mat);
    osg::Vec3d moonVec(mati(3,0), mati(3,1), mati(3,2));
    osg::Vec3d mv = _sunVec * mati;
    mv -= moonVec;
    mv.normalize();
    _moon->setSunPosition( mv );

    // moon light
    if( _moonLightSource.valid() )
    {
        osg::Vec3d sunVecNormalized = _sunVec;
        sunVecNormalized.normalize();
        osg::Vec3d vecToMoon(mat(3,0), mat(3,1), mat(3,2));
        vecToMoon.normalize();
          // moon brightness in range {0,1}
        // increasing sunlight makes moonlight disappear
        double sunfactor = -2.0 * osg::RadiansToDegrees(_ephemerisData->data[CelestialBodyNames::Sun].alt);
        sunfactor = sunfactor < 0.0 ? 0.0 : sunfactor;
        sunfactor = sunfactor > 1.0 ? 1.0 : sunfactor;
        const double moonBrightness = ((sunVecNormalized * vecToMoon) * -0.5 + 0.5) * sunfactor;

        double visible(vecToMoon[2] / 0.05);
        visible = visible < 0.0 ? 0.0 : visible;
        visible = visible > 1.0 ? 1.0 : visible;
        // 0.5 is arbitrary (moon just shouldn't be as bright as the sun
        const double moonlight(moonBrightness * visible * 0.5);

        // Make the final values a little bit blue, and always add in a tiny bit of ambient starlight
        osg::Vec4 ambient(moonlight * 0.32 + 0.05, moonlight * 0.32 + 0.05, moonlight * 0.4 + 0.05, 1);
        osg::Vec4 diffuse(moonlight * 0.8, moonlight * 0.8, moonlight, 1);

        osg::Light &light = *(_moonLightSource->getLight());

        light.setAmbient( ambient );
        light.setDiffuse( diffuse );
        light.setSpecular( diffuse );
        osg::Vec4 position(vecToMoon[0], vecToMoon[1], vecToMoon[2], 0.0);
        light.setPosition( position );
    }
}

#if 0
static double findIncidentLength( double alpha )
{
    const double EARTH_RADIUS     = 6378140;
    const double ATMOSPHERE_DEPTH =   30000;
    const double E = 0.00001;

    // sides
    const double a = EARTH_RADIUS + ATMOSPHERE_DEPTH;
    const double b = EARTH_RADIUS;
    double c = 0.0;

    // Angles
    double A = alpha + osg::PI*0.5;
    double B = asin( b * sin(A)/a );
    double C = osg::PI - (A + B);

    if( fabs(sin(A)) < E)
        c = ATMOSPHERE_DEPTH;
    else
        c = a * sin(C)/sin(A);

    return c;
}
#endif


void EphemerisModel::_updateSun()
{
    double sunAz  = osg::RadiansToDegrees(_ephemerisData->data[CelestialBodyNames::Sun].azimuth);
    double sunAlt = osg::RadiansToDegrees(_ephemerisData->data[CelestialBodyNames::Sun].alt);

    if( _skyDome.valid() )
    {
        _skyDome->setSunPos( sunAz, sunAlt );
        _skyDome->setTurbidity( _ephemerisData->turbidity );
    }

    if( _starField.valid() )
        _starField->setSunAltitude(  sunAlt );

    // Update the sun's position
    osg::Vec4 position = osg::Vec4(
                            sin(osg::DegreesToRadians(sunAz))*cos(osg::DegreesToRadians(sunAlt)),
                            cos(osg::DegreesToRadians(sunAz))*cos(osg::DegreesToRadians(sunAlt)),
                            sin(osg::DegreesToRadians(sunAlt)),
                           0.0f );

    osg::Vec3d n = osg::Vec3(position[0], position[1], position[2] );
    n.normalize();
    // Mean distance to sun  1.496x10^8 km
    // Use 1/2 distance.  In reality, light "goes around corners".  Using half the distance
    // allows us to mimic the light surrounding the moon sphere a bit more.
    n *= (1.496 * 100000000000.0) * 0.5;
    _sunVec = n;

    // Set atmosphere lighting
    // Note - This is similar to the computing the sky color... Perhaps the two
    // should be combined.
    if( _sunLightSource.valid() )
    {
        double red   = sunAlt * 0.5;
        double green = sunAlt * 0.25;
        double blue  = sunAlt * 0.125;

        red   = red < 0.0 ? 0.0 : red;
        red   = red > 1.0 ? 1.0 : red;
        green = green < 0.0 ? 0.0 : green;
        green = green > 1.0 ? 1.0 : green;
        blue  = blue < 0.0 ? 0.0 : blue;
        blue  = blue > 1.0 ? 1.0 : blue;

        osg::Vec4 diffuse(red, green, blue, 1);

        red   = (sunAlt + 10.0) * 0.04;
        green = (sunAlt + 10.0) * 0.02;
        blue  = (sunAlt + 10.0) * 0.01;

        red   = red < 0.0 ? 0.0 : red;
        red   = red > 0.2 ? 0.2 : red;

        green = green < 0.0 ? 0.0 : green;
        green = green > 0.2 ? 0.2 : green;
        blue  = blue < 0.0 ? 0.0 : blue;
        blue  = blue > 0.2 ? 0.2 : blue;

        osg::Vec4 ambient(red, green, blue, 1);

        osg::Light &light = *(_sunLightSource->getLight());
        light.setAmbient( ambient );
        light.setDiffuse( diffuse );
        light.setSpecular( diffuse );
        light.setPosition( position );

        /*osg::Light &light = *(_sunLightSource->getLight());

        double max  = findIncidentLength( 0.0 );
        double silf = findIncidentLength( osg::DegreesToRadians(sunAlt + 4) )/max;

        double source = 0.8;
        double rsource = 0.8;

        //if( silf > 0.35 )
        {
            double f = silf + 0.35;
            source /= f;
            rsource /= f*0.35;
        }

        if( silf > 1.0 )
            silf = 1.0;

        if( rsource > 1.0 )
            rsource = 1.0;

        if( source > 1.0 )
            source = 1.0;

        double a = rsource * 1.0 - silf;
        if( a < 0.0 ) a = 0.0;

        ambient.set( a, a, a, 1.0 );
        double dr = rsource * silf;
        double dg = source * silf;
        double db = source * (1.0 - silf) ;

        diffuse.set( dr, dg, db, 1.0 );
        specular.set( 1.0, 1.0, 1.0, 1.0 );

        light.setAmbient( ambient );
        light.setDiffuse( diffuse );
        light.setSpecular( specular );
        light.setPosition(position);*/
    }
}



void EphemerisModel::traverse(osg::NodeVisitor&nv)
{
    if( !_inited ) _init();

    // Abort traversals by the Optimizer.  The optimizer will try
    // and combine stateSets we need to keep separate.
    if (dynamic_cast<osgUtil::BaseOptimizerVisitor*>(&nv))
        return;

    osg::Group::traverse( nv );
}

void EphemerisModel::setEphemerisUpdateCallback( EphemerisUpdateCallback *updateCallback )
{
    _ephemerisUpdateCallback = updateCallback;
}

const EphemerisUpdateCallback *EphemerisModel::getEphemerisUpdateCallback()  const
{
    return _ephemerisUpdateCallback.get();
}

