#pragma once
#include "AnthemEnv.h"
#include "AnthemDefs.h"
#include "AnthemConfig.h"
#include "AnthemLogger.h"
#include "AnthemValLayer.h"
#include "AnthemPhyDeviceSelector.h"
#include "AnthemLogicalDeviceSelector.h"
#include "AnthemWindowSurface.h"

namespace Anthem{
    namespace Core{
        class AnthemEnvImpl: public AnthemEnv {
        private: //Member Variables
            GLFWwindow* window;
            VkApplicationInfo appInfo = {};
            VkInstanceCreateInfo createInfo = {};
            VkInstance instance;
            
            ANTH_SHARED_PTR(AnthemConfig) cfg;
            ANTH_SHARED_PTR(AnthemValLayer) valLayer;
            ANTH_SHARED_PTR(AnthemPhyDeviceSelector) phyDeviceSelector;
            ANTH_SHARED_PTR(AnthemLogicalDeviceSelector) logicalDeviceSelector;
            ANTH_SHARED_PTR(AnthemWindowSurface) windowSurface;

        public: //Member Functions
            AnthemEnvImpl(ANTH_SHARED_PTR(AnthemConfig) cfg);
            bool virtual createWindow(int w,int h) override;
            void virtual drawLoop() override;
            void virtual createInstance() override;
            void virtual destroyEnv() override;
            void virtual run() override; 
            void virtual init() override;
            
            std::vector<const char*> virtual getRequiredExtensions();

        };
    }
}


