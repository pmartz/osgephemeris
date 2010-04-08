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

#include <iostream>
#include <stdio.h>

#include <osgDB/ReadFile>

#include <osgUtil/UpdateVisitor>

#include <osg/Version>
#include <osg/StateSet>
#include <osg/BlendFunc>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexEnv>

#include <osgEphemeris/SkyDome.h>


using namespace osgEphemeris;

const double SkyDome::_meanDistanceToMoon = 384403000.0;
#define SKY_DOME_X_SIZE 128
#define SKY_DOME_Y_SIZE 128

// Looks like somewhere along the way, OSG stopped calling Drawable Callbacks....
// So we'll do it ourselves.
class SkyDomeUpdateCallback: public osg::NodeCallback
{
    public:
        SkyDomeUpdateCallback( SkyDome *skyDome ):
            _skyDome(skyDome)
        {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            _callDrawableCallbacks( _skyDome->getNorthernHemisphere(), nv );
            _callDrawableCallbacks( _skyDome->getSouthernHemisphere(), nv );
            traverse(node,nv);
        }

    private:
        osg::ref_ptr<SkyDome> _skyDome;

        void _callDrawableCallbacks( osg::Geode *geode, osg::NodeVisitor *nv )
        {
            if( geode != NULL )
            {
                for( unsigned int i= 0; i < geode->getNumDrawables(); i++ )
                {
                    osg::Drawable *dbl = geode->getDrawable(i);
                    osg::Drawable::UpdateCallback *updateCallback = dbl->getUpdateCallback();
                    if( updateCallback != NULL )
                        updateCallback->update( nv, dbl );
                }
            }
        }

};

SkyDome::SkyDome( bool useBothHemispheres, bool mirrorInSouthernHemisphere ):
    Sphere( _meanDistanceToMoon,
            Sphere::TessHigh,
            Sphere::InnerOrientation,
            useBothHemispheres ? Sphere::BothHemispheres : Sphere::NorthernHemisphere,
            true ),
    _sunFudgeScale(1.0),
    _skyTextureUnit(0),
    _sunTextureUnit(1),
    _mirrorInSouthernHemisphere( mirrorInSouthernHemisphere ),
    _T(2.0f),
    _current_tex_row(0)
{
    unsigned int nsectors = _northernHemisphere->getNumDrawables();

    /*_northernHemisphere->*/setUpdateCallback( new SkyDomeUpdateCallback( this ) );

    // This sets up the "state culling" of the projected texture for the sun.  
    // We want to avoid projecting the sun texture on any other sphere sectors
    // than where the sun is positioned, thus avoiding the "back projected" 
    // anomaly that occurs from projected textures.
    for( unsigned int sector = 0; sector < nsectors; sector++ )
    {
        double div = 360.0/double(nsectors);
        double margin = 0.25 * div;

        double min = 90.0 - double(sector+1) * div;
        double max = 90.0 - double(sector+0) * div;

        min -= margin;
        max += margin;

        min = _range( min, 360.0 );
        max = _range( max, 360.0 );

        osg::Drawable *dbl = _northernHemisphere->getDrawable(sector);
        dbl->setStateSet( new osg::StateSet );

        dbl->setUpdateCallback( new SectorUpdateCallback( _sunAzimuth, min, max, _sunTextureUnit));

        if( _southernHemisphere.valid () )
        {
            dbl = _southernHemisphere->getDrawable(sector);
            dbl->setStateSet( new osg::StateSet );
            dbl->setUpdateCallback( new SectorUpdateCallback( _sunAzimuth, min, max, _sunTextureUnit));
        }
    }

    _updateDistributionCoefficients();

    _buildStateSet();

}

// Alpha in radians
double SkyDome::_findIncidenceLength( double alpha )
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


void SkyDome::setSunPos( double azimuth, double altitude )
{
    _sunAzimuth = azimuth;
    _sunAltitude = altitude;

    _dark_alt = -0.29;
    _day_exp = 0.2f;

    _light_due_to_alt = osg::DegreesToRadians(altitude);
    if(_light_due_to_alt < _dark_alt)
        _light_due_to_alt = 0.0f;
    else{
        // remap {_dark_alt, pi} to {-pi, pi}
        _light_due_to_alt = ((_light_due_to_alt - _dark_alt) / (osg::PI + _dark_alt))
            * osg::PI_2 - osg::PI;
        // clamp to -pi
        _light_due_to_alt = _light_due_to_alt < -osg::PI ? -osg::PI : _light_due_to_alt;
        // cosine of this new angle remapped to {0, 1}
        _light_due_to_alt = cosf(_light_due_to_alt) * 0.5f + 0.5f;
        // brighten it
        _light_due_to_alt = powf(_light_due_to_alt, _day_exp);
    }

    _computeSkyTexture();
}

