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
#include <osgText/Text>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Texture1D>
#include <osg/Point>
#include <osg/ClipPlane>
#include <osgUtil/CullVisitor>

#include <osgEphemeris/StarField.h>
#include <osgEphemeris/SkyDome.h>

#include <osg/Version>

#include "star_data.h"

using namespace osgEphemeris;

const double StarField::_defaultRadius = SkyDome::getMeanDistanceToMoon() * 1.1;

StarField::StarField( const std::string &fileName, double radius ):
    _radius(radius)
{
    if( fileName.empty() || _parseFile( fileName ) == false )
    {
        if( !fileName.empty() )
            std::cerr << "Warning.. unable to use star field defined in file \"" << fileName << "\".  Using default star field." << std::endl;
        /*
         * can't use this method for Winders, because... well, you guessed it.
         * Previously, star_data was:
         *
         *   static const char *star_data = 
         *       "Sirius,1.767793,-0.291751,-1.46\n"
         *       "Canopus,1.675305,-0.919716,-0.72\n"
         *       "Arcturus,3.733528,0.334798,-0.04\n"
         *       ...
         *       ;

        std::stringstream ss(star_data);
        _parseStream( ss );
        */

        for( const char **sptr = star_data; *sptr; sptr++ )
        {
            std::stringstream ss(*sptr);
            _stars.push_back( StarData(ss) );
        }
    }

    _buildGeometry();
    //_buildLabels();
    // We will need this if we need to rebuild the labels
    //_stars.clear();
}

static inline osg::ref_ptr<osgText::Text> makeText( const std::string &textString, osg::Vec3 pos )
{
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    text->setFont( "fonts/arial.ttf" );
    text->setFontResolution( 252, 252 );
    text->setAlignment( osgText::Text::LEFT_BOTTOM );
    text->setAutoRotateToScreen( true );
    text->setCharacterSizeMode( osgText::Text::SCREEN_COORDS );
    text->setColor( osg::Vec4( 1, 1, 0, 1 ));
    text->setText( textString );
    text->setCharacterSize( 25.0 );
    text->setPosition( pos );

    return text;
}

void StarField::_buildLabels()
{
    _starLabelsGeode = new osg::Geode;
    std::vector<StarData>::iterator p;
    for( p = _stars.begin(); p != _stars.end(); p++ )
    {
        if( p->magnitude < 1.8 || p->name == "Polaris" )
        {
            double ra = p->right_ascension;
            double dc = p->declination;

            osg::Vec3 pos = osg::Vec3(0,_radius,0) *
                                osg::Matrix::rotate( dc, 1, 0, 0 ) *
                                osg::Matrix::rotate( ra, 0, 0, 1 );

            _starLabelsGeode->addDrawable( makeText( p->name, pos ).get());
        }
    }

    addChild( _starLabelsGeode.get() );
}


std::string StarField::_vertexShaderProgram = 
    "uniform float starAlpha;"
    "uniform float pointSize;"
    "varying vec4 starColor;"
    "void main()"
    "{"
    "    starColor = gl_Color - 1.0 + starAlpha;"
    "    gl_PointSize = pointSize;"
    "    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;"
    "    gl_Position = ftransform();"
    "}"
    ;

std::string StarField::_fragmentShaderProgram = 
    "varying vec4 starColor;"
    "void main( void )"
    "{"
    "    gl_FragColor = starColor;"
    "}";

class UPCB : public osg::NodeCallback
{
    public:
        UPCB( osg::Uniform *MVi ): _MVi(MVi), a(0.0) {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(nv);
            if( cv != 0L )
            {
#if (((OSG_VERSION_MAJOR>=1) && (OSG_VERSION_MINOR>2)) || (OSG_VERSION_MAJOR>=2))
                osg::Matrixf m = *(cv->getModelViewMatrix());
#else
                osg::Matrixf m = cv->getModelViewMatrix();
#endif

                osg::Matrixf mi;
                mi.invert(m);
                
                //a += osg::PI/180.0;
                //mi = osg::Matrix::rotate( a, 1, 0, 0 );

                mi(3,0) = mi(3,1) = mi(3,2) = 0.0;

                _MVi->set( mi );
            }
            traverse(node,nv);
        }
    private:
        osg::ref_ptr<osg::Uniform>_MVi;
        double a;
};

