#pragma once
#include <vector>

namespace Anthem{
    namespace Core{
        class AnthemConfig {
        public:
            const char* APP_NAME = "Anthem";
            const char* ENGINE_NAME = "No Engine?";
            
            int APP_RESLOUTION_W = 800;
            int APP_RESLOUTION_H = 600;

            std::vector<const char*> VKCFG_VALIDATION_LAYERS = {"VK_LAYER_KHRONOS_validation"};
            bool VKCFG_ENABLE_VALIDATION_LAYERS = true;
            int VKCFG_MAX_IMAGES_IN_FLIGHT = 2;

        public:
            AnthemConfig();
        };


    }
}