#pragma once
#include "AL/alc.h"


class Device {
public:
	static Device* get_device();
private:
	Device();
	~Device();

	ALCdevice* alc_device;
	ALCcontext* alc_context; 
};