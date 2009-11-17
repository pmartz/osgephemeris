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

#include <string>
#include <osgDB/ReadFile>
#include <stdio.h>
#include <osg/Image>


int main(int argc, char **argv )
{
    if( argc < 3 )
        return 1;

    std::string moonImageFile = std::string(argv[1]);
    std::string moonNormalImageFile = std::string(argv[2]);

    if( moonImageFile.empty() || moonNormalImageFile.empty() )
        return 1;
    /*
        static unsigned int _moonImageLoLodWidth, _moonImageLoLodHeight;
        static unsigned char _moonImageLoLodData[];
        static unsigned int _moonImageHiLodWidth, _moonImageHiLodHeight;
        static unsigned char _moonImageHiLodData[];

        static unsigned int _moonNormalImageLoLodWidth, _moonNormalImageLoLodHeight;
        static unsigned char _moonNormalImageLoLodData[];
        static unsigned int _moonNormalImageHiLodWidth, _moonNormalImageHiLodHeight;
        static unsigned char _moonNormalImageHiLodData[];
        */


    osg::ref_ptr<osg::Image> moonImage = osgDB::readImageFile( moonImageFile );

    printf( "\n#include <Ephemeris/MoonModel>\n\n" );
    printf( "osgEphemeris::MoonModel::_moonImageHiLodWidth  = %u;\n", moonImage->s() );
    printf( "osgEphemeris::MoonModel::_moonImageHiLodHeight = %u;\n", moonImage->t() );

    //unsigned char *ptr = moonImage->data();

    return 0;
}
