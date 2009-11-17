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

#include <osg/Geometry>
#include <osgEphemeris/Sphere.h>

using namespace osgEphemeris;

const double Sphere::_defaultRadius =     3476000.0 * 0.5; // diameter of moon * 0.5 = radius of moon

Sphere::Sphere( double radius,
        Sphere::TesselationResolution tr,
        Sphere::Orientation orientation,
        Sphere::Hemisphere whichHemisphere,
        bool stc
      )
{
    _skyTexCoords = stc;

    unsigned int nsectors = 4;
    double latStep  = 
        ( tr == TessHigh ) ?  1.875 :
        ( tr == TessLow ) ?   15 : 7.5;


    osg::ref_ptr<osg::Vec3Array> v = new osg::Vec3Array;
    int R = 0;
    for( unsigned int sector = 0; sector < nsectors; sector++ )
    {
        double off = double(sector) * 360.0/(double)(nsectors);
        int div = 1;

        v->push_back( osg::Vec3(0,0,radius));
        for( double lat = latStep; lat <= 91.0; lat += latStep )
        {
            double dd = (360.0/double(nsectors))/(double)(div);
            for( int i = 0; i <= div; i++ ) 
            {
                double lon = off + double(i) * dd;

                double x = radius * cos(osg::DegreesToRadians(lon)) *  sin(osg::DegreesToRadians(lat));
                double y = radius * sin(osg::DegreesToRadians(lon)) *  sin(osg::DegreesToRadians(lat));
                double z = radius * cos(osg::DegreesToRadians(lat));

                v->push_back( osg::Vec3(x,y,z));
            }
            div++;
            if( sector == 0 )
                R++;
        }
    }

    for( int hemisphere = 0; hemisphere < 2; hemisphere++ )
    {
        if( !((1<<hemisphere) & whichHemisphere) )
            continue;

        osg::Geode *geode = new osg::Geode;

        int iii = 0;
        osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
        osg::ref_ptr<osg::Vec2Array> tcoords = new osg::Vec2Array;

        for( unsigned int sector = 0; sector < nsectors; sector++ )
        {
            int len = 3;
            int ii[] = { 1, 0 };
            int toggle = 0;
    
            for( int j = 0; j < R; j++ )
            {
                osg::ref_ptr<osg::Vec3Array> tarray = new osg::Vec3Array;

                for( int i = 0; i < len; i++ )
                {
                    int index = iii + ii[toggle];
                    tarray->push_back( (*v)[index] );
                    ii[toggle]++;
                    toggle = 1 - toggle;
                }

                if( hemisphere == 0 )
                {
                    if( orientation == InnerOrientation )
                    {
                        for( unsigned int n = 0; n < tarray->size(); n++ )
                        {
                            osg::Vec3 v = (*tarray)[n];
                            coords->push_back( v );
                            v.normalize();
                            v *= -1;
                            normals->push_back( v );
                            tcoords->push_back( makeTexCoord(v, sector) );
                        }
                    }
                    else
                    {
                        for( int n = tarray->size() - 1; n >= 0 ; n-- )
                        {
                            osg::Vec3 v = (*tarray)[n];
                            coords->push_back( v );
                            v.normalize();
                            normals->push_back( v );
                            tcoords->push_back( makeTexCoord(v, sector) );
                        }
                    }
                }
                else
                {
                    if( orientation == InnerOrientation )
                    {
                        for( int n = tarray->size() - 1; n >= 0 ; n-- )
                        {
                            osg::Vec3 v = (*tarray)[n];
                            v[2] *= -1.0;
                            coords->push_back( v );
                            v.normalize();
                            v *= -1;
                            normals->push_back( v );
                            tcoords->push_back( makeTexCoord(v, sector) );
                        }
                    }
                    else
                    {
                        for( unsigned int n = 0; n < tarray->size(); n++ )
                        {
                            osg::Vec3 v = (*tarray)[n];
                            v[2] *= -1.0;
                            coords->push_back( v );
                            v.normalize();
                            normals->push_back( v );
                            tcoords->push_back( makeTexCoord(v, sector) );
                        }
                    }
                }
    
                toggle = 1 - toggle;
                len += 2;
            }
            iii += ii[0];
    
        }

        int index = 0;
        for( unsigned int sector = 0; sector < nsectors; sector++ )
        {
            osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
            colors->push_back( osg::Vec4( 1.0, 1.0, 1.0,1.0 ));

            osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
            geometry->setVertexArray( coords.get() );
            geometry->setTexCoordArray( 0, tcoords.get() );
            geometry->setNormalArray( normals.get() );
            geometry->setNormalBinding( osg::Geometry::BIND_PER_VERTEX );
            geometry->setColorArray( colors.get() );
            geometry->setColorBinding( osg::Geometry::BIND_OVERALL );


            int len = 3;
            for( int i = 0; i < R; i++ )
            {
                geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, index, len ));
                index += len;
                len += 2;
            }

            geode->addDrawable( geometry.get() );
        }

        //geode->addDrawable( geometry.get() );

        if( hemisphere == 0 )
        {
            _northernHemisphere = geode;
            addChild( _northernHemisphere.get() );
        }
        else
        {
            _southernHemisphere = geode;
            addChild( _southernHemisphere.get() );
        }
    }
}

osg::Vec2 Sphere::makeTexCoord(osg::Vec3 &normal, unsigned int sector)
{
    double s = 0.0;
    {
        double a = atan2( normal[1], normal[0] );
        if( a < 0.0 )
            a += osg::PI*2;
        a /= (2*osg::PI);
        
        // Prevent wrapping of texcoord, which creates ugly seam
        if(sector == 2 && a > 0.999)
            a = 0.0;
        
        s = a;
    }

    double t;
    if( _skyTexCoords )
        t = (1.0 + ((asin(normal[2]))/(osg::PI_2)));
    else
        t = (0.5 + ((asin(normal[2]))/(osg::PI)));

    return osg::Vec2( s, t );
}

double Sphere::getDefaultRadius() 
{ 
    return _defaultRadius; 
}

SphereLOD::SphereLOD( double radius,
                      Sphere::Orientation orientation,
                        Sphere::Hemisphere hemisphere )
{
    addChild( new Sphere( radius, Sphere::TessHigh, orientation, hemisphere ) );
    addChild( new Sphere( radius, Sphere::TessNormal, orientation, hemisphere ) );
    addChild( new Sphere( radius, Sphere::TessLow, orientation, hemisphere ) );

    setRange( 0, 0.0, 10*radius );
    setRange( 1,  10*radius, 50*radius );
    setRange( 2,  50*radius, 1e12*radius );
}
