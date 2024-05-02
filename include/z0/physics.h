#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace z0 {

    // Class that determines if two nodes can collide
    class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer layersAndMask1, JPH::ObjectLayer layersAndMask2) const override {
            auto sourceMask = layersAndMask1 & 0b1111;
            auto targetLayer = (layersAndMask2 >> 4) & 0b1111;
            return (targetLayer & sourceMask) != 0;
        }
    };

    // This defines a mapping between object and broadphase layers.
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface {
    public:
        uint32_t GetNumBroadPhaseLayers() const override {
            return 1;
        }

        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
            return static_cast<JPH::BroadPhaseLayer>(0);
        }

        const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
            return "?";
        }};

    // Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
        bool ShouldCollide(JPH::ObjectLayer layers, JPH::BroadPhaseLayer masks) const override{
            return true;
        }
    };

}