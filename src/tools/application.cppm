module;
#include "z0/libraries.h"

export module z0:DummyApplication;

import :Application;
import :ApplicationConfig;
import :Node;

export namespace z0 {

    class DummyApplication : public Application {
    public:
        explicit DummyApplication(const ApplicationConfig &applicationConfig) : Application{applicationConfig, nullptr} {};

        ~DummyApplication() override {};

        float getAspectRatio() const override {return 1.0f;};

        uint64_t getDedicatedVideoMemory() const override {return 0;};

        const string &getAdapterDescription() const override {return "";};

        uint64_t getVideoMemoryUsage() const override {return 0;};

        void initRenderingSystem() override {};

        void processDeferredUpdates(uint32_t currentFrame) override {};

        inline void renderFrame(const uint32_t currentFrame) override {   }

        inline void waitForRenderingSystem() override {  }

        void setShadowCasting(const bool enable) const override {};

    };
}
