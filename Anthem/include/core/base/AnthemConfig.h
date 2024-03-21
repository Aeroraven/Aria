#pragma once
#include <vector>

namespace Anthem{
    namespace Core{
        class AnthemConfig {
        public:
            const char* appName = "Anthem";
            const char* demoName = "Untitled";
            const char* engineName = "No Engine?";
            
            int appcfgResolutionWidth = 1440;
            int appcfgResolutionHeight = 900;

            std::vector<const char*> vkcfgValidationLayers = {"VK_LAYER_KHRONOS_validation"};
            bool vkcgEnableValidationLayers = true;
            int vkcfgMaxImagesInFlight = 2;
            
            bool vkcfgPreferSrgbImagePresentation = true;

        public:
            AnthemConfig();
        };


    }
}