void SkyDome::setTurbidity( float t )
{
    if(t < 1.0f)
        _T = 1.0f;
    else if(t > 60.0f)
        _T = 60.0f;
    else
        _T = t;

    _updateDistributionCoefficients();
}

void SkyDome::traverse(osg::NodeVisitor&nv)
{
    if (dynamic_cast<osgUtil::UpdateVisitor*>(&nv))
                   return;

    // The sun fills 0.53 degrees of visual angle.  The 1.45 multiplier is because the sun texture includes
    // a partially transparent halo around it so there isn't a hard edge.
    osg::Matrix  P;
    double hfov   = osg::DegreesToRadians(0.53 * 1.45 * _sunFudgeScale);
    double vfov   = osg::DegreesToRadians(0.53 * 1.45 * _sunFudgeScale);
    double left   = -tan(hfov/2.0);
    double right  =  tan(hfov/2.0);
    double bottom = -tan(vfov/2.0);
    double top    =  tan(vfov/2.0);
    P.makeFrustum( left, right, bottom, top, 1.0, 100.0 );

    osg::Matrix C(
        0.5, 0, 0, 0,
        0, 0.5, 0, 0,
        0, 0, 0.5, 0,
        0.5, 0.5, 0.5, 1
    );

    osg::Vec3 p = osg::Vec3( sin(osg::DegreesToRadians(_sunAzimuth))*cos(osg::DegreesToRadians(_sunAltitude)),
                             cos(osg::DegreesToRadians(_sunAzimuth))*cos(osg::DegreesToRadians(_sunAltitude)),
                             sin(osg::DegreesToRadians(_sunAltitude)) );

    //_aSunAzimuth = _range( osg::RadiansToDegrees(atan2( p[0], p[1] )), 360 );


    osg::Matrix M;
    M.makeRotate( osg::Vec3(0,0,1), p );
    osg::Matrix Mi;
    Mi.invert( M );

    osg::Matrix PC = Mi * P * C;
    _sunTexGenNorth->setPlanesFromMatrix( PC );

    if( _southernHemisphere.valid() )
    {
        if( _mirrorInSouthernHemisphere )
        {
            p[2] *= -1.0;
            M.makeRotate( osg::Vec3(0,0,-1), p );
            Mi.invert( M );
            PC = Mi * P * C ;
        }
        _sunTexGenSouth->setPlanesFromMatrix( PC );
    }

    osg::Group::traverse( nv );
}



