#pragma once
namespace Anthem{
    namespace Core{
        class AnthemEnv {
        public:
            bool virtual createWindow(int w,int h) = 0;
            void virtual drawLoop() = 0;
            void virtual createInstance() = 0;
            void virtual destroyEnv() = 0;
            void virtual run() = 0;
        };
    }
}
