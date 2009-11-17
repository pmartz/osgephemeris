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

#include <osgEphemeris/EphemerisUpdateCallback.h>

using namespace osgEphemeris;


EphemerisUpdateCallbackRegistry::EphemerisUpdateCallbackRegistry()
{
}

EphemerisUpdateCallbackRegistry *EphemerisUpdateCallbackRegistry::instance()
{
    static EphemerisUpdateCallbackRegistry *s = new EphemerisUpdateCallbackRegistry;
    return s;
}

//void UpdateCallbackRegistry::registerUpdateCallback( const std::string &name, UpdateCallback * callback )
void EphemerisUpdateCallbackRegistry::registerUpdateCallback( EphemerisUpdateCallback * callback )
{
        _callbacks[callback->getName()] = callback;
}

EphemerisUpdateCallback *EphemerisUpdateCallbackRegistry::getUpdateCallback( const std::string &name )
{
    std::map<std::string, osg::ref_ptr<EphemerisUpdateCallback> >::iterator p;
    if( (p = _callbacks.find(name)) == _callbacks.end() )
        return 0L;
    return (*p).second.get();
}

