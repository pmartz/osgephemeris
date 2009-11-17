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

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/LineWidth>
#include <osgText/Text>
#include <osgUtil/CullVisitor>
#include <osg/ClipPlane>

#include "Compass.h"

//const osg::Vec4 Compass::color = osg::Vec4( 0, 0.6, 0, 1.0 );

osg::Vec4 color( 0, 0.6, 0, 1.0 );

Compass::Compass():
    _initialized(false)
{
    _init();
}

Compass::Compass(osg::Viewport *vp):
    _initialized(false)
{
    _viewport = vp;
    _init();
}

Compass::Compass(const Compass& copy, const osg::CopyOp& copyop)
{
    _init();
}

void  Compass::setViewport( osg::Viewport *vp )
{
    _viewport = vp;
    getStateSet()->setAttribute( _viewport.get() ); 
}

const osg::Viewport  *Compass::getViewport()  const
{
    return _viewport.get();
}

void Compass::_init()
{
    osg::ref_ptr<osg::StateSet> stateSet = new osg::StateSet;

    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
    stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
    stateSet->setTextureMode( 0, GL_TEXTURE_2D, osg::StateAttribute::OFF );
    stateSet->setRenderBinDetails( 110, "RenderBin" );

    if( _viewport.valid() )
        stateSet->setAttribute( _viewport.get() ); 

    double ar = 120.0/640.0;
    setMatrix( osg::Matrix::frustum( -0.35, 0.35, -0.35 * ar, 0.35 * ar, 1.0, 10000.0 ));
    //setMatrix( osg::Matrix::ortho2D( -1, 1, 0, 2 * ar ));

    _clipPlane =  new osg::ClipPlane( 0, osg::Plane( 0.0, -1.0, 0.0, 0.0 ));
    stateSet->setAttributeAndModes( _clipPlane.get() );
    stateSet->setAttributeAndModes( new osg::LineWidth( 4.0 ));

    setStateSet( stateSet.get());

    setCullingActive( false );

    _tx = new osg::MatrixTransform;
    _tx->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    _tx->addChild( _makeGeode().get() );
    addChild(_tx.get());

    _ltx = new osg::MatrixTransform;
    _ltx->addChild(_lineGeode().get());
    _tx->addChild( _ltx.get() );

    _initialized = true;
}

osg::ref_ptr<osg::Geode> Compass::_lineGeode()
{
    osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
    coords->push_back( osg::Vec3(-0.03, -1.0, -0.07) );
    coords->push_back( osg::Vec3( 0.0,  -1.0, -0.01) );
    coords->push_back( osg::Vec3( 0.0,  -1.0, -0.01) );
    coords->push_back( osg::Vec3( 0.03, -1.0, -0.07) );

    /*
    coords->push_back( osg::Vec3(-0.03, -1.0,  0.15) );
    coords->push_back( osg::Vec3( 0.0,  -1.0,  0.11) );
    coords->push_back( osg::Vec3( 0.0,  -1.0,  0.11) );
    coords->push_back( osg::Vec3( 0.03, -1.0,  0.15) );
    */

    osg::ref_ptr<osg::Vec4Array> colors   = new osg::Vec4Array;
    colors->push_back( color );

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray(coords.get());
    geometry->setColorArray(colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, coords->size()));

    osg::ref_ptr<osg::Geode> geode = new  osg::Geode;
    geode->addDrawable( geometry.get() );

    return geode;
}

osg::ref_ptr<osg::Geode> Compass::_makeGeode()
{
    osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors   = new osg::Vec4Array;

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    for( int i = 0; i < 360; i += 5 )
    {
        double a = osg::DegreesToRadians( double(i-180) );

        coords->push_back( osg::Vec3( sin(-a), cos(-a), 0.0 ));
        if( !(i%10 ) )
            coords->push_back( osg::Vec3( sin(-a), cos(-a), 0.06 ));
        else
            coords->push_back( osg::Vec3( sin(-a), cos(-a), 0.02 ));

        if( !(i % 30 ))
        {
            osg::ref_ptr<osgText::Text> text = new osgText::Text;
            text->setFont( "fonts/arial.ttf" );
            text->setAlignment( osgText::Text::CENTER_CENTER );
            text->setFontResolution(100,100);
            text->setCharacterSize( 0.1 );
            text->setColor( color );

            osg::Matrix m = osg::Matrix::rotate( osg::PI*0.5, 1, 0, 0 ) *
                            osg::Matrix::rotate( osg::PI + a, 0, 0, 1 );
            osg::Quat q;
            q.set( m );
            text->setRotation( q );

            char buff[8];

            (i == 0 )   ?  sprintf( buff, "N" ):
            (i == 90 )  ?  sprintf( buff, "W" ):
            (i == 180 ) ?  sprintf( buff, "S" ):
            (i == 270 ) ?  sprintf( buff, "E" ): sprintf( buff, "%2d", (((360-i)%360)/10));

            text->setText( buff );
            text->setPosition( osg::Vec3( sin(-a),  cos(-a), 0.1 ));

            geode->addDrawable( text.get() );
        }
    }

    colors->push_back( color );

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray(coords.get());
    geometry->setColorArray(colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, coords->size()));

    //geometry->getOrCreateStateSet()->setAttributeAndModes( new osg::LineWidth( 4.0 ));
    geode->addDrawable( geometry.get() );

    return geode;
}


void Compass::traverse(osg::NodeVisitor&nv)
{
    if( !_initialized )
        _init();

    if( nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
    {
        osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
        osg::Matrix ivm = cv->getState()->getInitialViewMatrix();
        ivm(3,0) = ivm(3,1) = ivm(3,2) = 0.0;
        osg::Vec3 v = osg::Vec3(0,1,0) * ivm * osg::Matrix::rotate( osg::PI*0.5, 1, 0, 0 );
        double a = atan2( v[0], v[1] );
        _tx->setMatrix( 
                osg::Matrix::rotate( -a, 0, 0, 1 ) * 
                osg::Matrix::translate( 0.0, 3.0, -0.05 ) *
                osg::Matrix::rotate( -osg::PI*0.5, 1, 0, 0 ) );

        osg::Vec4d cp = osg::Vec4d(0.0, -1.0, 0.0, -0.75 ) * osg::Matrix::rotate( a, 0, 0, 1 );
        _clipPlane->setClipPlane( cp );

        _ltx->setMatrix( osg::Matrix::rotate(  a, 0, 0, 1 ) );


    }
    osg::Projection::traverse( nv );
}

