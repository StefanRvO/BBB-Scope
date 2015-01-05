#pragma once
#include <cstdint>
struct ADCOptions
{
    uint64_t sampleTimeMax=1000000;
    uint64_t sampleTimeMin=6290;
    uint64_t sampleTime=30000;
};
