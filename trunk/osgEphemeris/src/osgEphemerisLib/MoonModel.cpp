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
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osg/StateSet>
#include <osg/Texture2D>

#include <osgEphemeris/MoonModel.h>

using namespace osgEphemeris;

const double MoonModel::_moonRadius = 3476000.0 * 0.5; // Diameter of moon * 0.5 = moon radius;

MoonModel::MoonModel():
    Sphere( _moonRadius,
            Sphere::TessNormal,
            Sphere::OuterOrientation,
            Sphere::BothHemispheres
            ) ,
    _baseTextureUnit(0),
    _bumpTextureUnit(1)
{
    _buildStateSet();
}

std::string MoonModel::_vertexShaderProgram = 
    "    attribute vec3 Tangent;"
    "    uniform vec3 light;"
    "    varying vec2 uv;"
    "    varying vec3 lightVec;"
    "    varying vec3 halfVec;"
    "    varying vec3 eyeVec;"
    "    void main()"
    "    {"
    "        gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;"
    "        gl_Position = ftransform();"
    "        gl_FrontColor = vec4(1,1,1,1);"
    "        uv = gl_MultiTexCoord0.st;"
    " uv.s -= 0.25;"
    //" uv.t = 1.0- uv.t;"
    "        vec3 n = normalize( gl_Normal);"
    "        vec3 t = normalize( Tangent);"
    "        vec3 bi = normalize(cross(n, t));"
    "        mat3 tangentBasis = mat3 (t, bi, n);"
    "        lightVec = light * tangentBasis;"
    "    }";

//    "        vec3 n = normalize(gl_NormalMatrix * gl_Normal);"
//    "        vec3 t = normalize(gl_NormalMatrix * Tangent);"
    //"const float diffuseCoeff = 0.7;"

std::string MoonModel::_fragmentShaderProgram = 
    "uniform sampler2D baseMap;"
    "uniform sampler2D normalMap;"
    "varying vec2 uv;"
    "varying vec3 lightVec;"
    "const float diffuseCoeff = 2.5;"
    "void main( void )"
    "{"
    "    vec2 texUV = uv;"
    "    vec3 normal = 2.0 * (texture2D(normalMap, texUV).rgb - 0.5);"
    "    normal = normalize(normal);"
    "    float diffuse = max(dot(lightVec, normal), 0.0) * diffuseCoeff;"
    "    vec3 decalColor = texture2D(baseMap, texUV).rgb;"
    "    gl_FragColor = vec4(vec3(diffuse) * decalColor, 1.0);"
    "}";

void MoonModel::_buildStateSet()
{
    osg::ref_ptr<osg::StateSet> sset = new osg::StateSet;
    sset->setGlobalDefaults();

    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX, _vertexShaderProgram ));
    /*
    osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX);
    vertexShader->loadShaderSourceFromFile("prog.vert");
    program->addShader( vertexShader.get() );
    */

    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, _fragmentShaderProgram ));
    /*
    osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
    fragmentShader->loadShaderSourceFromFile("prog.frag");
    program->addShader( fragmentShader.get() );
    */

    sset->setAttributeAndModes( program.get(), osg::StateAttribute::ON );

    //osg::ref_ptr<osg::Image> baseImage = osgDB::readImageFile( "moon.jpg" );

    osg::ref_ptr<osg::Image> baseImage = new osg::Image;
    baseImage->setImage( 
            _moonImageHiLodWidth, 
            _moonImageHiLodHeight, 1,
            _moonImageInternalTextureFormat,
            _moonImagePixelFormat,
            GL_UNSIGNED_BYTE,
            _moonImageHiLodData,
            osg::Image::NO_DELETE );


    osg::ref_ptr<osg::Texture2D> baseTexture = new osg::Texture2D;
    baseTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    baseTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    baseTexture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    baseTexture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    baseTexture->setImage( baseImage.get() );
    sset->setTextureAttributeAndModes( _baseTextureUnit, baseTexture.get() );


    //osg::ref_ptr<osg::Image> bumpImage = osgDB::readImageFile( "moon_normal.jpg" );

    osg::ref_ptr<osg::Image> bumpImage = new osg::Image;
    bumpImage->setImage( 
            _moonNormalImageHiLodWidth, 
            _moonNormalImageHiLodHeight, 1,
            _moonNormalImageInternalTextureFormat,
            _moonNormalImagePixelFormat,
            GL_UNSIGNED_BYTE,
            _moonNormalImageHiLodData,
            osg::Image::NO_DELETE );

    osg::ref_ptr<osg::Texture2D> bumpTexture = new osg::Texture2D;
    bumpTexture->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
    bumpTexture->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    bumpTexture->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    bumpTexture->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
    bumpTexture->setImage( bumpImage.get());
    sset->setTextureAttributeAndModes( _bumpTextureUnit, bumpTexture.get() );

    sset->addUniform( new osg::Uniform("baseMap",   _baseTextureUnit ));
    sset->addUniform( new osg::Uniform("normalMap", _bumpTextureUnit ));

    _light = new osg::Uniform("light", osg::Vec3(0,-1,0));
    sset->addUniform( _light.get() );

    sset->setRenderBinDetails(-10,"RenderBin");

    setStateSet( sset.get() );
}

void MoonModel::setSunPosition( osg::Vec3 sun )
{
    _light->set( sun );
}

