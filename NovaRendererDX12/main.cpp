#include "DXDevice.h"
#include <iostream>

int main() {
	std::string deviceName;
	std::string driverVersion;

	Nova::DXDevice device;
	device.Create("Test", "Test", true, true);

	std::cout << "[+] Device created successfully!" << "\n";
	
	device.GetDeviceInfo(&deviceName, &driverVersion);
	
	std::cout << "\t- Device Name: " << deviceName << "\n";
	std::cout << "\t- Driver Version: " << driverVersion << "\n\n";
	
	std::cout << std::boolalpha;

	std::cout << "[+] Floating point support." << "\n";
	std::cout << "\t- Half precision support: " << device.IsFp16Supported() << "\n\n";
	
	std::cout << "[+] Ray Tracing support." << "\n";
	std::cout << "\t- Ray Tracing 1.0 support: " << device.IsRT10Supported() << "\n";
	std::cout << "\t- Ray Tracing 1.1 support: " << device.IsRT11Supported() << "\n\n";
	
	std::cout << "[+] Variable rate shading support." << "\n";
	std::cout << "\t- Tier 1 support: " << device.IsVRSTier1Supported() << "\n";
	std::cout << "\t- Tier 2 support: " << device.IsVRSTier2Supported() << "\n\n";

	std::cout << "[+] Baycentrics support: " << device.IsBarycentricsSupported() << "\n";

	device.Destroy();
	
	return 0;
}