void SkyDome::_buildStateSet()
{
    osg::StateSet *sset = getOrCreateStateSet();

    sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF    | osg::StateAttribute::PROTECTED );
    sset->setMode( GL_BLEND,    osg::StateAttribute::ON     | osg::StateAttribute::PROTECTED );
    {
        osg::ref_ptr<osg::BlendFunc> blendFunc = new osg::BlendFunc(GL_ONE,GL_ONE);
        sset->setAttributeAndModes( blendFunc.get() );
    }

    sset->setRenderBinDetails(-9,"RenderBin");

    ///////////////////////  Sky Texture unit 
    {
        /*unsigned char *data = new unsigned char[256*3];
        unsigned char *ptr = data;
        for( int i = 0; i < 256; i++ )
        {
            double f = double(255 - i)/255.0;
            *(ptr++) = (unsigned char)(255.0*(f*f*f*f));
            *(ptr++) = (unsigned char)(255.0*(f*f*f*f));
            *(ptr++) = 0xFF;
            // *(ptr++) = 0xFF;
        }

        osg::Image *image = new osg::Image;
        image->setImage( 256, 1, 1, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE );

        _skyTexture = new osg::Texture1D;
        _skyTexture->setWrap(osg::Texture1D::WRAP_S,osg::Texture1D::MIRROR);
        _skyTexture->setFilter(osg::Texture1D::MIN_FILTER,osg::Texture1D::LINEAR);
        _skyTexture->setImage( image );

        sset->setTextureAttributeAndModes( _skyTextureUnit, _skyTexture.get() );

        osg::TexGen *texGen = new osg::TexGen;
        texGen->setPlane( osg::TexGen::S, osg::Vec4( 0.0, 0.0, 1.0/_meanDistanceToMoon, 0.0 ));

        sset->setTextureMode( _skyTextureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON );
        sset->setTextureMode( _skyTextureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::OFF );
        sset->setTextureMode( _skyTextureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::OFF );
        sset->setTextureMode( _skyTextureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::OFF );
        sset->setTextureAttributeAndModes( _skyTextureUnit, texGen, osg::StateAttribute::ON );*/
		  
        unsigned char *data = new unsigned char[SKY_DOME_X_SIZE * SKY_DOME_Y_SIZE * 3];
        unsigned char *ptr = data;
        for( int i = 0; i < SKY_DOME_X_SIZE * SKY_DOME_Y_SIZE; i++ )
        {
            *(ptr++) = 0x30;
            *(ptr++) = 0x30;
            *(ptr++) = 0xFF;
        }
        
        osg::Image *skyImage = new osg::Image;
        skyImage->setImage( SKY_DOME_X_SIZE, SKY_DOME_Y_SIZE, 1, GL_RGB, GL_RGB,
            GL_UNSIGNED_BYTE, data, osg::Image::USE_NEW_DELETE );
        _skyTexture = new osg::Texture2D;
        _skyTexture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        _skyTexture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::MIRROR);
        _skyTexture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
        _skyTexture->setImage( skyImage );
        sset->setTextureAttributeAndModes( _skyTextureUnit, _skyTexture.get() );
        //sset->setTextureMode( _skyTextureUnit, GL_TEXTURE_2D, osg::StateAttribute::ON);
    }

    ///////////////////// Sun Texture unit 
    {
        //_sunImage = osgDB::readImageFile( "sun.rgba" );
        _sunImage = new osg::Image;
        _sunImage->setImage( 
            _sunImageWidth, 
            _sunImageHeight, 1,
            _sunImageInternalTextureFormat,
            _sunImagePixelFormat,
            GL_UNSIGNED_BYTE,
            _sunImageData,
            osg::Image::NO_DELETE );


        if( _sunImage.valid() )
        {
            _sunTexture  = new osg::Texture2D;
            _sunTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
            _sunTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
            _sunTexture->setImage( _sunImage.get() );

            sset->setTextureAttributeAndModes( _sunTextureUnit, _sunTexture.get() );
        }
        //else
        //    std::cerr << "Ephemeris::SkyDome() - Can't find sun texture \"sun.rgba\""<< std::endl;


        _sunTexGenNorth = new osg::TexGen;
        _sunTexGenNorth->setMode( osg::TexGen::OBJECT_LINEAR );
        osg::StateSet *ssn = _northernHemisphere->getOrCreateStateSet();
        ssn->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON );
        ssn->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON );
        ssn->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON );
        ssn->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON );
        ssn->setTextureAttributeAndModes( _sunTextureUnit, _sunTexGenNorth.get(), osg::StateAttribute::ON );

        if( _southernHemisphere.valid() )
        {
            _sunTexGenSouth = new osg::TexGen;
            _sunTexGenSouth->setMode( osg::TexGen::OBJECT_LINEAR );
            osg::StateSet *sss = _southernHemisphere->getOrCreateStateSet();
            sss->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON );
            sss->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON );
            sss->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON );
            sss->setTextureMode( _sunTextureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON );
            sss->setTextureAttributeAndModes( _sunTextureUnit, _sunTexGenSouth.get(), osg::StateAttribute::ON );
        }

        sset->setTextureAttributeAndModes( _sunTextureUnit, new osg::TexEnv( osg::TexEnv::DECAL ));
    }
}

SkyDome::SectorUpdateCallback::SectorUpdateCallback( double &sunAz, double min, double max, unsigned int sunTextureUnit ):
    _sunAz(sunAz),
    _sunTextureUnit(sunTextureUnit)
{
    _min = _range( min, 360 );
    _max = _range( max, 360 );
}

