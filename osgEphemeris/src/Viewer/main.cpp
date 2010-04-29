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
#include <osgViewer/Viewer>

#include <osg/NodeCallback>
#include <osg/StateSet>
#include <osg/LightModel>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>
#include <osgViewer/ViewerEventHandlers>

#include <osgEphemeris/EphemerisModel.h>

#include "Compass.h"

// If added to the EphemerisModel, this callback will increment time by one 
// minute per frame.
class TimePassesCallback : public osgEphemeris::EphemerisUpdateCallback
{
    public:
        TimePassesCallback(): EphemerisUpdateCallback( "TimePassesCallback" ),
             _seconds(0)
        {}

        void operator()( osgEphemeris::EphemerisData *data )
        {
             _seconds += 60;        // 1 minute
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

// If called from the frame loop, this function will increment time by 5 
// minutes per frame.
static void SetCurrentTime( osgEphemeris::EphemerisModel &ephem )
{
    //time_t seconds = time(0L);
    static time_t seconds = 0;
    seconds += 60 * 5;      // 5 minutes
    struct tm *_tm = localtime(&seconds);

    osgEphemeris::EphemerisData *data = ephem.getEphemerisData();

    data->dateTime.setYear( _tm->tm_year + 1900 ); // DateTime uses _actual_ year (not since 1900)
    data->dateTime.setMonth( _tm->tm_mon + 1 );    // DateTime numbers months from 1 to 12, not 0 to 11
    data->dateTime.setDayOfMonth( _tm->tm_mday + 1 ); // DateTime numbers days from 1 to 31, not 0 to 30
    data->dateTime.setHour( _tm->tm_hour );
    data->dateTime.setMinute( _tm->tm_min );
    data->dateTime.setSecond( _tm->tm_sec );
}

// This handler lets you control the passage of time using keys.
// TODO: add keys to make the time increment by a given increment each frame
// with control over the increment.
class TimeChangeHandler : public osgGA::GUIEventHandler
{
public:
    TimeChangeHandler(osgEphemeris::EphemerisModel *ephem) : m_ephem(ephem) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add)
            {
                // Increment time
                // Hopefully the DateTime will wrap around correctly if we get 
                // to invalid dates / times...
                osgEphemeris::EphemerisData* data = m_ephem->getEphemerisData();
                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Increment by one hour
                    data->dateTime.setHour( data->dateTime.getHour() + 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Increment by one day
                    data->dateTime.setDayOfMonth( data->dateTime.getDayOfMonth() + 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Increment by one month
                    data->dateTime.setMonth( data->dateTime.getMonth() + 1 );
                else                                                                    // Increment by one minute
                    data->dateTime.setMinute( data->dateTime.getMinute() + 1 );

                return true;
            }

            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract)
            {
                // Decrement time
                // Hopefully the DateTime will wrap around correctly if we get 
                // to invalid dates / times...
                osgEphemeris::EphemerisData* data = m_ephem->getEphemerisData();
                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                    data->dateTime.setHour( data->dateTime.getHour() - 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                    data->dateTime.setDayOfMonth( data->dateTime.getDayOfMonth() - 1 );
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                    data->dateTime.setMonth( data->dateTime.getMonth() - 1 );
                else                                                                    // Decrement by one minute
                    data->dateTime.setMinute( data->dateTime.getMinute() - 1 );

                return true;
            }

        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Increment time by one minute");
        usage.addKeyboardMouseBinding("Shift Keypad +", "Increment time by one hour"  );
        usage.addKeyboardMouseBinding("Alt Keypad +",   "Increment time by one day"   );
        usage.addKeyboardMouseBinding("Ctrl Keypad +",  "Increment time by one month" );
        usage.addKeyboardMouseBinding("Keypad -",       "Decrement time by one minute");
        usage.addKeyboardMouseBinding("Shift Keypad -", "Decrement time by one hour"  );
        usage.addKeyboardMouseBinding("Alt Keypad -",   "Decrement time by one day"   );
        usage.addKeyboardMouseBinding("Ctrl Keypad -",  "Decrement time by one month" );
    }

    osg::ref_ptr<osgEphemeris::EphemerisModel> m_ephem;
};

int main(int argc, char **argv)
{
    // Parse command line arguments 
    osg::ArgumentParser args( &argc, argv );
    osg::ref_ptr<osg::Group> root = new osg::Group;


    // Set up the viewer
    osgViewer::Viewer viewer(args);

    // Set the clear color to black
    viewer.getCamera()->setClearColor(osg::Vec4(0,0,0,1));

    // Use a default camera manipulator
    osgGA::TrackballManipulator* manip = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(manip);
    // Initially, place the TrackballManipulator so it's in the center of the scene
    manip->setHomePosition(osg::Vec3(0,0,0), osg::Vec3(0,1,0), osg::Vec3(0,0,1));
    manip->home(0);

    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);

    // Tell the viewer what to display for a help message
    viewer.getUsage(*args.getApplicationUsage());


    // Load up the models specified on the command line
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(args);
    if( model.valid() )
        root->addChild( model.get() );

    // Add a compass model so we can see which way we are facing
    osg::ref_ptr<osg::Viewport> vp = new osg::Viewport( 320, 20, 640, 100 );
    root->addChild( new Compass( vp.get() ));


    // Define the Ephemeris Model
    osg::ref_ptr<osgEphemeris::EphemerisModel> ephemerisModel = new osgEphemeris::EphemerisModel;
    root->addChild( ephemerisModel.get() );


    // Set some acceptable defaults.
    double latitude = 38.4765;                                  // San Francisco, California
    double longitude = -122.493;
    osgEphemeris::DateTime dateTime( 2010, 4, 1, 8, 0, 0 );     // 8 AM
    double radius = 10000;                                      // Default radius in case no files were loaded above
    osg::BoundingSphere bs = root->getBound();
    if (bs.valid())                                             // If the bs is not valid then the radius is -1
        radius = bs.radius()*2;                                 // which would result in an inside-out skydome...

    ephemerisModel->setLatitudeLongitude( latitude, longitude );
    ephemerisModel->setDateTime( dateTime );
    ephemerisModel->setSkyDomeRadius( radius );


    // Optionally, Set the AutoDate and Time to false so we can control it with the GUI
    //ephemerisModel->setAutoDateTime( false );

    // Optionally, uncomment this if you want to move the Skydome, Moon, Planets and StarField with the mouse
    /*
    ephemerisModel->setMoveWithEyePoint(false);
    // Reposition the TrackballManipulator to see the whole scene.
    manip->setNode(root.get());
    manip->setAutoComputeHomePosition(true);
    manip->home(0);
    */

    // Optionally, use a callback to update the Ephemeris Data
    //ephemerisModel->setEphemerisUpdateCallback( new TimePassesCallback );

    // Optionally, use a handler to update the Ephemeris Data
    viewer.addEventHandler( new TimeChangeHandler( ephemerisModel.get() ) );


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
        // Optionally, update the Ephemeris Data from the main viewer loop
        //SetCurrentTime( *ephemerisModel.get() );
        viewer.frame();
    }
    return 0;
}
