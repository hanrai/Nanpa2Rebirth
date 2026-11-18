#pragma once

#include <cstdint>

struct WaveFileHeader
{
    char id[4];
    std::int32_t totallength;
    char wavefmt[8];
    std::int32_t format;
    std::int16_t pcm;
    std::int16_t channelCount;
    std::int32_t sampleRate;
    std::int32_t bytes_per_second;
    std::int16_t bytes_by_capture;
    std::int16_t bits_per_sample;
    char data[4];
    std::int32_t bytes_in_data;
};
