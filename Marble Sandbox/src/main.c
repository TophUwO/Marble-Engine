#include <entrypoint.h>


struct layer1_userdata {
	int m_selfid;
} s_userdata;


int MB_CALLBACK marble_callback_submitsettings(char const *pz_cmdline, struct marble_app_settings *ps_settings) {
	*ps_settings = (struct marble_app_settings){
		.m_xpos     = MB_DEFAULT,
		.m_ypos     = MB_DEFAULT,

		.m_width    = 32,
		.m_height   = 32,
		.m_tilesize = 32
	};

	return 0;
}

int MB_CALLBACK marble_layer1_onpush(int layerid, void *p_userdata) {

	return 0;
}

int MB_CALLBACK marble_layer1_onupdate(int layerid, float frametime, void *p_userdata) {
	// TODO: any updates that have to be carried out.

	return 0;
}

int MB_CALLBACK marble_callback_userinit(char const *pz_cmdline) {
	struct marble_layer_callbacks s_callbacks = {
		.cb_onpush   = &marble_layer1_onpush,
		.cb_onupdate = &marble_layer1_onupdate
	};

	return marble_application_createlayer(
		true,
		false,
		&s_callbacks,
		&s_userdata,
		"game_layer",
		&s_userdata.m_selfid
	);
}