void SkyDome::SectorUpdateCallback::update(osg::NodeVisitor* nv, osg::Drawable* dbl) 
{
    osg::ref_ptr<osgUtil::UpdateVisitor> updateVisitor = dynamic_cast<osgUtil::UpdateVisitor*>(nv);

    if( !updateVisitor.valid()  )
    {
       return;
    }

    double sun = _range(_sunAz, 360);

    if( _withinDeg( sun, _min, _max ))
    {
        dbl->getStateSet()->setTextureMode( _sunTextureUnit, GL_TEXTURE_2D, osg::StateAttribute::ON);
    }
    else
    {
        dbl->getStateSet()->setTextureMode( _sunTextureUnit, GL_TEXTURE_2D, osg::StateAttribute::OFF);
        // For testing, cull geometry
        //return true;
    }

    return;
}

bool SkyDome::SectorUpdateCallback::_withinDeg( double x, double min, double max ) const
{
    if( min > max )
        return ((x >= min) && (x <= (max+360.0))) || ((x <= max) && (x >= (min-360.0)));

    return ((x >= min) && (x <= max));
}


void SkyDome::_updateDistributionCoefficients()
{
    // Preetham   
/*    _Ax = _T * -0.0193f + -0.2592f;
    _Bx = _T * -0.0665f +  0.0008f;
    _Cx = _T * -0.0004f +  0.2125f;
    _Dx = _T * -0.0641f + -0.8989f;
    _Ex = _T * -0.0033f +  0.0452f;

    _Ay = _T * -0.0167f + -0.2608f;
    _By = _T * -0.0950f +  0.0092f;
    _Cy = _T * -0.0079f +  0.2102f;
    _Dy = _T * -0.0441f + -1.6537f;
    _Ey = _T * -0.0109f +  0.0529f;

    _AY = _T *  0.1787f + -1.4630f;
    _BY = _T * -0.3554f +  0.4275f;
    _CY = _T * -0.0227f +  5.3251f;
    _DY = _T *  0.1206f + -2.5771f;
    _EY = _T * -0.0670f +  0.3703f;*/

    _Ar = _T * 0.00367f + 0.09f;
    _Br = _T * -0.08f + 6.0f;
    _Cr = 0.5f;
    _Dr = _T * -0.63333f + 40.0f;
    _Er = 0.19f;
    _horiz_atten_r = 0.0f;
    _solar_atten_r = 0.0f;

    _Ag = _T * 0.00367f + 0.11f;
    _Bg = _T * -0.08f + 6.0f;
    _Cg = 0.5f;
    _Dg = _T * -0.63333f + 40.0f;
    _Eg = 0.17f;
    _horiz_atten_g = powf(0.2f, 1.0f + 0.1f * _T);
    _solar_atten_g = powf(0.4f, 1.0f + 0.1f * _T);

    _Ab = 0.1f;
    _Bb = _T * -0.11f + 8.0f;
    _Cb = 0.5f;
    _Db = _T * -0.63333f + 40.0f;
    _Eb = 0.4f;
    _horiz_atten_b = powf(0.3f, 1.0f + 0.1f * _T);
    _solar_atten_b = powf(0.6f, 1.0f + 0.1f * _T);
}

void SkyDome::_updateZenithxyY()
{
    const float ths2(_theta_sun * _theta_sun);
    const float ths3(ths2 * _theta_sun);
    const float T2(_T * _T);

    _xz = T2 * (0.00166f * ths3 + -0.00375f * ths2 + 0.00209f * _theta_sun)
        + _T * (-0.02903f * ths3 + 0.06377f * ths2 + -0.03202f * _theta_sun + 0.00394f)
        + (0.11693f * ths3 + -0.21196f * ths2 + 0.06052f * _theta_sun + 0.25886f);
    
    _yz = T2 * (0.00275f * ths3 + -0.0061f * ths2 + 0.00317f * _theta_sun)
        + _T * (-0.04214f * ths3 + 0.0897f * ths2 + -0.04153f * _theta_sun + 0.00516f)
        + (0.15346f * ths3 + -0.26756f * ths2 + 0.0667f * _theta_sun + 0.26688f);

    const float chi( (0.44444444f - (_T * 0.00833333f))
        * (3.14159265f - (2.0f * _theta_sun)) );

    _Yz = (((4.0453f * _T) - 4.971f) * tanf(chi)) - (0.2155f * _T) + 2.4192f;
}

