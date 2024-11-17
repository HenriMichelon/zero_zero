/*
 * Copyright (c) 2024 Henri Michelon
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
*/
module;
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
        uint32_t            updateDelay{100};
        //! Draw with depth-testing, can hide center of mass transforms & world transforms
        bool                drawWithDepthTest{true};
        //! Draw coordinate system (x = red, y = green, z = blue)
        bool                drawCoordinateSystem{false};
        //! Coordinate system world position
        vec3                drawCoordinateSystemPosition{0.0f};
        //! Coordinate system world position
        float               drawCoordinateSystemScale{1.0f};
        //! Draw the faces that were found colliding during collision detection
		bool			    drawGetSupportingFace{false};
        //! Draw the collision shapes of all collision objects
		bool			    drawShape{true};
        //! The collision shapes will be drawn in wireframe instead of solid
        bool                drawShapeWireframe{true};
        //! Coloring scheme to use for collision shapes
		DebugShapeColor	    drawShapeColor{DebugShapeColor::ShapeTypeColor};
        //! Draw a bounding box per collision object
		bool				drawBoundingBox{false};
        //! Draw the velocity vector for collision object
		bool				drawVelocity{false};
        //! Draw the mass and inertia (as the box equivalent) for collision object
		bool				drawMassAndInertia{false};
        //! Draw stats regarding the sleeping algorithm of each collision object
		bool				drawSleepStats{false};
        //! Draw the center of mass for each collision object
        bool                drawCenterOfMassTransform{false};
        //! Draw the world transform (which can be different from the center of mass) for each collision object
        bool				drawWorldTransform{false};

	};

}