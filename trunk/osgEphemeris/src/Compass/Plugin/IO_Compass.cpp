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
#include <string>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "../Lib/Compass.h"

bool Compass_readLocalData(osg::Object& obj, osgDB::Input& fr);
bool Compass_writeLocalData(const osg::Object& obj, osgDB::Output& fw);

osgDB::RegisterDotOsgWrapperProxy g_CompassProxy (
     new Compass,
         "Compass",
         "Compass",
          &Compass_readLocalData,
          &Compass_writeLocalData,
          osgDB::DotOsgWrapper::READ_AND_WRITE
);

bool Compass_readLocalData(osg::Object& obj, osgDB::Input& fr)
{
    bool itAdvanced = false;
    Compass &compass = static_cast<Compass &>(obj);

    if( fr[0].matchWord("viewport")) 
    {
        ++fr;
        int x = atoi( fr[0].getStr() );
        ++fr;
        int y = atoi( fr[0].getStr() );
        ++fr;
        int width = atoi( fr[0].getStr() );
        ++fr;
        int height = atoi( fr[0].getStr() );

        compass.setViewport( new osg::Viewport( x, y, width, height ));

        itAdvanced = true;
    }

    return itAdvanced;
}

bool Compass_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const Compass &compass = static_cast<const Compass &>(obj);

    const osg::Viewport *vp = compass.getViewport();
    fw.indent() << " viewport " << vp->x() << " " << vp->y() << " " << vp->width() << " " << vp->height() << std::endl;
         

    return true;
}
