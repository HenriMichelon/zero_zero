/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include "z0/libraries.h"

export module z0.DebugConfig;

export namespace z0 {

    enum class DebugShapeColor {
        //! Random color per instance
        InstanceColor,
        //! Convex = green, scaled = yellow, compound = orange, mesh = red
        ShapeTypeColor,
        //! Static = grey, keyframed = green, dynamic = random color per instance
        MotionTypeColor,
    };

    struct DebugConfig {
        //! Delay in milliseconds between collision shapes updates for debug (between 0 and 500)
        uint32_t            updateDelay  = 100;
        bool                drawCoordinateSystem = false;
        //! Draw the faces that were found colliding during collision detection
		bool			    drawGetSupportingFace = false;
        //! Draw the shapes of all collision objects
		bool			    drawShape = true;
        //! Coloring scheme to use for shapes
		DebugShapeColor	    drawShapeColor = DebugShapeColor::ShapeTypeColor;
        //! Draw a bounding box per collision object
		bool				drawBoundingBox = false;
        //! Draw the velocity vector for collision object
		bool				drawVelocity = false;
        //! Draw the mass and inertia (as the box equivalent) for collision object
		bool				drawMassAndInertia = false;
        //! Draw stats regarding the sleeping algorithm of each collision object
		bool				drawSleepStats = false;
	};

}