inline float SkyDome::_xDistributionFunction(const float /*theta*/, const float cos_theta,
                                            const float gamma, const float cos_gamma_sq)
{
    return _xz
        * ( (1.0f + (_Ax * expf(_Bx / cos_theta)))
        * (1.0f + (_Cx * expf(_Dx * gamma)) + (_Ex * cos_gamma_sq)) )
        / ( (1.0f + (_Ax * expf(_Bx)))
        * (1.0f + (_Cx * expf(_Dx * _theta_sun)) + (_Ex * _cos_theta_sun_squared)) );
}

inline float SkyDome::_yDistributionFunction(const float /*theta*/, const float cos_theta,
                                            const float gamma, const float cos_gamma_sq)
{
    return _yz
        * ( (1.0f + (_Ay * expf(_By / cos_theta)))
        * (1.0f + (_Cy * expf(_Dy * gamma)) + (_Ey * cos_gamma_sq)) )
        / ( (1.0f + (_Ay * expf(_By)))
        * (1.0f + (_Cy * expf(_Dy * _theta_sun)) + (_Ey * _cos_theta_sun_squared)) );
}

inline float SkyDome::_YDistributionFunction(const float /*theta*/, const float cos_theta,
                                            const float gamma, const float cos_gamma_sq)
{
    return _Yz
        * ( (1.0f + (_AY * expf(_BY / cos_theta)))
        * (1.0f + (_CY * expf(_DY * gamma)) + (_EY * cos_gamma_sq)) )
        / ( (1.0f + (_AY * expf(_BY)))
        * (1.0f + (_CY * expf(_DY * _theta_sun)) + (_EY * _cos_theta_sun_squared)) );
}

inline float SkyDome::_RedFunction(const float /*theta*/, const float theta_0_1,
                                         const float /*gamma*/, const float gamma_1_0)
{
    return ( (_Ar * powf(theta_0_1, _Br) * (1.0f - _horiz_atten_r * _sunset_atten))  // horizon light
        + (_Cr * powf(gamma_1_0, _Dr) * (1.0f - _solar_atten_r * _sunset_atten))     // circumsolar light
        + _Er )                            // overall light
        * _light_due_to_alt;
}

inline float SkyDome::_GreenFunction(const float /*theta*/, const float theta_0_1,
                                           const float /*gamma*/, const float gamma_1_0)
{
    return ( (_Ag * powf(theta_0_1, _Bg) * (1.0f - _horiz_atten_g * _sunset_atten))
        + (_Cg * powf(gamma_1_0, _Dg) * (1.0f - _solar_atten_g * _sunset_atten))
        + _Eg )
        * _light_due_to_alt;
}

inline float SkyDome::_BlueFunction(const float /*theta*/, const float theta_0_1,
                                          const float /*gamma*/, const float gamma_1_0)
{
    return ( (_Ab * powf(theta_0_1, _Bb) * (1.0f - _horiz_atten_b * _sunset_atten))
        + (_Cb * powf(gamma_1_0, _Db) * (1.0f - _solar_atten_b * _sunset_atten))
        + _Eb )
        * _light_due_to_alt;
}

