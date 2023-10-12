#include "../base/AnthemPhyDevice.h"

namespace Anthem::Core::Util{
    class AnthemUtlPhyDeviceReqBase{
    protected:
        const AnthemPhyDevice* phyDevice = nullptr;
    public:
        bool virtual specifyPhyDevice(const AnthemPhyDevice* device);
        const AnthemPhyDevice* getPhyDevice() const;
    };
}