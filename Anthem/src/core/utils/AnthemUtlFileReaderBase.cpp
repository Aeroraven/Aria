#include "../../../include/core/utils/AnthemUtlFileReaderBase.h"

namespace Anthem::Core::Util{
    bool  AnthemUtlFileReaderBase::readFile(const std::string& filename, std::vector<char>* outBuffer){
        std::ifstream ifs(filename,std::ios::ate | std::ios::binary);
        ANTH_ASSERT(ifs.is_open(),"Failed to open file");
        size_t fileSize = (size_t)ifs.tellg();
        outBuffer->resize(fileSize);
        ifs.seekg(0);
        ifs.read(outBuffer->data(),fileSize);
        ifs.close();
        return true;
    }
}