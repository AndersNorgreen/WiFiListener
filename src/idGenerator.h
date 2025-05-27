#ifndef IDGENERATOR_H
#define IDGENERATOR_H

#include <stdint.h>

class IdGenerator{
    public:
        void initId();
        void checkId();
        uint32_t getRandomID();
        uint32_t currentId;
};

#endif