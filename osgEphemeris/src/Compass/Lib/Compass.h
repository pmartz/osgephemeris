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

#include <osg/MatrixTransform>
#include <osg/Projection>
#include <osg/Vec4>

#include <osg/NodeVisitor>
#include <osg/ClipPlane>
#include <osg/Viewport>


class Compass : public osg::Projection
{
    public:
        Compass();
        Compass( osg::Viewport *vp);

        Compass(const Compass& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

        META_Node(Compass, Compass);

        void setViewport( osg::Viewport *vp );
        const osg::Viewport *getViewport() const;

    protected:

        virtual void traverse(osg::NodeVisitor&);
        //static const osg::Vec4 color;

    private:
        osg::ref_ptr<osg::MatrixTransform> _tx;
        osg::ref_ptr<osg::MatrixTransform> _ltx;
        osg::ref_ptr<osg::ClipPlane> _clipPlane;

        osg::ref_ptr<osg::Geode> _makeGeode();
        osg::ref_ptr<osg::Geode> _lineGeode();

        osg::ref_ptr<osg::Viewport> _viewport;

        bool _initialized;
        void _init();

};
