#pragma once
#include "AnthemDefs.h"
#include <iostream>
#include <ctime>
#include <cstring>

#define ANTH_LOGGER_RED     "\033[31m"     
#define ANTH_LOGGER_GREEN   "\033[32m"     
#define ANTH_LOGGER_YELLOW  "\033[33m"     
#define ANTH_LOGGER_RESET   "\033[0m"
#define ANTH_LOGGER_CYAN    "\033[36m"      
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

            inline std::string className(const std::string& prettyFunction){
                // Adapted from: https://stackoverflow.com/a/15775519
                size_t colons = prettyFunction.find("(");
                if (colons == std::string::npos)
                    return "::";
                size_t begin = prettyFunction.substr(0,colons).rfind(" ")+1;  
                size_t end = colons - begin;

                return prettyFunction.substr(begin,end);
            }


            void log_prefix(const char* level){
                auto timestamp = std::time(nullptr);
                auto formattedTime = std::asctime(std::localtime(&timestamp));
                formattedTime[strlen(formattedTime)-1] = '\0';
                std::cout << ANTH_LOGGER_CYAN << "["  << level << "] "<< ANTH_LOGGER_GREEN <<"(" << formattedTime << ") " << ANTH_LOGGER_RESET;
            }
 

            template<typename... _Args>
            void log_wrapper(const char* func, const char* level, std::function<void(std::tuple<_Args...>)> wrapper,std::tuple<_Args...> args){
                this->log_prefix(level);
                std::cout << ANTH_LOGGER_RED << "<" << func << ">"<<ANTH_LOGGER_RESET<<" :";
                wrapper(args);
            }

            template<typename... _Args>
            void log_content(std::tuple<_Args...> args){
                std::apply([](auto&&... args) {((std::cout << args << " "), ...)<<std::endl;}, args);
            }

            template<typename... _Args>
            void logi2(const char* func, std::tuple<_Args...> args){
                std::function<void(std::tuple<_Args...>)> f = std::bind(&ANTH_CLASSTP::log_content<_Args...>, this, std::placeholders::_1);
                this->log_wrapper(func,"INFO",f,args);
            }
            template<typename... _Args>
            void logw2(const char* func, std::tuple<_Args...> args){
                std::function<void(std::tuple<_Args...>)> f = std::bind(&ANTH_CLASSTP::log_content<_Args...>, this, std::placeholders::_1);
                this->log_wrapper(func,"WARN",f,args);
            }
            template<typename... _Args>
            void loge2(const char* func, std::tuple<_Args...> args){
                std::function<void(std::tuple<_Args...>)> f = std::bind(&ANTH_CLASSTP::log_content<_Args...>, this, std::placeholders::_1);
                this->log_wrapper(func,"ERROR",f,args);
                exit(1);
            }
            template<typename... _Args>
            void log_assert2(const char* func, bool cond, std::tuple<_Args...> args){
                if(!cond){
                    std::function<void(std::tuple<_Args...>)> f = std::bind(&ANTH_CLASSTP::log_content<_Args...>, this, std::placeholders::_1);
                    this->log_wrapper(func,"ASSERT",f,args);
                    exit(1);
                }
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
            std::optional<T> assert_fallback(bool condition, T (*p)() , Args... args){
                if(!condition){
                    this->log_prefix("ASSERT");
                    (std::cout << ... << args) << std::endl;
                    return (*p)();
                }
                return std::nullopt;
            }
        };
    }
}