void StarField::_buildGeometry()
{
    osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;

    std::vector<StarData>::iterator p;
    for( p = _stars.begin(); p != _stars.end(); p++ )
    {
        osg::Vec3 v = osg::Vec3(0,_radius,0) * 
                        osg::Matrix::rotate( p->declination, 1, 0, 0 ) * 
                        osg::Matrix::rotate( p->right_ascension, 0, 0, 1 );


        coords->push_back( v );

        double c = 1.0 - (p->magnitude/8.0);
        colors->push_back(osg::Vec4 (c,c,c,1.0));
    }

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray( coords.get() );
    geometry->setColorArray( colors.get() );
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, coords->size()));

    osg::ref_ptr<osg::StateSet> sset = new osg::StateSet;

    for( int i= 0; i < 8; i++ )
        sset->setTextureMode( i, GL_TEXTURE_2D, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );
    sset->setMode(  GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED );

    sset->setMode( GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON );
    osg::ref_ptr<osg::Program> program = new osg::Program;

    program->addShader( new osg::Shader(osg::Shader::VERTEX, _vertexShaderProgram ));
    /*
    osg::ref_ptr<osg::Shader> vertexShader = new osg::Shader(osg::Shader::VERTEX);
    vertexShader->loadShaderSourceFromFile("./stars.vert");
    program->addShader( vertexShader.get() );
    */

     program->addShader( new osg::Shader(osg::Shader::FRAGMENT, _fragmentShaderProgram ));
     /*
     osg::ref_ptr<osg::Shader> fragmentShader = new osg::Shader(osg::Shader::FRAGMENT);
     fragmentShader->loadShaderSourceFromFile("./stars.frag");
     program->addShader( fragmentShader.get() );
     */

    sset->setAttributeAndModes( program.get(), osg::StateAttribute::ON );

    _starAlpha = new osg::Uniform( "starAlpha", 1.0f );
    _pointSize = new osg::Uniform( "pointSize", 2.4f );


    sset->addUniform( _starAlpha.get() );
    sset->addUniform( _pointSize.get() );

    /*
    _MVi = new osg::Uniform( "MVi", osg::Matrix::identity() );
    sset->addUniform( _MVi.get() );
    setCullCallback( new UPCB( _MVi.get() ));

    osg::ClipPlane *cp = new osg::ClipPlane(0,  1.0, 0.0, 0.0, 0.0 );
    sset->setAttributeAndModes( cp );
    */

    sset->setRenderBinDetails(-10,"RenderBin");
    geometry->setStateSet( sset.get() );

    _starGeode = new osg::Geode;
    _starGeode->addDrawable( geometry.get() );

    addChild( _starGeode.get() );
}

void StarField::setSunAltitude( double sunAltitude )
{
#if 0 // Nope..   textuing brings performance to its knees.  we will need to use a shader here
    osg::Image *image = _tex->getImage();
    if( image != 0L )
    {
        unsigned char *ptr = image->data();

        double alpha = sunAltitude > 0.0 ? 0.0 : fabs(sunAltitude/10.0);

        // Transparent stars to see the right color for the dark part of the moon.
        if( alpha > 1.0 )
            alpha = 1.0;
        if( alpha < 0.0 ) 
            alpha = 0.0;

        unsigned char calpha = (unsigned char)(alpha * 255.0);
        for( int i = 0; i < 4; i++ )
        {
            *(ptr++) = 0xFF;
            *(ptr++) = calpha;
        }

        _tex->setImage( image );
    }
#endif

    float alpha = sunAltitude > 0.0 ? 0.0 : fabs(sunAltitude/10.0);
    if( alpha > 1.0 ) 
        alpha = 1.0;
    _starAlpha->set( alpha );
}


unsigned int StarField::getNumStars() 
{ 
    return _stars.size(); 
}

StarField::StarData::StarData( std::stringstream &ss )
{
    getline( ss, name, ',' );
    std::string buff;
    getline( ss, buff, ',' );
    std::stringstream(buff) >> right_ascension;
    getline( ss, buff, ',' );
    std::stringstream(buff) >> declination;
    getline( ss, buff, '\n' );
    std::stringstream(buff) >> magnitude;
}



void StarField::_parseStream( std::istream  &is )
{
    while( !is.eof() )
    {
        std::string line;

        std::getline( is, line );
        if( is.eof() )
            break;

        if( line.empty() || line[0] == '#' ) 
            continue;

        std::stringstream ss(line);
        _stars.push_back( StarData(ss) );
    }
}

bool StarField::_parseFile( const std::string fileName )
{
    std::fstream in( fileName.c_str() );
    if( !in )
    {
        std::cerr <<  "StarField... unable to open file \"" << fileName << "\"" << std::endl;
        return false ;
    }

    _parseStream( in );
    in.close();

    return true;
}


/*
void StarField::traverse(osg::NodeVisitor&nv)
{
    osg::MatrixTransform::traverse( nv );
}
*/
