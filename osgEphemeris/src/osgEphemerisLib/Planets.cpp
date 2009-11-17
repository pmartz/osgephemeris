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

#include <osgDB/ReadFile>

#include <osgEphemeris/SkyDome.h>
#include <osgEphemeris/Sphere.h>
#include <osgEphemeris/Planets.h>
#include <osgText/Text>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Point>
#include <osg/MatrixTransform>
#include <osg/Texture2D>

/*
   * From http://www.windows.ucar.edu/tour/link=/mercury/statistics.html
  Stats:

    Planet       Diameter    Radius      Distance
    ------       --------    ------      --------
    Mercury       4,878 km   2439 Km    46.0e6 km  
    Venus        12,104 km   6052        40.0e6 km 
    Mars          6,785 km   3393        35.0e6 km 
    Jupiter     142,800 km  71400       588.0e6 km
    Saturn      119,871 km  59935      1200.0e6 km
    Uranus       51,488 km  25744      2570.0e6 km 
    Neptune      49,493 km  24747      4300.0e6 km
*/

using namespace osgEphemeris;

static inline osg::ref_ptr<osgText::Text> makeText( const std::string &textString, osg::Vec3 pos )
{
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont( "fonts/arial.ttf" );
    text->setFontResolution( 252, 252 );
    text->setAlignment( osgText::Text::LEFT_BOTTOM );
    text->setAutoRotateToScreen( true );
    text->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
    text->setColor( osg::Vec4( 1, 0, 0, 1 ));
    text->setText( textString );
    text->setCharacterSize( 45.0 );
    text->setPosition( pos );
    return text;
}

static inline osg::Group *makeAPlanet( const std::string &name, const std::string imageName="", double radius=0.0)
{
    osg::Group * group = new osg::Group;
    osg::ref_ptr<Sphere> sphere;

   if( radius > 0.0 )
      sphere  = new Sphere(radius, Sphere::TessLow);
   else
      sphere  = new Sphere;

    group->addChild( sphere.get() );

    osg::Geode *geode =new osg::Geode;
    geode->addDrawable( makeText( name, osg::Vec3(0,0,0)).get());

    if( !imageName.empty() )
    {
        osg::ref_ptr<osg::Image> image = osgDB::readImageFile( imageName );
        if( image.valid() )
        {
            osg::ref_ptr<osg::Texture2D> tex = new osg::Texture2D;
            tex->setImage( image.get() ); 
            sphere->getOrCreateStateSet()->setTextureAttributeAndModes( 0, tex.get() );
        }
    }
    group->addChild( geode );

    //group->addChild( new Sphere );

    return group;
}

Planets::Planets()
{
    double radius;

    radius = (SkyDome::getMeanDistanceToMoon()/46e9) * (2439.0e3);
    _mercuryTx  = new osg::MatrixTransform;
    _mercuryTx->addChild( makeAPlanet( "Mercury", "SolarSystem/mercury256128.jpg", radius ));
    addChild( _mercuryTx.get() );
    
    radius = (SkyDome::getMeanDistanceToMoon()/35e9) * (6042.0e3);
    _venusTx    = new osg::MatrixTransform;
    _venusTx->addChild( makeAPlanet( "Venus", "SolarSystem/venus256128.jpg", radius ));
    addChild( _venusTx.get() );

    radius = (SkyDome::getMeanDistanceToMoon()/46e9) * (3393.0e3);
    _marsTx     = new osg::MatrixTransform;
    _marsTx->addChild( makeAPlanet( "Mars", "SolarSystem/mars256128.jpg", radius ));
    addChild( _marsTx.get() );

    radius = (SkyDome::getMeanDistanceToMoon()/588e9) * (71400.0e3);
    _jupiterTx  = new osg::MatrixTransform;
    _jupiterTx->addChild( makeAPlanet( "Jupiter", "SolarSystem/jupiter256128.jpg", radius ));
    addChild( _jupiterTx.get() );
    
    radius = (SkyDome::getMeanDistanceToMoon()/1200e9) * (59935.0e3);
    _saturnTx   = new osg::MatrixTransform;
    _saturnTx->addChild( makeAPlanet( "Saturn", "SolarSystem/saturn256128.jpg", radius ));
    addChild( _saturnTx.get() );
    
    radius = (SkyDome::getMeanDistanceToMoon()/2570e9) * (51488.0e3);
    _uranusTx   = new osg::MatrixTransform;
    _uranusTx->addChild( makeAPlanet( "Uranus", "SolarSystem/pluto256128.jpg", radius ));
    addChild( _uranusTx.get() );
    
    radius = (SkyDome::getMeanDistanceToMoon()/4300e9) * (24747.0e3);
    _neptuneTx  = new osg::MatrixTransform;
    _neptuneTx->addChild( makeAPlanet( "Neptune", "SolarSystem/neptune256128.jpg", radius ));
    addChild( _neptuneTx.get() );


    setCullingActive(false);
}

static inline osg::Matrix makeMatrix( 
        EphemerisData *ephemData, 
        CelestialBodyNames::CelestialBodyName name)
{
    const  double distance = SkyDome::getMeanDistanceToMoon() * 1.05;
    return osg::Matrix::translate( 0.0, distance, 0.0 ) *
           osg::Matrix::rotate( ephemData->data[name].alt, 1, 0, 0 ) *
           osg::Matrix::rotate( ephemData->data[name].azimuth, 0, 0, -1 );
}

void Planets::update( EphemerisData *ephemData )
{
    _mercuryTx->setMatrix(  makeMatrix(ephemData, CelestialBodyNames::Mercury)); 
    _venusTx->setMatrix(    makeMatrix(ephemData, CelestialBodyNames::Venus)); 
    _marsTx->setMatrix(     makeMatrix(ephemData, CelestialBodyNames::Mars)); 
    _jupiterTx->setMatrix(  makeMatrix(ephemData, CelestialBodyNames::Jupiter)); 
    _saturnTx->setMatrix(   makeMatrix(ephemData, CelestialBodyNames::Saturn)); 
    _uranusTx->setMatrix(   makeMatrix(ephemData, CelestialBodyNames::Uranus)); 
    _neptuneTx->setMatrix(  makeMatrix(ephemData, CelestialBodyNames::Neptune)); 
}


