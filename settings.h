
#ifndef _SETTINGS_H
#define _SETTINGS_H

#include "input.h"

struct Settings
{
	int resolution;
	int last_save_slot;
	bool multisave;
	bool files_extracted;
	bool show_fps;
	bool displayformat;
	
	bool enable_debug_keys;
	bool sound_enabled;
	int music_enabled;
	
	bool instant_quit;
	bool emulate_bugs;
	bool no_quake_in_hell;
	bool inhibit_fullscreen;
	
	bool skip_intro;
	int reserved[8];
	
	int input_mappings[INPUT_COUNT];
};

bool settings_load(Settings *settings=NULL);
bool settings_save(Settings *settings=NULL);

extern Settings *settings;
extern Settings normal_settings;
extern Settings replay_settings;


#endif
