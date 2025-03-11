#include <obs-module.h>

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

static uint32_t playlist_source_width(void *data)
{
	return 1920;
}

static uint32_t playlist_source_height(void *data)
{
	return 1080;
}

struct playlist_source {
	bool loop;
};
struct playlist_source *playlist = {0};

static void *playlist_source_create(obs_data_t *settings, obs_source_t *source);

static void playlist_source_destroy(void *data);

static const char *playlist_source_name(void *data);

static obs_properties_t *playlist_get_properties(void *data);

static void playlist_get_defaults(obs_data_t *settings);

struct obs_source_info playlist_source_info;