module;
#include "z0/jolt.h"
#include "z0/libraries.h"
#include <Jolt/Physics/Collision/ContactListener.h>

export module z0:Physics;

import :Signal;

export namespace z0 {

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
        [[nodiscard]] const char * GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override { return "?";}
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