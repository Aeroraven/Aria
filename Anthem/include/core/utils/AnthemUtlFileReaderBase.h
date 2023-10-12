#pragma once
#include "../base/AnthemBaseImports.h"
namespace Anthem::Core::Util{
    class AnthemUtlFileReaderBase{
    public:
        bool virtual readFile(const std::string& filename,std::vector<char>* outBuffer);
    };
}