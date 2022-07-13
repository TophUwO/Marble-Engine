#include <dwrite.h>
#include <dwritewr.h>


ULONG WINAPI WrDWrite_TextFormat_Release(IDWriteTextFormat *sTextFormat) {
	return sTextFormat->Release();
}


