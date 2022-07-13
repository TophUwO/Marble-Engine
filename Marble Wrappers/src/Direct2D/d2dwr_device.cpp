#include <d2dwr.h>


HRESULT WINAPI D2DWr_Device_CreateDeviceContext(ID2D1Device *sDevice, D2D1_DEVICE_CONTEXT_OPTIONS eOptions, ID2D1DeviceContext **ptrpDeviceContext) {
	return sDevice->CreateDeviceContext(eOptions, ptrpDeviceContext);
}

ULONG WINAPI D2DWr_Device_Release(ID2D1Device *sDevice) {
	return sDevice->Release();
}


