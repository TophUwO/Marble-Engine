#include <editor.h>


int WINAPI WinMain(
	_In_     HINSTANCE p_hinst,
	_In_opt_ HINSTANCE p_hprevinst,
	_In_     LPSTR pz_cmdline,
	_In_     int showcmd
) {
	marble_ecode_t ecode = mbeditor_init(p_hinst, pz_cmdline);
	if (ecode != MARBLE_EC_OK)
		return ecode;

	return mbeditor_run();
}


