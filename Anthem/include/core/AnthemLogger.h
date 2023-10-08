#pragma once
#include "AnthemDefs.h"
#include <iostream>
#include <ctime>
#include <cstring>

namespace Anthem{
    namespace Core{
        class AnthemLogger {
        private:
            AnthemLogger();
        public:
            static auto getInstance(){
                static AnthemLogger instance;
                return instance;
            }
            void log_prefix(const char* level){
                auto timestamp = std::time(nullptr);
                auto formattedTime = std::asctime(std::localtime(&timestamp));
                formattedTime[strlen(formattedTime)-1] = '\0';
                std::cout << "[" << level << "] (" << formattedTime << ") ";
            }

            template<typename... Args>
            void logi(Args... args){
                this->log_prefix("INFO");
                (std::cout << ... << args) << std::endl;
            }

            template<typename... Args>
            void logw(Args... args){
                this->log_prefix("WARN");
                (std::cout <<  ... << args) << std::endl;
            }

            template<typename Tt, typename Tf>
            void logiif(bool condition, Tt trueMsg, Tf falseMsg){
                this->log_prefix("INFO");
                if(condition){
                    std::cout <<  trueMsg << std::endl;
                }else{
                    std::cout << falseMsg << std::endl;
                }
            }

            template<typename... Args>
            void assert(bool condition, Args... args){
                if(!condition){
                    this->log_prefix("ASSERT");
                    (std::cout << ... << args) << std::endl;
                    exit(1);
                }
            }
            template<typename T, typename... Args>
            void assert_fallback(bool condition, T (*p)() , Args... args){
                if(!condition){
                    this->log_prefix("ASSERT");
                    (std::cout << ... << args) << std::endl;
                    (*p)();
                }
            }
        };
    }
}