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
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>

#include <osg/NodeCallback>
#include <osg/StateSet>
#include <osg/LightModel>

#include <osgEphemeris/EphemerisModel.h>

#include "Compass.h"

class TimePassesCallback : public osgEphemeris::EphemerisUpdateCallback
{
    public:
        TimePassesCallback(): EphemerisUpdateCallback( "TimePassesCallback" ),
             _seconds(0)
        {}

        void operator()( osgEphemeris::EphemerisData *data )
        {
             _seconds += 60;
             struct tm *_tm = localtime(&_seconds);

             data->dateTime.setYear( _tm->tm_year + 1900 );
             data->dateTime.setMonth( _tm->tm_mon + 1 );
             data->dateTime.setDayOfMonth( _tm->tm_mday + 1 );
             data->dateTime.setHour( _tm->tm_hour );
             data->dateTime.setMinute( _tm->tm_min );
             data->dateTime.setSecond( _tm->tm_sec );
        }

    private:
        time_t _seconds;
};

static void SetCurrentTime( osgEphemeris::EphemerisModel &ephem )
{
    //time_t seconds = time(0L);
static time_t seconds = 0;
seconds += 60 * 5;
    struct tm *_tm = localtime(&seconds);

    osgEphemeris::EphemerisData *data = ephem.getEphemerisData();

    data->dateTime.setYear( _tm->tm_year + 1900 ); // DateTime uses _actual_ year (not since 1900)
    data->dateTime.setMonth( _tm->tm_mon + 1 );    // DateTime numbers months from 1 to 12, not 0 to 11
    data->dateTime.setDayOfMonth( _tm->tm_mday + 1 ); // DateTime numbers days from 1 to 31, not 0 to 30
    data->dateTime.setHour( _tm->tm_hour );
    data->dateTime.setMinute( _tm->tm_min );
    data->dateTime.setSecond( _tm->tm_sec );
}

int main(int argc, char **argv)
{
    // Parse command line arguments 
    osg::ArgumentParser args( &argc, argv );

    // Set up the viewer
    osgProducer::Viewer viewer(args);
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // Tell the viewer what to display for a help message
    viewer.getUsage(*args.getApplicationUsage());

    // Load up the models specified on the command line
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(args);
    if( model.valid() )
        root->addChild( model.get() );

    // Add a compass model so we can see which way we are facing
    osg::ref_ptr<osg::Viewport> vp = new osg::Viewport( 320, 20, 640, 100 );
    root->addChild( new Compass( vp.get() ));

    // Define the Ephemeris Model and its radius
    osg::BoundingSphere bs = root->getBound();
    osg::ref_ptr<osgEphemeris::EphemerisModel> ephemerisModel = new osgEphemeris::EphemerisModel;
    ephemerisModel->setSkyDomeRadius( bs.radius()*2 );

    // Optionally, Set the AutoDate and Time to false so we can control it with the GUI
    //ephemerisModel->setAutoDateTime( false );

    // Optionally, uncomment this if you want to move the Skydome, Moon, Planets and StarField with the mouse
    //ephemerisModel->setMoveWithEyePoint(false);

    // Use a callback to update the Ephemeris Data
    //ephemerisModel->setEphemerisUpdateCallback( new TimePassesCallback );

    root->addChild( ephemerisModel.get() );

    //Experiment with setting the LightModel to very dark to get better sun lighting effects
    {
        osg::ref_ptr<osg::StateSet> sset = root->getOrCreateStateSet();
        osg::ref_ptr<osg::LightModel> lightModel = new osg::LightModel;
        lightModel->setAmbientIntensity( osg::Vec4( 0.0025, 0.0025,0.0025, 1.0 ));
        sset->setAttributeAndModes( lightModel.get() );
    }

    osgUtil::Optimizer optimizer;
    optimizer.optimize(root.get());

    // set the scene to render
    viewer.setSceneData(root.get());

    // Realize the viewer
    viewer.realize();
    while( !viewer.done() )
    {
        viewer.sync();
        SetCurrentTime( *ephemerisModel.get() );
        viewer.update();
        viewer.frame();
    }
    viewer.sync();
    return 0;
}
