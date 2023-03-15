#include "device.h"

#include <iostream>

Device* Device::get_device() {
	static Device* device = new Device();
	return device;
}

Device::Device() {
	alc_device = alcOpenDevice(nullptr);
	
	if (!alc_device) 
		std::cout << "device(";
	
	alc_context = alcCreateContext(alc_device, nullptr);

	if (!alc_context)
		std::cout << "context((";
		
	if (!alcMakeContextCurrent(alc_context))
		std::cout << "curcontext((";


	const ALCchar* device_name = nullptr; 
	if (alcIsExtensionPresent(alc_device, "ALC_ENUMERATE_ALL_EXT"))
		device_name = alcGetString(alc_device, ALC_ALL_DEVICES_SPECIFIER);
	
	if (!device_name || alcGetError(alc_device) != ALC_NO_ERROR)
		device_name = alcGetString(alc_device, ALC_DEVICE_SPECIFIER);

	if (device_name) {

		std::cout << "device is open!\n";

	}
	else 
		std::cout << "device not found:(\n";
}

Device::~Device() {
	alcDestroyContext(alc_context);
}