void SkyDome::_computeSkyTexture()
{
    const float altitude = static_cast<float>(osg::DegreesToRadians(_sunAltitude));
    const float azimuth  = static_cast<float>(osg::DegreesToRadians(_sunAzimuth));

    _theta_sun = osg::DegreesToRadians(90.0 - _sunAltitude);
    _theta_sun_0_1 = (90.0 - _sunAltitude) / 90.0f;
    _cos_theta_sun = cosf(_theta_sun);
    _cos_theta_sun_squared = _cos_theta_sun * _cos_theta_sun;
    _sin_theta_sun = sinf(_theta_sun);

    // part of Preetham model
    //_updateZenithxyY();

    _sunset_atten = powf(_sin_theta_sun, 20.0f);

    osg::Image *image = _skyTexture->getImage();
    if( image != 0L )
    {
        unsigned char *ptr = image->data() + (_current_tex_row * SKY_DOME_X_SIZE * 3);

        osg::Vec3f sun_vec( sinf(azimuth) * cosf(altitude),
                            cosf(azimuth) * cosf(altitude),
                            sinf(altitude) );

        const int end_row( _current_tex_row + 4 );
        while(_current_tex_row < end_row)
        //for(int j=0; j<SKY_DOME_Y_SIZE; j++)
        {
            const float texel_alt( 1.57079633f - 
                ((float(_current_tex_row) + 0.5f) * 1.57079633f / float(SKY_DOME_Y_SIZE)) );
            const float cos_texel_alt( cosf(texel_alt) );
            const float sin_texel_alt( sinf(texel_alt) );
            // theta = angle between zenith and texel
            const float theta( 1.57079633f - texel_alt );
            // theta remapped from {0, pi} to {0, 1}
            const float theta_0_1( theta / 1.57079633f );

            for(int i=0; i<SKY_DOME_X_SIZE; ++i)
            {
                const float texel_azi( -1.57079633f - 
                    (float(i) + 0.5f) * 6.28318531f / float(SKY_DOME_X_SIZE) );
                const float cos_texel_azi( cosf(texel_azi) );
                const float sin_texel_azi( sinf(texel_azi) );
                osg::Vec3f texel_vec( sin_texel_azi * cos_texel_alt,
                                      cos_texel_azi * cos_texel_alt,
                                      sin_texel_alt );
                // gamma = angle between sun and texel
                const float cos_gamma( sun_vec * texel_vec );
                const float gamma( acosf(cos_gamma) );
                //const float cos_gamma_sq( cos_gamma * cos_gamma );
                // gamma remapped from {0, pi} to {1, 0}
                const float gamma_1_0( 1.0f - (gamma / 3.14159265f) );
                // gamma_1_0 weighted such that it is larger for texels that are lower in the sky
                // This is used for a more realistic--less circular--circumsolar glow
                const float weighted_gamma_1_0(powf(gamma_1_0, 1.0f - theta_0_1 * 0.9f) );

                // Run all this data through Preetham sky color math model
                /*const float x( _xDistributionFunction(theta, sin_texel_alt, gamma, cos_gamma_sq) );
                const float y( _yDistributionFunction(theta, sin_texel_alt, gamma, cos_gamma_sq) );
                const float Y( _YDistributionFunction(theta, sin_texel_alt, gamma, cos_gamma_sq) );

                // Convert xyY color space to XYZ color space
                // Conversions from Danny Pascale, "A Review of RGB Color Spaces"
                const float temp( Y / y );
                const float X( x * temp );
                const float Z( (1.0f - x - y) * temp );

                // Convert XYZ color space to sRGB color space
                float R( X *  3.2405 + -1.5371 * Y + -0.4985 * Z );
                float G( X * -0.9693 +  1.8760 * Y +  0.0416 * Z );
                float B( X *  0.0556 + -0.2040 * Y +  1.0572 * Z );*/

                // Our home grown sky color math model
                float R( _RedFunction(theta, theta_0_1, gamma, weighted_gamma_1_0) );
                float G( _GreenFunction(theta, theta_0_1, gamma, weighted_gamma_1_0) );
                float B( _BlueFunction(theta, theta_0_1, gamma, weighted_gamma_1_0) );
                
                // tone mapping
                const float exposure( 5.0f );
                const float luminance( R * 0.299f + G * 0.587f + B * 0.114f );
                const float brightness( 1.0f - expf(-luminance * exposure) );
                const float scale( brightness / (luminance + 0.001f) );
                R *= scale;
                G *= scale;
                B *= scale;

                // Clamp upper bound to 1.0
                // No need to clamp lower bound to 0.0 because our lighting is purely additive
                R = (R > 1.0f) ? 1.0f : R;
                G = (G > 1.0f) ? 1.0f : G;
                B = (B > 1.0f) ? 1.0f : B;
                
                *(ptr++) = (unsigned char)(R * 255.0f);
                *(ptr++) = (unsigned char)(G * 255.0f);
                *(ptr++) = (unsigned char)(B * 255.0f);
            }
            
            _current_tex_row ++;
        }
        
        if(_current_tex_row >= SKY_DOME_Y_SIZE)
            _current_tex_row = 0;
        
// DANG robert.... how about some backwards compatibility... especially with versions?
//#if (OSG_VERSION_MAJOR >= 2) && (OSG_VERSION_MINOR >= 6 )
#if ( OPENSCENEGRAPH_MAJOR_VERSION > 2 ) || ( ( OPENSCENEGRAPH_MAJOR_VERSION >= 2 ) && ( OPENSCENEGRAPH_MINOR_VERSION >= 6 ) )
        image->dirty();
        _skyTexture->setImage( image );
#else
        _skyTexture->setImage( image );
#endif
    }
}
