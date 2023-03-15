#pragma once
#include <vector>
#include "AL/al.h"

class Buffer {
public:
    static Buffer* get_buffer();

    ALuint add_sound(const char* filename);
    bool remove_sound(const ALuint& buffer);
    
private:
    Buffer();
    ~Buffer();

    std::vector<ALuint> sound_buffers;
};