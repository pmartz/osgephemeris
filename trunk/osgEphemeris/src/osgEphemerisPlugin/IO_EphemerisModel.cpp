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

#ifndef WIN32
#include <strings.h>
#endif

#include <iostream>
#include <string>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/DynamicLibrary>
#include <osgDB/FileUtils>

#include <osgEphemeris/EphemerisModel.h>
#include <osgEphemeris/EphemerisUpdateCallback.h>

bool EphemerisModel_readLocalData(osg::Object& obj, osgDB::Input& fr);
bool EphemerisModel_writeLocalData(const osg::Object& obj, osgDB::Output& fw);

osgDB::RegisterDotOsgWrapperProxy g_EphemerisModelProxy (
     new osgEphemeris::EphemerisModel,
         "EphemerisModel",
         "EphemerisModel",
          &EphemerisModel_readLocalData,
          &EphemerisModel_writeLocalData,
          osgDB::DotOsgWrapper::READ_AND_WRITE

);

bool EphemerisModel_readLocalData(osg::Object& obj, osgDB::Input& fr)
{
    bool itAdvanced = false;
    osgEphemeris::EphemerisModel &em = static_cast<osgEphemeris::EphemerisModel &>(obj);

    if( fr[0].matchWord("Latitude")) 
    {
        ++fr;
        double latitude = atof( fr[0].getStr() );
        ++fr;
        em.setLatitude( latitude );
        itAdvanced = true;
    }

    if( fr[0].matchWord("Longitude")) 
    {
        ++fr;
        double longitude = atof( fr[0].getStr() );
        ++fr;
        em.setLongitude( longitude );
        itAdvanced = true;
    }

    if( fr[0].matchWord("DateTime" ))
    {
        ++fr;
        std::string dateTimeStr(fr[0].getStr());
        ++fr;
        int year  = 0, 
            month = 0, 
            day   = 0, 
            hour  = 0, 
            min   = 0, 
            sec   = 0;

        if( dateTimeStr.length() >= 4 )
            year  = atoi( dateTimeStr.substr(0,4).c_str());
        if( dateTimeStr.length() >= 6 )
            month = atoi( dateTimeStr.substr(4,2).c_str());
        if( dateTimeStr.length() >= 8 )
            day   = atoi( dateTimeStr.substr(6,2).c_str());
        if( dateTimeStr.length() >= 10 )
            hour  = atoi( dateTimeStr.substr(8,2).c_str());
        if( dateTimeStr.length() >= 12 )
            min   = atoi( dateTimeStr.substr(10,2).c_str());
        if( dateTimeStr.length() >= 14 )
            sec   = atoi( dateTimeStr.substr(12,2).c_str());

        em.setDateTime( osgEphemeris::DateTime( 
                    year, month, day,
                    hour, min, sec ));

        itAdvanced = true;
    }

    if( fr[0].matchWord("SkyDomeRadius")) 
    {
        ++fr;
        double radius = atof( fr[0].getStr() );
        ++fr;
        em.setSkyDomeRadius( radius );
        itAdvanced = true;
    }

    if( fr[0].matchWord("SkyDomeCenter")) 
    {
        ++fr;
        double x = atof( fr[0].getStr() );
        double y = atof( fr[1].getStr() );
        double z = atof( fr[2].getStr() );
        ++fr; ++fr; ++fr;
        
        em.setSkyDomeCenter( osg::Vec3(x,y,z) );
        itAdvanced = true;
    }

    if( fr[0].matchWord( "AutoDateTime" ))
    {
        ++fr;
#ifdef _MSC_VER
		bool flag = (!_stricmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#else
        bool flag = (!strcasecmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#endif
        em.setAutoDateTime( flag );
        ++fr;
        itAdvanced = true;
    }

    if( fr[0].matchWord( "MirrorSouthernHemisphere" ))
    {
        ++fr;
#ifdef _MSC_VER
		bool flag = (!_stricmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#else
        bool flag = (!strcasecmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#endif
        em.setSkyDomeMirrorSouthernHemisphere( flag );
        ++fr;
        itAdvanced = true;
    }

    if( fr[0].matchWord( "UseSouthernHemisphere" ))
    {
        ++fr;
#ifdef _MSC_VER
		bool flag = (!_stricmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#else
        bool flag = (!strcasecmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#endif
        em.setSkyDomeUseSouthernHemisphere( flag );
        ++fr;
        itAdvanced = true;
    }

    if( fr[0].matchWord( "MoveWithEyePoint" ))
    {
        ++fr;
#ifdef _MSC_VER
        bool flag = (!_stricmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#else
        bool flag = (!strcasecmp( fr[0].getStr(), "True" )) || atoi(fr[0].getStr());
#endif
       em.setMoveWithEyePoint( flag );
        ++fr;
        itAdvanced = true;
    }

    if( fr[0].matchWord( "SunLightNumber" ))
    {
        ++fr;
        int lightNum = atoi( fr[0].getStr() );
        em.setSunLightNum( lightNum );
        ++fr;
        itAdvanced = true;
    }
    if( fr[0].matchWord( "UpdateCallback" ))
    {
        ++fr;
        std::string callbackName(fr[0].getStr());
        std::string tlibName = callbackName.substr( 0, callbackName.find_first_of(":"));
        std::string procName = callbackName.substr( callbackName.find_last_of(":")+1);
        std::string libName = osgDB::findLibraryFile( tlibName );
         osgDB::DynamicLibrary::loadLibrary( libName );
         osgEphemeris::EphemerisUpdateCallback *cb = 
             osgEphemeris::EphemerisUpdateCallbackRegistry::instance()->getUpdateCallback( procName );
         if( cb != 0L )
             em.setEphemerisUpdateCallback( cb );

        ++fr;
    }

    return itAdvanced;
}

bool EphemerisModel_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgEphemeris::EphemerisModel &em = static_cast<const osgEphemeris::EphemerisModel &>(obj);
    fw.indent() << "Latitude " << em.getLatitude() << std::endl;
    fw.indent() << "Longitude " << em.getLongitude() << std::endl;
    fw.indent() << "SkyDomeRadius " << em.getSkyDomeRadius() << std::endl;
    fw.indent() << "AutoDateTime " << (em.getAutoDateTime()?"True":"False") << std::endl;
    fw.indent() << "MoveWithEyePoint " << (em.getMoveWithEyePoint()?"True":"False") << std::endl;
    fw.indent() << "SunLightNumber " << em.getSunLightNum() << std::endl;

    const osgEphemeris::EphemerisUpdateCallback *updateCallback = em.getEphemerisUpdateCallback();
    if( updateCallback != 0L )
        fw.indent() << "UpdateCallback " << updateCallback->getName() << std::endl;

    return true;
}
