#pragma region Main
#include <obs-module.h>
#include <plugin-support.h>
#include "../include/utils/utils.h"

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

// Define your enum with a macro for string mapping
#define ENUM_START_BEHAVIOR_LIST \
	X(RESTART)               \
	X(UNPAUSE)

#define X(name) name,
enum { ENUM_START_BEHAVIOR_LIST } StartBehavior;
#undef X

// Create a lookup table using macros
#define X(name) #name,
const char *StartBehaviorName[] = {ENUM_START_BEHAVIOR_LIST};
#undef X

#define ENUM_END_BEHAVIOR_LIST \
	X(STOP)                \
	X(LOOP)                \
	X(LOOP_SPECIFIC_MEDIA) \
	X(LOOP_LAST_MEDIA)

#define X(name) name,
enum { ENUM_END_BEHAVIOR_LIST } EndBehavior;
#undef X

// Create a lookup table using macros
#define X(name) #name,
const char *EndBehaviorName[] = {ENUM_END_BEHAVIOR_LIST};
#undef X

struct PlaylistSource {
	obs_source_t *source;
	enum StartBehavior playlist_start_behavior;
	enum EndBehavior playlist_end_behavior;
	MediaFileDataArray *all_media;
	MediaFileData *current_media;
	size_t current_media_index;
	bool debug;
};

const char *playlist_source_name(void *data);

void *playlist_source_create(obs_data_t *settings, obs_source_t *source);

void playlist_source_destroy(void *data);

uint32_t playlist_source_width(void *data);

uint32_t playlist_source_height(void *data);

void playlist_get_defaults(obs_data_t *settings);

obs_properties_t *playlist_get_properties(void *data);

void playlist_update(void *data, obs_data_t *settings);

void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings);

void playlist_activate(void *data);

void playlist_deactivate(void *data);

void playlist_tick(void *data, float seconds);

static struct obs_source_info playlist_source_template = {
	.id = "media_playlist_source_codeyan",
	.type = OBS_SOURCE_TYPE_INPUT,
	.get_name = playlist_source_name,
	.create = playlist_source_create,
	.destroy = playlist_source_height,
	.get_width = playlist_source_width,
	.get_height = playlist_source_height,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW | OBS_SOURCE_AUDIO | OBS_SOURCE_CONTROLLABLE_MEDIA,
	.get_defaults = playlist_get_defaults,
	.get_properties = playlist_get_properties,
	.update = playlist_update,
	.video_tick = playlist_tick,
	.icon_type = OBS_ICON_TYPE_MEDIA,
};
#pragma endregion