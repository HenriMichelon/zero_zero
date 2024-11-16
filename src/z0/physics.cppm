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

export module z0.Physics;

import z0.Signal;

export namespace z0 {

    enum class PhysicsDebugColor {
        InstanceColor,				///< Random color per instance
        ShapeTypeColor,				///< Convex = green, scaled = yellow, compound = orange, mesh = red
        MotionTypeColor,			///< Static = grey, keyframed = green, dynamic = random color per instance
        SleepColor,					///< Static = grey, keyframed = green, dynamic = yellow, sleeping = red
        IslandColor,				///< Static = grey, active = random color per island, sleeping = light grey
        MaterialColor,				///< Color as defined by the PhysicsMaterial of the shape
    };

    struct PhysicsDebugConfig {
        //! Delay in milliseconds between collision shapes updates for debug (between 0 and 500)
        uint32_t            updateDelay  = 100;
        bool                drawCoordinateSystem = false;
		bool			    drawGetSupportingFace = false;					///< Draw the faces that were found colliding during collision detection
		bool			    drawShape = true;								///< Draw the shapes of all bodies
		PhysicsDebugColor	drawShapeColor = PhysicsDebugColor::ShapeTypeColor; ///< Coloring scheme to use for shapes
		bool				drawBoundingBox = false;						///< Draw a bounding box per body
		// bool				drawCenterOfMassTransform = false;				///< Draw the center of mass for each body
		// bool				drawWorldTransform = false;					///< Draw the world transform (which can be different than the center of mass) for each body
		bool				drawVelocity = false;							///< Draw the velocity vector for each body
		bool				drawMassAndInertia = false;					///< Draw the mass and inertia (as the box equivalent) for each body
		bool				drawSleepStats = false;						///< Draw stats regarding the sleeping algorithm of each body
		// bool				drawSoftBodyVertices = false;					///< Draw the vertices of soft bodies
		// bool				drawSoftBodyVertexVelocities = false;			///< Draw the velocities of the vertices of soft bodies
		// bool				drawSoftBodyEdgeConstraints = false;			///< Draw the edge constraints of soft bodies
		// bool				drawSoftBodyBendConstraints = false;			///< Draw the bend constraints of soft bodies
		// bool				drawSoftBodyVolumeConstraints = false;			///< Draw the volume constraints of soft bodies
		// bool				drawSoftBodySkinConstraints = false;			///< Draw the skin constraints of soft bodies
		// bool				drawSoftBodyLRAConstraints = false;			///< Draw the LRA constraints of soft bodies
		// bool				drawSoftBodyPredictedBounds = false;			///< Draw the predicted bounds of soft bodies
		// ESoftBodyConstraintColor	mDrawSoftBodyConstraintColor = ESoftBodyConstraintColor::ConstraintType; ///< Coloring scheme to use for soft body constraints
	};


    // Class that determines if two nodes can collide
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer layersAndMask1, JPH::ObjectLayer layersAndMask2) const override;
    };

    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        [[nodiscard]] uint32_t GetNumBroadPhaseLayers() const override { return 1;}
        [[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override { return static_cast<JPH::BroadPhaseLayer>(0); }
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        [[nodiscard]] const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override { return "?";}
#endif

    };

    // Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        [[nodiscard]] bool ShouldCollide(JPH::ObjectLayer layers, JPH::BroadPhaseLayer masks) const override { return true; }
    };

    class ContactListener : public JPH::ContactListener {
    public:
        void OnContactAdded(const JPH::Body &inBody1,
                            const JPH::Body &inBody2,
                            const JPH::ContactManifold &inManifold,
                            JPH::ContactSettings &ioSettings) override;
        void OnContactPersisted(const JPH::Body &inBody1, 
                                const JPH::Body &inBody2, 
                                const JPH::ContactManifold &inManifold, 
                                JPH::ContactSettings &ioSettings) override;

    private:
        void emit(Signal::signal signal,
                  const JPH::Body &body1, 
                  const JPH::Body &body2, 
                  const JPH::ContactManifold &inManifold) const;
    };

}