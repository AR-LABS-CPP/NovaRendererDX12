#include "stdafx.h"
#include "DXDevice.h"

int main() {
	Nova::DXDevice device;
	device.Create("Test", "Test", true, true, NULL);

	return 0;
}