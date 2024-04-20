#pragma once
#include <vector>

namespace Anthem{
    namespace Core{
        class AnthemConfig {
        public:
            const char* appName = "Anthem";
            const char* demoName = "Untitled";
            const char* engineName = "No Engine?";
            
            int appcfgResolutionWidth = 1920;
            int appcfgResolutionHeight = 1080;

            std::vector<const char*> vkcfgValidationLayers = {"VK_LAYER_KHRONOS_validation"};
            bool vkcfgEnableValidationLayers = true;
            int vkcfgMaxImagesInFlight = 2;
            
            bool vkcfgPreferSrgbImagePresentation = true;

        public:
            AnthemConfig();
        };


    }
}