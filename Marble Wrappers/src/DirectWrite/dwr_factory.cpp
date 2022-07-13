#include <dwrite.h>
#include <dwritewr.h>


HRESULT WINAPI WrDWrite_Factory_Create(DWRITE_FACTORY_TYPE eType, IDWriteFactory **ptrpFactory) {
	return DWriteCreateFactory(eType, __uuidof(IDWriteFactory), (IUnknown **)ptrpFactory);
}

HRESULT WINAPI WrDWrite_Factory_CreateTextFormat(IDWriteFactory *sFactory, WCHAR const *wstrFontFamilyName, IDWriteFontCollection *sFontCollection, DWRITE_FONT_WEIGHT eWeight, DWRITE_FONT_STYLE eStyle, DWRITE_FONT_STRETCH eStretchStyle, FLOAT fSize, WCHAR const *wstrLocaleName, IDWriteTextFormat **ptrpTextFormat) {
	return sFactory->CreateTextFormat(wstrFontFamilyName, sFontCollection, eWeight, eStyle, eStretchStyle, fSize, wstrLocaleName, ptrpTextFormat);
}

ULONG WINAPI WrDWrite_Factory_Release(IDWriteFactory *sFactory) {
	return sFactory->Release();
}


