#pragma once
#include <vector>

namespace Anthem{
    namespace Core{
        class AnthemConfig {
        public:
            const char* APP_NAME;
            const char* ENGINE_NAME;
            
            int APP_RESLOUTION_W;
            int APP_RESLOUTION_H;

            std::vector<const char*> VKCFG_VALIDATION_LAYERS = {
                "VK_LAYER_KHRONOS_validation"
            };

            bool VKCFG_ENABLE_VALIDATION_LAYERS = true;

        public:
            AnthemConfig();
        };


    }
}