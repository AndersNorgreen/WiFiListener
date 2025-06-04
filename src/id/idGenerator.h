#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <stdint.h>

class IdGenerator{
    public:
        void initId();
        uint32_t getRandomID();
        uint32_t currentId;
};

#endif