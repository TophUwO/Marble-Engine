#include <entrypoint.h>


struct layer1_userdata {
	int m_selfid;
} s_userdata;


void MB_CALLBACK marble_callback_submitsettings(char const *pz_cmdline, struct marble_app_settings *ps_settings) {
	*ps_settings = (struct marble_app_settings){
		.mpz_title  = "Marble Engine Sandbox",
		.m_width    = 48,
		.m_height   = 32,
		.m_tilesize = 24
	};
}

static void MB_CALLBACK marble_layer1_oncreate(int layerid, void *p_userdata) {
	MB_LOG_INFO("Create user-layer.");
}

static void MB_CALLBACK marble_layer1_onupdate(int layerid, float frametime, void *p_userdata) {
	// TODO: any updates that have to be carried out.
}

marble_ecode_t MB_CALLBACK marble_callback_userinit(char const *pz_cmdline) {
	struct marble_layer_cbs s_callbacks = {
		.cb_oncreate = &marble_layer1_oncreate,
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


