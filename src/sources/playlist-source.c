#pragma region Main
#include "../include/sources/playlist-source.h"

#define S_FFMPEG_RESTART_PLAYBACK_ON_ACTIVE "restart_on_activate"
#define S_FFMPEG_LOCAL_FILE "local_file"
#define S_FFMPEG_INPUT "input"
#define S_FFMPEG_IS_LOCAL_FILE "is_local_file"
#define S_CURRENT_MEDIA_INDEX "current_media_index"

#pragma region Media Functions
void play_video(struct PlaylistSource *playlist_data, size_t index)
{
	if (!playlist_data || !playlist_data->all_media || !playlist_data->media_source ||
	    !playlist_data->media_source_settings)
		return;

	if (index >= playlist_data->all_media->size)
		return;

	// Get video file path from the array
	const MediaFileData *media_data = get_media(playlist_data->all_media, index);

	if (!media_data)
		return;

	// Set file path in ffmpeg_source
	obs_data_set_string(playlist_data->media_source_settings, S_FFMPEG_LOCAL_FILE, media_data->path);

	// obs_log(LOG_INFO, " PRINTING: \n%s", obs_data_get_json_pretty(playlist_data->media_source_settings));

	obs_source_update(playlist_data->media_source, playlist_data->media_source_settings);

	// Set it as active
	obs_source_media_play_pause(playlist_data->source, false);
}

void playlist_audio_callback(void *data, obs_source_t *source, const struct audio_data *audio_data, bool muted)
{
	UNUSED_PARAMETER(muted);
	UNUSED_PARAMETER(source);
	struct PlaylistSource *playlist_data = data;
	pthread_mutex_lock(&playlist_data->audio_mutex);
	size_t size = audio_data->frames * sizeof(float);
	for (size_t i = 0; i < playlist_data->num_channels; i++) {
		deque_push_back(&playlist_data->audio_data[i], audio_data->data[i], size);
	}
	deque_push_back(&playlist_data->audio_frames, &audio_data->frames, sizeof(audio_data->frames));
	deque_push_back(&playlist_data->audio_timestamps, &audio_data->timestamp, sizeof(audio_data->timestamp));
	pthread_mutex_unlock(&playlist_data->audio_mutex);
}

#pragma endregion

const char *playlist_source_name(void *data)
{
	return "Playlist"; // This should match the filename (without extension) in data/
}

void *playlist_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct PlaylistSource *playlist_data = bzalloc(sizeof(*playlist_data));

	playlist_data->source = source;
	playlist_data->media_source = NULL;
	playlist_data->media_source_settings = NULL;

	playlist_data->all_media = NULL;

	playlist_data->start_index = 0;
	playlist_data->end_index = 0;

	// playlist_data->current_media = NULL;
	playlist_data->current_media_index = 0;
	playlist_data->loop_index = 0;
	playlist_data->infinite = true;
	playlist_data->loop_count = 0;

	playlist_data->run = false;
	playlist_data->paused = false;

	pthread_mutex_init_value(&playlist_data->mutex);
	if (pthread_mutex_init(&playlist_data->mutex, NULL) != 0)
		goto error;

	pthread_mutex_init_value(&playlist_data->audio_mutex);
	if (pthread_mutex_init(&playlist_data->audio_mutex, NULL) != 0)
		goto error;

	update_playlist_data(playlist_data, settings);

	return playlist_data;
error:
	playlist_source_destroy(playlist_data);
	return NULL;
}

void playlist_source_destroy(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_release(playlist_data->media_source);
	}
	if (playlist_data->media_source_settings != NULL) {
		obs_data_release(playlist_data->media_source_settings);
	}

	for (size_t i = 0; i < MAX_AUDIO_CHANNELS; i++) {
		deque_free(&playlist_data->audio_data[i]);
	}
	deque_free(&playlist_data->audio_frames);
	deque_free(&playlist_data->audio_timestamps);
	pthread_mutex_destroy(&playlist_data->mutex);
	pthread_mutex_destroy(&playlist_data->audio_mutex);

	free_media_array(playlist_data->all_media);

	// if (playlist_data->current_media != NULL) {
	// 	free(playlist_data->current_media);
	// }

	bfree(playlist_data);
	obs_source_release(playlist_data->source);
}

uint32_t playlist_source_width(void *data)
{
	return 1920;
}

uint32_t playlist_source_height(void *data)
{
	return 1080;
}

void playlist_get_defaults(obs_data_t *settings)
{
	// obs_data_set_default_int(settings, "start_index", 0);
	// obs_data_set_default_int(settings, "end_index", 0);
	// obs_data_set_default_int(settings, "playlist_start_behavior", 0);
	// obs_data_set_default_int(settings, "playlist_end_behavior", 0);
	// obs_data_set_default_int(settings, "loop_index", 0);
	obs_data_set_default_bool(settings, "infinite", true);
	// obs_data_set_default_int(settings, "loop_count", 0);
	obs_data_set_default_bool(settings, "debug", false);
}

obs_properties_t *make_playlist_properties(struct PlaylistSource *playlist_data)
{
	obs_properties_t *props = obs_properties_create();

	int all_media_size = 0;

	if (playlist_data->all_media != NULL) {
		all_media_size = (int)playlist_data->all_media->size;
	}

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	obs_properties_add_int_slider(props, "start_index", "Start Index", 0, last_index, 1);

	obs_properties_add_int_slider(props, "end_index", "End Index", 0, last_index, 1);

	obs_properties_add_editable_list(props, "playlist", "Playlist", OBS_EDITABLE_LIST_TYPE_FILES_AND_URLS,
					 media_filter, "");

	obs_property_t *psb_property = obs_properties_add_list(
		props, "playlist_start_behavior", "Playlist Start Behavior", OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	long long i = 0;
	const char *name = StartBehavior[i];
	while (name != "") {
		obs_property_list_add_int(psb_property, name, i);
		i++;
		name = StartBehavior[i];
	}

	obs_property_t *peb_property = obs_properties_add_list(props, "playlist_end_behavior", "Playlist End Behavior",
							       OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_INT);

	i = 0;
	name = EndBehavior[i];
	while (name != "") {
		obs_property_list_add_int(peb_property, name, i);
		i++;
		name = EndBehavior[i];
	}

	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX) {
		obs_properties_add_int_slider(props, "loop_index", "Loop Index", 0, last_index, 1);
	}
	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX ||
	    playlist_data->playlist_end_behavior == LOOP_AT_END) {
		obs_properties_add_bool(props, "infinite", "infinite");
		if (playlist_data->infinite == false) {
			obs_properties_add_int(props, "loop_count", "Loop Count", 0, INT_MAX, 1);
		}
	}

	obs_properties_add_bool(props, "debug", "Debug");
	return props;
}

obs_properties_t *playlist_get_properties(void *data)
{
	return make_playlist_properties(data);
}

/**
 * @brief Updates the playlist data.
 * @param settings The settings of the playlist source.
 * @return void
 */
void update_playlist_data(struct PlaylistSource *playlist_data, obs_data_t *settings)
{
	bool update_properties = false;
	// int previous_start_index = playlist_data->start_index;
	int previous_end_index = playlist_data->end_index;

	playlist_data->start_index = (int)obs_data_get_int(settings, "start_index");
	playlist_data->end_index = (int)obs_data_get_int(settings, "end_index");

	playlist_data->debug = obs_data_get_bool(settings, "debug");
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "Debug: %s", playlist_data->debug ? "true" : "false");
	}

	playlist_data->playlist_start_behavior = obs_data_get_int(settings, "playlist_start_behavior");
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "Start Behavior: %s", StartBehavior[playlist_data->playlist_start_behavior]);
	}

	enum EndBehavior playlist_end_behavior = obs_data_get_int(settings, "playlist_end_behavior");
	if (playlist_data->playlist_end_behavior != playlist_end_behavior) {
		update_properties = true;
	}

	playlist_data->playlist_end_behavior = playlist_end_behavior;
	if (playlist_data->debug == true) {
		obs_log(LOG_INFO, "end Behavior: %s", EndBehavior[playlist_data->playlist_end_behavior]);
	}

	bool previous_size_initialized = false;
	size_t previous_size = 0;
	// obs_log(LOG_INFO, "Sizes: %d", previous_size);
	if (playlist_data->all_media != NULL && playlist_data->all_media->size > 0) {
		previous_size = playlist_data->all_media->size;
		previous_size_initialized = true;
		free_media_array(playlist_data->all_media);
	}

	playlist_data->all_media = obs_data_array_retain(obs_data_get_array(settings, "playlist"));
	int all_media_size = 0;

	if (playlist_data->all_media != NULL) {
		all_media_size = (int)playlist_data->all_media->size;
	}

	// obs_log(LOG_INFO, "Sizes: %d, %s", previous_size, playlist_data->all_media->size);

	bool update_start_index_ui = false;
	bool update_end_index_ui = false;

	int last_index = all_media_size - 1;

	if (last_index < 0) {
		last_index += 1;
	}

	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX ||
	    playlist_data->playlist_end_behavior == LOOP_AT_END) {
		bool infinite = obs_data_get_bool(settings, "infinite");
		if (infinite != playlist_data->infinite) {
			update_properties = true;
		}
		playlist_data->infinite = infinite;
	}
	if (playlist_data->playlist_end_behavior == LOOP_AT_INDEX) {
		bool update_loop_index_ui = false;

		int loop_index = (int)obs_data_get_int(settings, "loop_index");
		int loop_count = (int)obs_data_get_int(settings, "loop_count");

		if (playlist_data->debug == true) {
			obs_log(LOG_INFO, "Infinite New Value: %s", loop_index);
			obs_log(LOG_INFO, "Infinite New Value: %s", playlist_data->infinite == true ? "true" : "false");
			obs_log(LOG_INFO, "Infinite New Value: %s", loop_count);
		}

		playlist_data->loop_index = loop_index;
		playlist_data->loop_count = loop_count;

		if (playlist_data->loop_index < 0) {
			playlist_data->loop_index = playlist_data->start_index;
			update_loop_index_ui = true;
			update_properties = true;
		} else if (playlist_data->end_index >= all_media_size) {
			playlist_data->loop_index = last_index;
			update_loop_index_ui = true;
			update_properties = true;
		}

		if (update_loop_index_ui) {
			obs_data_set_int(settings, "loop_index", playlist_data->loop_index);
		}
	}

	if (previous_size_initialized == true) {
		if (playlist_data->all_media->size != 0 && previous_size != playlist_data->all_media->size) {
			update_properties = true;
			if (playlist_data->end_index == previous_size - 1) {
				playlist_data->end_index = last_index;
				update_end_index_ui = true;
			}
		}
	}

	if (playlist_data->all_media != NULL && playlist_data->debug) {
		obs_log_media_array(LOG_INFO, playlist_data->all_media, 90, "    ");
	}

	if (playlist_data->start_index > previous_end_index) {
		playlist_data->start_index = previous_end_index;
		update_start_index_ui = true;
		update_properties = true;
	}

	if (playlist_data->end_index < playlist_data->start_index) {
		playlist_data->end_index = playlist_data->start_index;
		update_end_index_ui = true;
		update_properties = true;
	} else if (playlist_data->end_index >= all_media_size) {
		playlist_data->end_index = last_index;
		update_end_index_ui = true;
		update_properties = true;
	}

	if (update_start_index_ui) {
		obs_data_set_int(settings, "start_index", playlist_data->start_index);
	}

	if (update_end_index_ui) {
		obs_data_set_int(settings, "end_index", playlist_data->end_index);
	}

	if (update_properties == true) {
		obs_source_update_properties(playlist_data->source);
	}
}

void playlist_update(void *data, obs_data_t *settings)
{
	struct PlaylistSource *playlist_data = data;
	update_playlist_data(playlist_data, settings);

	// Ensure properties are refreshed or updated if necessary
	// obs_properties_t *props = make_playlist_properties(playlist_data);
	// obs_source_update(playlist_data->source, props);
}

void playlist_activate(void *data)
{
	obs_log(LOG_INFO, "playlist_activate");
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source == NULL) {
		playlist_data->media_source_settings = obs_data_create();

		// for (size_t i = 0; i < 61; i++) {
		// 	const char *id = "";
		// 	obs_enum_source_types(i, &id);
		// 	blog(LOG_INFO, "This is an id: %s", id);
		// }

		// const char *video_path =
		// 	"C:/Users/aamax/OneDrive/Documents/OBSSceneVids/Start Of Purple Pink Orange Arcade Pixel Just Chatting Twitch Screen.mp4"; // Replace with your actual video path
		// obs_data_set_string(playlist_data->media_source_settings, S_FFMPEG_LOCAL_FILE, video_path);

		obs_data_set_bool(playlist_data->media_source_settings, "log_changes", true);
		playlist_data->media_source = obs_source_create_private("ffmpeg_source", "Video Source",
									playlist_data->media_source_settings);

		obs_source_add_active_child(playlist_data->source, playlist_data->media_source);
		obs_source_add_audio_capture_callback(playlist_data->media_source, playlist_audio_callback,
						      playlist_data);

		playlist_data->run = true;

		switch (playlist_data->playlist_start_behavior) {
		case RESTART:
			playlist_data->current_media_index = 0;
			// playlist_data->current_media =
			// 	get_media(playlist_data->all_media, playlist_data->current_media_index);
			// playlist_data->current_media_source;
			play_video(playlist_data, playlist_data->current_media_index);
			break;
		case UNPAUSE:
			/* code */
			break;
		case PAUSE:
			/* code */
			break;
		default:
			break;
		}
	}
}

void playlist_deactivate(void *data)
{
	obs_log(LOG_INFO, "playlist_deactivate");
	// struct PlaylistSource *playlist_data = data;
}

void playlist_video_tick(void *data, float seconds)
{
	// struct PlaylistSource *playlist_data = data;
	// //UNUSED_PARAMETER(data);
	// UNUSED_PARAMETER(seconds);

	// const audio_t *a = obs_get_audio();
	// const struct audio_output_info *aoi = audio_output_get_info(a);
	// pthread_mutex_lock(&playlist_data->audio_mutex);
	// while (playlist_data->audio_frames.size > 0) {
	// 	struct obs_source_audio audio;
	// 	audio.format = aoi->format;
	// 	audio.samples_per_sec = aoi->samples_per_sec;
	// 	audio.speakers = aoi->speakers;
	// 	deque_pop_front(&playlist_data->audio_frames, &audio.frames, sizeof(audio.frames));
	// 	deque_pop_front(&playlist_data->audio_timestamps, &audio.timestamp, sizeof(audio.timestamp));
	// 	for (size_t i = 0; i < playlist_data->num_channels; i++) {
	// 		audio.data[i] =
	// 			(uint8_t *)playlist_data->audio_data[i].data + playlist_data->audio_data[i].start_pos;
	// 	}
	// 	obs_source_output_audio(playlist_data->source, &audio);
	// 	for (size_t i = 0; i < playlist_data->num_channels; i++) {
	// 		deque_pop_front(&playlist_data->audio_data[i], NULL, audio.frames * sizeof(float));
	// 	}
	// }
	// playlist_data->num_channels = audio_output_get_channels(a);
	// pthread_mutex_unlock(&playlist_data->audio_mutex);
}

void playlist_video_render(void *data, gs_effect_t *effect)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_video_render(playlist_data->media_source);
	} else {
		obs_source_video_render(NULL);
	}

	UNUSED_PARAMETER(effect);
	// obs_log(LOG_INFO, "video_render");
}

bool playlist_audio_render(void *data, uint64_t *ts_out, struct obs_source_audio_mix *audio_output, uint32_t mixers,
			   size_t channels, size_t sample_rate)
{
	// struct PlaylistSource *playlist_data = data;
	// if (!playlist_data->media_source)
	// 	return false;

	// struct obs_source_audio_mix child_audio;
	// uint64_t source_ts;

	// /*if (obs_source_audio_pending(mps->current_media_source))
	// 	return false;*/

	// source_ts = obs_source_get_audio_timestamp(playlist_data->media_source);
	// if (!source_ts)
	// 	return false;

	// obs_source_get_audio_mix(playlist_data->media_source, &child_audio);
	// for (size_t mix = 0; mix < MAX_AUDIO_MIXES; mix++) {
	// 	if ((mixers & (1 << mix)) == 0)
	// 		continue;

	// 	for (size_t ch = 0; ch < channels; ch++) {
	// 		float *out = audio_output->output[mix].data[ch];
	// 		float *in = child_audio.output[mix].data[ch];

	// 		memcpy(out, in, AUDIO_OUTPUT_FRAMES * MAX_AUDIO_CHANNELS * sizeof(float));
	// 	}
	// }

	// *ts_out = source_ts;

	// UNUSED_PARAMETER(sample_rate);
	// return true;
	return false;
}
void playlist_save(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_save");
	// struct PlaylistSource *playlist_data = data;
	// obs_data_set_int(settings, S_CURRENT_MEDIA_INDEX, playlist_data->current_media_index);
	// update_current_filename_setting(playlist_data, settings);
}

void playlist_load(void *data, obs_data_t *settings)
{
	obs_log(LOG_INFO, "playlist_load");
}

void media_play_pause(void *data, bool pause)
{
	obs_log(LOG_INFO, "We be playing the video");
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_play_pause(playlist_data->media_source, pause);
	}
	playlist_data->paused = pause;
}

void media_restart(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_restart(playlist_data->media_source);
	}
}

void media_stop(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_stop(playlist_data->media_source);
	}
}

void media_next(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_next(playlist_data->media_source);
	}
}

void media_previous(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_previous(playlist_data->media_source);
	}
}

int64_t media_get_duration(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		return obs_source_media_get_duration(playlist_data->media_source);
	}
	return 0;
}

int64_t media_get_time(void *data)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		return obs_source_media_get_time(playlist_data->media_source);
	}
	return 0;
}

void media_set_time(void *data, int64_t miliseconds)
{
	struct PlaylistSource *playlist_data = data;

	if (playlist_data->media_source != NULL) {
		obs_source_media_set_time(playlist_data->media_source, miliseconds);
	}
}

enum obs_media_state media_get_state(void *data)
{
	struct PlaylistSource *playlist_data = data;

	enum obs_media_state media_state = OBS_MEDIA_STATE_NONE;

	if (playlist_data->all_media != NULL && playlist_data->all_media->size > 0 &&
	    playlist_data->media_source != NULL) {
		media_state = obs_source_media_get_state(playlist_data->media_source);
	}

	return media_state;
}

#pragma endregion
/*
obs_encoder_t *obs_audio_encoder_create(const char *id, const char *name, obs_data_t *settings, size_t mixer_idx, obs_data_t *hotkey_data);
bool obs_audio_monitoring_available(void);
audio_t *obs_get_audio(void);
bool obs_get_audio_info(struct obs_audio_info *oai);
void obs_get_audio_monitoring_device(const char **name, const char **id);
bool obs_set_audio_monitoring_device(const char *name, const char *id);
void obs_enum_audio_monitoring_devices(obs_enum_audio_device_cb cb, void *data);
bool obs_reset_audio2(const struct obs_audio_info2 *oai);
bool obs_reset_audio(const struct obs_audio_info *oai);
void obs_reset_audio_monitoring(void);
audio_t *obs_output_audio(const obs_output_t *output);
bool obs_source_audio_active(const obs_source_t *source);
bool obs_source_audio_pending(const obs_source_t *source);
audio_t *obs_encoder_audio(const obs_encoder_t *encoder);
void obs_add_raw_audio_callback(size_t mix_idx, const struct audio_convert_info *conversion, audio_output_callback_t callback, void *param);
obs_encoder_t *obs_output_get_audio_encoder(const obs_output_t *output, size_t idx);
void obs_output_set_audio_conversion(obs_output_t *output, const struct audio_convert_info *conversion);
void obs_output_set_audio_encoder(obs_output_t *output, obs_encoder_t *encoder, size_t idx);
void obs_remove_raw_audio_callback(size_t mix_idx, audio_output_callback_t callback, void *param);
void obs_source_add_audio_capture_callback(obs_source_t *source, obs_source_audio_capture_t callback, void *param);
void obs_source_add_audio_pause_callback(obs_source_t *source, signal_callback_t callback, void *param);
void obs_source_get_audio_mix(const obs_source_t *source, struct obs_source_audio_mix *audio);
uint32_t obs_source_get_audio_mixers(const obs_source_t *source);
uint64_t obs_source_get_audio_timestamp(const obs_source_t *source);
void obs_source_set_audio_active(obs_source_t *source, bool show);
void obs_source_set_audio_mixers(obs_source_t *source, uint32_t mixers);
bool obs_transition_audio_render(obs_source_t *transition, uint64_t *ts_out, struct obs_source_audio_mix *audio, uint32_t mixers, size_t channels, size_t sample_rate, obs_transition_audio_mix_callback_t mix_a_callback, obs_transition_audio_mix_callback_t mix_b_callback);
void obs_encoder_set_audio(obs_encoder_t *encoder, audio_t *audio);
void obs_hotkeys_set_audio_hotkeys_translations(const char *mute, const char *unmute, const char *push_to_mute, const char *push_to_talk);
void obs_source_output_audio(obs_source_t *source, const struct obs_source_audio *audio);
void obs_source_remove_audio_capture_callback(obs_source_t *source, obs_source_audio_capture_t callback, void *param);
void obs_source_remove_audio_pause_callback(obs_source_t *source, signal_callback_t callback, void *param);
const char *obs_get_output_supported_audio_codecs(const char *id);
const char *obs_output_get_supported_audio_codecs(const obs_output_t *output);
const char **obs_service_get_supported_audio_codecs(const obs_service_t *service);
*/

/*
scene
group
audio_line
image_source
color_source
color_source_v2
color_source_v3
slideshow
slideshow_v2
browser_source
ffmpeg_source
mask_filter
mask_filter_v2
crop_filter
gain_filter
basic_eq_filter
hdr_tonemap_filter
color_filter
color_filter_v2
scale_filter
scroll_filter
gpu_delay
color_key_filter
color_key_filter_v2
clut_filter
sharpness_filter
sharpness_filter_v2
chroma_key_filter
chroma_key_filter_v2
async_delay_filter
noise_suppress_filter
noise_suppress_filter_v2
invert_polarity_filter
noise_gate_filter
compressor_filter
limiter_filter
expander_filter
upward_compressor_filter
luma_key_filter
luma_key_filter_v2
text_gdiplus
text_gdiplus_v2
text_gdiplus_v3
cut_transition
fade_transition
swipe_transition
slide_transition
obs_stinger_transition
fade_to_color_transition
wipe_transition
vst_filter
text_ft2_source
text_ft2_source_v2
monitor_capture
window_capture
game_capture
dshow_input
wasapi_input_capture
wasapi_output_capture
wasapi_process_output_capture
*/

/*
Unhandled exception: c0000005
Date/Time: 2025-03-19, 16:16:59
Fault address: 7FFE1C175277 (c:\program files\obs-studio\bin\64bit\w32-pthreads.dll)
libobs version: 31.0.2 (64-bit)
Windows version: 10.0 build 26100 (release: 24H2; revision: 3476; 64-bit)
CPU: Intel(R) Core(TM) i9-14900K


Thread 3584: libobs: graphics thread (Crashed)
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE0CFA60 00007FFE1C175277 000001715F7D2710 0000017152D155A0 000001715FD54CE0 000001715FD54CE0 w32-pthreads.dll!pthread_mutex_unlock+0x27
00000068EE0CFAA0 00007FFDDD022AA2 0000017152D155A0 0000000000000014 000001715FD54CE0 0000000000000015 obs.dll!obs_source_video_tick+0x572
00000068EE0CFB10 00007FFDDD034EEF 0000017151D35020 000005BC57A34BC5 0000000000000008 0000000000000000 obs.dll!tick_sources+0x29f
00000068EE0CFB70 00007FFDDD038878 0000000000000000 00023D923BC99444 000001715A1FC320 0000000000FE502A obs.dll!obs_graphics_thread_loop+0x1c8
00000068EE0CFC20 00007FFDDD038F6F FFFFFFFFFFFFFFFF 000005BC574CD444 00000068EE0CFD30 000001715A05B8F0 obs.dll!obs_graphics_thread+0x12f
00000068EE0CFCD0 00007FFE1C171461 0000017159DCC990 0000000000000000 0000000000000000 0000000000000000 w32-pthreads.dll!ptw32_threadStart+0x171
00000068EE0CFD40 00007FFE1F8837B0 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ucrtbase.dll!0x7ffe1f8837b0
00000068EE0CFD70 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE0CFDA0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 8B18:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ECAFC448 00007FFE1FE5AC94 00000000000000A4 00000068ECAFC550 0000000000000001 00000068ECAFC550 win32u.dll!0x7ffe1fe5ac94
00000068ECAFC450 00007FFD77118057 0000000000000000 0000000000000000 0000017164A3FC60 000001714F6123A0 qt6core.dll!0x7ffd77118057
00000068ECAFF5A0 00007FFD7E57AF89 00000000000000A4 0000017164A3FC60 000001714F5D0120 00000068ECAFF670 qt6gui.dll!0x7ffd7e57af89
00000068ECAFF5D0 00007FFD76F9E734 00000068ECAFF670 000001714F5A4CD0 00000068ECAFFBD0 00007FFD7728B218 qt6core.dll!0x7ffd76f9e734
00000068ECAFF650 00007FFD76F94A5D 000001714F5F9880 0000017152B1AF80 0000000000000002 000001714F5F9880 qt6core.dll!0x7ffd76f94a5d
00000068ECAFF6B0 00007FF7BDDE6A6C 00000068ECAFFB10 0000000000000001 00000068ECAFFB28 000001714F5B40E0 obs64.exe!run_program+0xd3c
00000068ECAFFAD0 00007FF7BDDE806C 0000000000000000 0000000000000000 0000000000000000 0000000000000001 obs64.exe!main+0x7ac
00000068ECAFFCB0 00007FF7BDE5BB1D 0000000000000001 0000000000000000 000001714F5D02C0 000001714F5A9390 obs64.exe!qtEntryPoint+0x15d
00000068ECAFFD40 00007FF7BDE5B08A 0000000000000000 0000000000000000 0000000000000000 0000000000000000 obs64.exe!__scrt_common_main_seh+0x106
00000068ECAFFD80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ECAFFDB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C848:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ECBFF578 00007FFE227E34A4 0000000000000000 0000000000000000 00007FFE226E44E0 000001714F58E3E0 ntdll.dll!0x7ffe227e34a4
00000068ECBFF580 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068ECBFF8E0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ECBFF910 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A358:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ECCFF848 00007FFE227E34A4 000000000001001E 0000000000000001 000001714F593E40 000001714F58E3E0 ntdll.dll!0x7ffe227e34a4
00000068ECCFF850 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068ECCFFBB0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ECCFFBE0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread BDC8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ECDFF458 00007FFE227E34A4 0000000000000000 0000000000000000 00007FFE226E44E0 000001714F58E3E0 ntdll.dll!0x7ffe227e34a4
00000068ECDFF460 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068ECDFF7C0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ECDFF7F0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C158:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ECEFF888 00007FFE227E34A4 000000000000FFFF 0000000000000000 00007FFE226E44E0 000001714F581A80 ntdll.dll!0x7ffe227e34a4
00000068ECEFF890 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068ECEFFBF0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ECEFFC20 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 64D4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ECFFF648 00007FFE227E34A4 000000000000FFFF 0000000000000000 00007FFE226E44E0 000001714F581A80 ntdll.dll!0x7ffe227e34a4
00000068ECFFF650 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068ECFFF9B0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ECFFF9E0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 38E8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED0FF528 00007FFE227E34A4 000000000000FFFF 0000000000000000 00007FFE226E44E0 000001714F620520 ntdll.dll!0x7ffe227e34a4
00000068ED0FF530 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068ED0FF890 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED0FF8C0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B854:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED1FF748 00007FFE227E0344 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227e0344
00000068ED1FF750 00007FFE1FA1E1D3 0000000000000600 0000000000000000 0000000000000600 0000000000000000 kernelbase.dll!0x7ffe1fa1e1d3
00000068ED1FFA40 00007FFE20E14C4E 0000017152B9E7A0 0000000000000000 0000000002000002 0000000002000002 combase.dll!0x7ffe20e14c4e
00000068ED1FFCB0 00007FFE20E0E541 0000000000000000 0000000000000000 0000000000000000 0000000000000000 combase.dll!0x7ffe20e0e541
00000068ED1FFD20 00007FFE20E0E3F9 0000000000000000 0000000000000000 0000000000000000 0000000000000000 combase.dll!0x7ffe20e0e3f9
00000068ED1FFD50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED1FFD80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread D2A0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED2FF798 00007FFE1FE51314 0000017152C40070 0000000000000000 0000000000000000 0000000000000001 win32u.dll!0x7ffe1fe51314
00000068ED2FF7A0 00007FFE206E5068 0000000000000001 0000000000000000 00007FFE13669000 0000000000000000 user32.dll!0x7ffe206e5068
00000068ED2FF800 00007FFE136422A4 0000000000000000 0000000000000000 0000000000000000 0000000000000000 winmm.dll!0x7ffe136422a4
00000068ED2FF8A0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED2FF8D0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B538: libobs: hotkey thread
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED4FF7B8 00007FFE227DF874 0000017152B96980 0624DD2F1A9FBE77 0000000000000000 00007FFD00000000 ntdll.dll!0x7ffe227df874
00000068ED4FF7C0 00007FFE1FA1CE4F 0000000000000010 000001715F5247A0 0000000000000000 00000000000007F4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED4FF860 00007FFDDCFF2A02 FFFFFFFFFFFFFFFF 00000068ED4FF920 0000017152B0F970 0000017152B0F970 obs.dll!obs_hotkey_thread+0x92
00000068ED4FF8C0 00007FFE1C171461 0000017152D25BD0 0000000000000000 0000000000000000 0000000000000000 w32-pthreads.dll!ptw32_threadStart+0x171
00000068ED4FF930 00007FFE1F8837B0 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ucrtbase.dll!0x7ffe1f8837b0
00000068ED4FF960 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED4FF990 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A898: tiny_tubular_task_thread
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED5FF618 00007FFE227DF874 0000017152CF25B0 00007FFE1FABD45F 00007FFE1F9D0000 00007FFE1F9E2569 ntdll.dll!0x7ffe227df874
00000068ED5FF620 00007FFE1FA1CE4F 0000017152C356C0 0000017152C356C0 0000000000000000 0000000000000804 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED5FF6C0 00007FFDDD05DA18 FFFFFFFFFFFFFFFF 00000068ED5FF780 0000017152B11030 0000017152B11030 obs.dll!tiny_tubular_task_thread+0x78
00000068ED5FF720 00007FFE1C171461 0000017152D25C30 0000000000000000 0000000000000000 0000000000000000 w32-pthreads.dll!ptw32_threadStart+0x171
00000068ED5FF790 00007FFE1F8837B0 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ucrtbase.dll!0x7ffe1f8837b0
00000068ED5FF7C0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED5FF7F0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B58C: audio-io: audio thread
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED6FF8D8 00007FFE227DFE74 0000017152B953A0 00007FFE1F89E0FB 000001714F580000 0000000000000000 ntdll.dll!0x7ffe227dfe74
00000068ED6FF8E0 00007FFE2275AAA4 0000000000000000 0000000000000000 00023D9266BB5785 0000000000000001 ntdll.dll!0x7ffe2275aaa4
00000068ED6FF910 00007FFE1FA3A911 00023D926800DCDA 00023D9200000000 FFFFFFFFFFFCCBB0 0000017151D4B4C0 kernelbase.dll!0x7ffe1fa3a911
00000068ED6FF9A0 00007FFDDD08F66F 0000000000000000 000005BC581141C0 0000000000011000 0000017151D4B4C0 obs.dll!os_sleepto_ns_fast+0x8f
00000068ED6FF9D0 00007FFDDD0628BA 000000000000136D 000005BC5737226C 0000000000000884 FFFFFFFFFFFFFFFF obs.dll!audio_thread+0x10a
00000068ED6FFA30 00007FFE1C171461 0000017151C6CCF0 0000000000000000 0000000000000000 0000000000000000 w32-pthreads.dll!ptw32_threadStart+0x171
00000068ED6FFAA0 00007FFE1F8837B0 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ucrtbase.dll!0x7ffe1f8837b0
00000068ED6FFAD0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED6FFB00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 7538:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED7FF908 00007FFE227E0344 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227e0344
00000068ED7FF910 00007FFE1FA1E1D3 0000000000000000 0000000000000000 0000017151D41810 0000000000000000 kernelbase.dll!0x7ffe1fa1e1d3
00000068ED7FFC00 00007FFE202FF4E7 0000000000000000 0000000000000000 0000000000000000 0000000000000000 crypt32.dll!0x7ffe202ff4e7
00000068ED7FFC40 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED7FFC70 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 55C8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED9FF3F8 00007FFE227E0344 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227e0344
00000068ED9FF400 00007FFE1FA1E1D3 0000000000000008 00007FFE1FA6D7E3 0000017151E20780 0000000000000000 kernelbase.dll!0x7ffe1fa1e1d3
00000068ED9FF6F0 00007FFE1FA1E0A1 00000068ED9FF7A8 00000068ED9FF7A0 0000000000000000 00007FFE1FA6D0F8 kernelbase.dll!0x7ffe1fa1e0a1
00000068ED9FF730 00007FFE0C32A706 0000017100000000 0000000000000000 0000017151F94DE0 0000000000000000 nvwgf2umx.dll!0x7ffe0c32a706
00000068ED9FF7C0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED9FF7F0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED9FF820 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C93C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EC7AF868 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EC7AF870 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EC7AF910 00007FFE0CB1DDE8 0000000000000000 0000017100000000 000001715209D2E0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EC7AF970 00007FFE0CB1DD0F 000001715209D2E0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EC7AF9A0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EC7AF9D0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EC7AFA00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A938:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EC7BFC18 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EC7BFC20 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EC7BFCC0 00007FFE0CB1DDE8 0000000000000000 0000017100000001 000001715209CF80 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EC7BFD20 00007FFE0CB1DD0F 000001715209CF80 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EC7BFD50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EC7BFD80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EC7BFDB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 1C08:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EC7CF768 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EC7CF770 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EC7CF810 00007FFE0CB1DDE8 0000000000000000 0000017100000002 000001715209D130 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EC7CF870 00007FFE0CB1DD0F 000001715209D130 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EC7CF8A0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EC7CF8D0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EC7CF900 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A534:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EC7DFD68 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EC7DFD70 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EC7DFE10 00007FFE0CB1DDE8 0000000000000000 0000017100000003 000001715209D3A0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EC7DFE70 00007FFE0CB1DD0F 000001715209D3A0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EC7DFEA0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EC7DFED0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EC7DFF00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 9F08:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EC7EFB68 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EC7EFB70 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EC7EFC10 00007FFE0CB1DDE8 0000000000000000 0000017100000004 000001715209CCE0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EC7EFC70 00007FFE0CB1DD0F 000001715209CCE0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EC7EFCA0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EC7EFCD0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EC7EFD00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 4C70:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EC7FFC48 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EC7FFC50 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EC7FFCF0 00007FFE0CB1DDE8 0000000000000000 0000017100000005 000001715209D190 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EC7FFD50 00007FFE0CB1DD0F 000001715209D190 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EC7FFD80 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EC7FFDB0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EC7FFDE0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 98D4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED80FDD8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED80FDE0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED80FE80 00007FFE0CB1DDE8 0000000000000000 0000017100000006 000001715209D250 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED80FEE0 00007FFE0CB1DD0F 000001715209D250 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED80FF10 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED80FF40 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED80FF70 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 2D64:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED81FB28 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED81FB30 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED81FBD0 00007FFE0CB1DDE8 0000000000000000 0000017100000007 000001715209CEF0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED81FC30 00007FFE0CB1DD0F 000001715209CEF0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED81FC60 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED81FC90 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED81FCC0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B644:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED82F6E8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED82F6F0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED82F790 00007FFE0CB1DDE8 0000000000000000 0000017100000008 000001715209D310 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED82F7F0 00007FFE0CB1DD0F 000001715209D310 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED82F820 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED82F850 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED82F880 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 3074:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED83FA18 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED83FA20 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED83FAC0 00007FFE0CB1DDE8 0000000000000000 0000017100000009 000001715209CD10 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED83FB20 00007FFE0CB1DD0F 000001715209CD10 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED83FB50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED83FB80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED83FBB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 6360:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED84F978 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED84F980 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED84FA20 00007FFE0CB1DDE8 0000000000000000 000001710000000A 000001715209CD40 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED84FA80 00007FFE0CB1DD0F 000001715209CD40 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED84FAB0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED84FAE0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED84FB10 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 2A54:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED85FBB8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED85FBC0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED85FC60 00007FFE0CB1DDE8 0000000000000000 000001710000000B 000001715209D220 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED85FCC0 00007FFE0CB1DD0F 000001715209D220 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED85FCF0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED85FD20 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED85FD50 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A3B8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED86F658 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED86F660 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED86F700 00007FFE0CB1DDE8 0000000000000000 000001710000000C 000001715209CD70 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED86F760 00007FFE0CB1DD0F 000001715209CD70 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED86F790 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED86F7C0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED86F7F0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread AAEC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED87F648 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED87F650 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED87F6F0 00007FFE0CB1DDE8 0000000000000000 000001710000000D 000001715209D6D0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED87F750 00007FFE0CB1DD0F 000001715209D6D0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED87F780 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED87F7B0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED87F7E0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 27AC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED88F8F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED88F900 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED88F9A0 00007FFE0CB1DDE8 0000000000000000 000001710000000E 000001715209D040 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED88FA00 00007FFE0CB1DD0F 000001715209D040 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED88FA30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED88FA60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED88FA90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 9474:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED89F9F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED89FA00 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED89FAA0 00007FFE0CB1DDE8 0000000000000000 000001710000000F 000001715209D160 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED89FB00 00007FFE0CB1DD0F 000001715209D160 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED89FB30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED89FB60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED89FB90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 81BC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED8AFBC8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED8AFBD0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED8AFC70 00007FFE0CB1DDE8 0000000000000000 0000017100000010 000001715209D550 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED8AFCD0 00007FFE0CB1DD0F 000001715209D550 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED8AFD00 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED8AFD30 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED8AFD60 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 576C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED8BF6B8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED8BF6C0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED8BF760 00007FFE0CB1DDE8 0000000000000000 0000017100000011 000001715209D1F0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED8BF7C0 00007FFE0CB1DD0F 000001715209D1F0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED8BF7F0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED8BF820 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED8BF850 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 10E4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED8CFB18 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED8CFB20 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED8CFBC0 00007FFE0CB1DDE8 0000000000000000 0000017100000012 000001715209D7F0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED8CFC20 00007FFE0CB1DD0F 000001715209D7F0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED8CFC50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED8CFC80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED8CFCB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 937C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED8DF948 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED8DF950 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED8DF9F0 00007FFE0CB1DDE8 0000000000000000 0000017100000013 000001715209D0D0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED8DFA50 00007FFE0CB1DD0F 000001715209D0D0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED8DFA80 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED8DFAB0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED8DFAE0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread CAF8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED8EFDA8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED8EFDB0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED8EFE50 00007FFE0CB1DDE8 0000000000000000 0000017100000014 000001715209D5B0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED8EFEB0 00007FFE0CB1DD0F 000001715209D5B0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED8EFEE0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED8EFF10 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED8EFF40 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 274C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED8FFB78 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068ED8FFB80 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068ED8FFC20 00007FFE0CB1DDE8 0000000000000000 0000017100000015 000001715209D100 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068ED8FFC80 00007FFE0CB1DD0F 000001715209D100 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068ED8FFCB0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068ED8FFCE0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED8FFD10 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread AC28:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA0F788 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA0F790 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA0F830 00007FFE0CB1DDE8 0000000000000000 0000017100000016 000001715209D580 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA0F890 00007FFE0CB1DD0F 000001715209D580 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA0F8C0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA0F8F0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA0F920 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 7B38:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA1F8F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA1F900 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA1F9A0 00007FFE0CB1DDE8 0000000000000000 0000017100000017 000001715209D700 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA1FA00 00007FFE0CB1DD0F 000001715209D700 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA1FA30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA1FA60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA1FA90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 97B8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA2FBE8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA2FBF0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA2FC90 00007FFE0CB1DDE8 0000000000000000 0000017100000018 000001715209D2B0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA2FCF0 00007FFE0CB1DD0F 000001715209D2B0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA2FD20 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA2FD50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA2FD80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread CBE0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA3FA78 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA3FA80 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA3FB20 00007FFE0CB1DDE8 0000000000000000 0000017100000019 000001715209CE30 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA3FB80 00007FFE0CB1DD0F 000001715209CE30 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA3FBB0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA3FBE0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA3FC10 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread DAE8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA4F7F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA4F800 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA4F8A0 00007FFE0CB1DDE8 0000000000000000 000001710000001A 000001715209CE90 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA4F900 00007FFE0CB1DD0F 000001715209CE90 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA4F930 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA4F960 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA4F990 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 6440:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA5FA08 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA5FA10 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA5FAB0 00007FFE0CB1DDE8 0000000000000000 000001710000001B 000001715209D610 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA5FB10 00007FFE0CB1DD0F 000001715209D610 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA5FB40 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA5FB70 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA5FBA0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A828:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA6FD98 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA6FDA0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA6FE40 00007FFE0CB1DDE8 0000000000000000 000001710000001C 000001715209D340 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA6FEA0 00007FFE0CB1DD0F 000001715209D340 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA6FED0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA6FF00 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA6FF30 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 17A8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA7F798 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA7F7A0 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA7F840 00007FFE0CB1DDE8 0000000000000000 000001710000001D 000001715209CEC0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA7F8A0 00007FFE0CB1DD0F 000001715209CEC0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA7F8D0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA7F900 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA7F930 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread CCA0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA8F948 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA8F950 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA8F9F0 00007FFE0CB1DDE8 0000000000000000 000001710000001E 000001715209D430 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA8FA50 00007FFE0CB1DD0F 000001715209D430 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA8FA80 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA8FAB0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA8FAE0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A564:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDA9F8F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDA9F900 00007FFE1FA1CE4F 0000000000000000 000001715217B4C8 0000000000000000 00000000000009B4 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDA9F9A0 00007FFE0CB1DDE8 0000000000000000 000001710000001F 000001715209CFB0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDA9FA00 00007FFE0CB1DD0F 000001715209CFB0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDA9FA30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDA9FA60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDA9FA90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 81CC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDABF528 00007FFE227DF874 0000000000000000 0000043800000780 00000DBA4DB2D3DD 0000043800000780 ntdll.dll!0x7ffe227df874
00000068EDABF530 00007FFE1FA1CE4F 0000017151E1F940 0000006800000001 0000006800000000 0000000000000B30 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDABF5D0 00007FFE0C7ED38E 0000000000000001 0000000000000000 0000000000000001 0000017151E1F940 nvwgf2umx.dll!0x7ffe0c7ed38e
00000068EDABF8B0 00007FFE0C7ECD13 0000017151E1FE68 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0c7ecd13
00000068EDABF930 00007FFE0C7E162D 0000000000000000 0000017152287670 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0c7e162d
00000068EDABF960 00007FFE0CC80D2A 000001715209D460 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cc80d2a
00000068EDABF990 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDABF9C0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDABF9F0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A6C4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDACFB38 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDACFB40 00007FFE1FA1CE4F 0000000000000000 00000171522A3968 0000000000000000 0000000000000B38 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDACFBE0 00007FFE0CB1DDE8 0000000000000000 0000017100000000 000001715209CF50 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDACFC40 00007FFE0CB1DD0F 000001715209CF50 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDACFC70 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDACFCA0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDACFCD0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 65D8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDADF618 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDADF620 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDADF6C0 00007FFE0CB1DDE8 0000000000000000 0000017100000000 000001715209E0F0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDADF720 00007FFE0CB1DD0F 000001715209E0F0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDADF750 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDADF780 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDADF7B0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 214C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDAEF9C8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDAEF9D0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDAEFA70 00007FFE0CB1DDE8 0000000000000000 0000017100000001 000001715209D970 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDAEFAD0 00007FFE0CB1DD0F 000001715209D970 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDAEFB00 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDAEFB30 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDAEFB60 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 3DDC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDAFFC18 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDAFFC20 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDAFFCC0 00007FFE0CB1DDE8 0000000000000000 0000017100000002 000001715209DD90 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDAFFD20 00007FFE0CB1DD0F 000001715209DD90 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDAFFD50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDAFFD80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDAFFDB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C82C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB0F7C8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB0F7D0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB0F870 00007FFE0CB1DDE8 0000000000000000 0000017100000003 000001715209E180 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB0F8D0 00007FFE0CB1DD0F 000001715209E180 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB0F900 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB0F930 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB0F960 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread CFB0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB1FDA8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB1FDB0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB1FE50 00007FFE0CB1DDE8 0000000000000000 0000017100000004 000001715209E420 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB1FEB0 00007FFE0CB1DD0F 000001715209E420 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB1FEE0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB1FF10 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB1FF40 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread BBC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB2F688 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB2F690 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB2F730 00007FFE0CB1DDE8 0000000000000000 0000017100000005 000001715209E1E0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB2F790 00007FFE0CB1DD0F 000001715209E1E0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB2F7C0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB2F7F0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB2F820 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 32EC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB3F658 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB3F660 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB3F700 00007FFE0CB1DDE8 0000000000000000 0000017100000006 000001715209E210 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB3F760 00007FFE0CB1DD0F 000001715209E210 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB3F790 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB3F7C0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB3F7F0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 38A4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB4F868 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB4F870 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB4F910 00007FFE0CB1DDE8 0000000000000000 0000017100000007 000001715209DF70 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB4F970 00007FFE0CB1DD0F 000001715209DF70 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB4F9A0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB4F9D0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB4FA00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread E1C4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB5F998 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB5F9A0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB5FA40 00007FFE0CB1DDE8 0000000000000000 0000017100000008 000001715209DF10 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB5FAA0 00007FFE0CB1DD0F 000001715209DF10 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB5FAD0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB5FB00 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB5FB30 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A230:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB6FAE8 00007FFE227DF874 0000000000000000 0000000000000000 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB6FAF0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000000000000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB6FB90 00007FFE0CB1DDE8 0000000000000000 0000017100000009 000001715209DBB0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB6FBF0 00007FFE0CB1DD0F 000001715209DBB0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB6FC20 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB6FC50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB6FC80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 6D90:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB7FBB8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB7FBC0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB7FC60 00007FFE0CB1DDE8 0000000000000000 000001710000000A 000001715209DD60 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB7FCC0 00007FFE0CB1DD0F 000001715209DD60 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB7FCF0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB7FD20 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB7FD50 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C284:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB8FB28 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB8FB30 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB8FBD0 00007FFE0CB1DDE8 0000000000000000 000001710000000B 000001715209DCA0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB8FC30 00007FFE0CB1DD0F 000001715209DCA0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB8FC60 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB8FC90 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB8FCC0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 417C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDB9F628 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDB9F630 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDB9F6D0 00007FFE0CB1DDE8 0000000000000000 000001710000000C 000001715209E240 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDB9F730 00007FFE0CB1DD0F 000001715209E240 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDB9F760 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDB9F790 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDB9F7C0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread E6D4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDBAFC58 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDBAFC60 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDBAFD00 00007FFE0CB1DDE8 0000000000000000 000001710000000D 000001715209E390 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDBAFD60 00007FFE0CB1DD0F 000001715209E390 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDBAFD90 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDBAFDC0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDBAFDF0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 29BC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDBBFDA8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDBBFDB0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDBBFE50 00007FFE0CB1DDE8 0000000000000000 000001710000000E 000001715209E270 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDBBFEB0 00007FFE0CB1DD0F 000001715209E270 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDBBFEE0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDBBFF10 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDBBFF40 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread E384:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDBCFAF8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDBCFB00 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDBCFBA0 00007FFE0CB1DDE8 0000000000000000 000001710000000F 000001715209E450 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDBCFC00 00007FFE0CB1DD0F 000001715209E450 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDBCFC30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDBCFC60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDBCFC90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 3390:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDBDFC78 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDBDFC80 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDBDFD20 00007FFE0CB1DDE8 0000000000000000 0000017100000010 000001715209D9A0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDBDFD80 00007FFE0CB1DD0F 000001715209D9A0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDBDFDB0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDBDFDE0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDBDFE10 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 6244:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDBEFA78 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDBEFA80 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDBEFB20 00007FFE0CB1DDE8 0000000000000000 0000017100000011 000001715209DDC0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDBEFB80 00007FFE0CB1DD0F 000001715209DDC0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDBEFBB0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDBEFBE0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDBEFC10 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 9320:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDBFFB98 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDBFFBA0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDBFFC40 00007FFE0CB1DDE8 0000000000000000 0000017100000012 000001715209E2A0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDBFFCA0 00007FFE0CB1DD0F 000001715209E2A0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDBFFCD0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDBFFD00 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDBFFD30 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A190:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC0FBC8 00007FFE227DF874 0000000000000000 0000000000000000 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC0FBD0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000000000000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC0FC70 00007FFE0CB1DDE8 0000000000000000 0000017100000013 000001715209DC40 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC0FCD0 00007FFE0CB1DD0F 000001715209DC40 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC0FD00 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC0FD30 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC0FD60 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 1940:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC1FAF8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC1FB00 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC1FBA0 00007FFE0CB1DDE8 0000000000000000 0000017100000014 000001715209DDF0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC1FC00 00007FFE0CB1DD0F 000001715209DDF0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC1FC30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC1FC60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC1FC90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 71DC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC2FD08 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC2FD10 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC2FDB0 00007FFE0CB1DDE8 0000000000000000 0000017100000015 000001715209DE50 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC2FE10 00007FFE0CB1DD0F 000001715209DE50 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC2FE40 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC2FE70 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC2FEA0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 23A4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC3FAB8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC3FAC0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC3FB60 00007FFE0CB1DDE8 0000000000000000 0000017100000016 000001715209E3C0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC3FBC0 00007FFE0CB1DD0F 000001715209E3C0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC3FBF0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC3FC20 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC3FC50 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B120:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC4FAE8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC4FAF0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC4FB90 00007FFE0CB1DDE8 0000000000000000 0000017100000017 000001715209DBE0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC4FBF0 00007FFE0CB1DD0F 000001715209DBE0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC4FC20 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC4FC50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC4FC80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 8914:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC5F7F8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC5F800 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC5F8A0 00007FFE0CB1DDE8 0000000000000000 0000017100000018 000001715209DE80 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC5F900 00007FFE0CB1DD0F 000001715209DE80 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC5F930 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC5F960 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC5F990 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 5C50:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC6FA28 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC6FA30 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC6FAD0 00007FFE0CB1DDE8 0000000000000000 0000017100000019 000001715209E330 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC6FB30 00007FFE0CB1DD0F 000001715209E330 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC6FB60 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC6FB90 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC6FBC0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 4FE4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC7F8D8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC7F8E0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC7F980 00007FFE0CB1DDE8 0000000000000000 000001710000001A 000001715209D880 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC7F9E0 00007FFE0CB1DD0F 000001715209D880 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC7FA10 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC7FA40 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC7FA70 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread DD4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC8FDD8 00007FFE227DF874 0000017151E98980 0000017151E98130 0000000000000000 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC8FDE0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC8FE80 00007FFE0CB1DDE8 0000000000000000 000001710000001B 000001715209D8E0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC8FEE0 00007FFE0CB1DD0F 000001715209D8E0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC8FF10 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC8FF40 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC8FF70 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A7A8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDC9F7B8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDC9F7C0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDC9F860 00007FFE0CB1DDE8 0000000000000000 000001710000001C 000001715209E2D0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDC9F8C0 00007FFE0CB1DD0F 000001715209E2D0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDC9F8F0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDC9F920 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDC9F950 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread AD98:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDCAFC68 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDCAFC70 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDCAFD10 00007FFE0CB1DDE8 0000000000000000 000001710000001D 000001715209DCD0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDCAFD70 00007FFE0CB1DD0F 000001715209DCD0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDCAFDA0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDCAFDD0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDCAFE00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 5D2C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDCBFCF8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDCBFD00 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDCBFDA0 00007FFE0CB1DDE8 0000000000000000 000001710000001E 000001715209DEB0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDCBFE00 00007FFE0CB1DD0F 000001715209DEB0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDCBFE30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDCBFE60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDCBFE90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread DE80:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDCCFDE8 00007FFE227DF874 0000017151E98980 0000017151E98130 FFFFFFFFFFFFFFFF 0000017151E98130 ntdll.dll!0x7ffe227df874
00000068EDCCFDF0 00007FFE1FA1CE4F 0000000000000000 0000017151E98138 0000017100000000 0000000000000970 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDCCFE90 00007FFE0CB1DDE8 0000000000000000 000001710000001F 000001715209DC10 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDCCFEF0 00007FFE0CB1DD0F 000001715209DC10 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDCCFF20 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDCCFF50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDCCFF80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 34D0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDCDFCF8 00007FFE227DF874 0000017151E992D0 0000017151E98A80 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDCDFD00 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000017100000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDCDFDA0 00007FFE0CB1DDE8 0000000000000000 0000017100000000 0000017152364DE0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDCDFE00 00007FFE0CB1DD0F 0000017152364DE0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDCDFE30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDCDFE60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDCDFE90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C540:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDCEF9E8 00007FFE227DF874 0000017151E992D0 0000017151E98A80 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDCEF9F0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000017100000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDCEFA90 00007FFE0CB1DDE8 0000000000000000 0000017100000001 0000017152365440 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDCEFAF0 00007FFE0CB1DD0F 0000017152365440 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDCEFB20 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDCEFB50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDCEFB80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 54EC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDCFF6F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDCFF700 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDCFF7A0 00007FFE0CB1DDE8 0000000000000000 0000017100000002 0000017152364F90 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDCFF800 00007FFE0CB1DD0F 0000017152364F90 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDCFF830 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDCFF860 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDCFF890 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 98C0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD0F618 00007FFE227DF874 0000017151E992D0 0000017151E98A80 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD0F620 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000017100000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD0F6C0 00007FFE0CB1DDE8 0000000000000000 0000017100000003 0000017152365350 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD0F720 00007FFE0CB1DD0F 0000017152365350 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD0F750 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD0F780 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD0F7B0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread D00C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD1FD18 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD1FD20 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD1FDC0 00007FFE0CB1DDE8 0000000000000000 0000017100000004 0000017152365410 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD1FE20 00007FFE0CB1DD0F 0000017152365410 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD1FE50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD1FE80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD1FEB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread D214:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD2F938 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD2F940 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD2F9E0 00007FFE0CB1DDE8 0000000000000000 0000017100000005 00000171523650B0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD2FA40 00007FFE0CB1DD0F 00000171523650B0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD2FA70 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD2FAA0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD2FAD0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread AAA4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD3F9E8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD3F9F0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD3FA90 00007FFE0CB1DDE8 0000000000000000 0000017100000006 0000017152364F60 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD3FAF0 00007FFE0CB1DD0F 0000017152364F60 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD3FB20 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD3FB50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD3FB80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B654:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD4F808 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD4F810 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD4F8B0 00007FFE0CB1DDE8 0000000000000000 0000017100000007 0000017152364EA0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD4F910 00007FFE0CB1DD0F 0000017152364EA0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD4F940 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD4F970 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD4F9A0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread BAD8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD5F948 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD5F950 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD5F9F0 00007FFE0CB1DDE8 0000000000000000 0000017100000008 0000017152365140 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD5FA50 00007FFE0CB1DD0F 0000017152365140 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD5FA80 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD5FAB0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD5FAE0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 2BD4:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD6F858 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD6F860 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD6F900 00007FFE0CB1DDE8 0000000000000000 0000017100000009 0000017152364E10 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD6F960 00007FFE0CB1DD0F 0000017152364E10 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD6F990 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD6F9C0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD6F9F0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 9018:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD7F6C8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD7F6D0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD7F770 00007FFE0CB1DDE8 0000000000000000 000001710000000A 0000017152364F30 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD7F7D0 00007FFE0CB1DD0F 0000017152364F30 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD7F800 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD7F830 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD7F860 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 2A28:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD8FB78 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD8FB80 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD8FC20 00007FFE0CB1DDE8 0000000000000000 000001710000000B 0000017152364E40 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD8FC80 00007FFE0CB1DD0F 0000017152364E40 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD8FCB0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD8FCE0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD8FD10 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A2E0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDD9F778 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDD9F780 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDD9F820 00007FFE0CB1DDE8 0000000000000000 000001710000000C 0000017152364FC0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDD9F880 00007FFE0CB1DD0F 0000017152364FC0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDD9F8B0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDD9F8E0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDD9F910 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A170:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDDAFD38 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDDAFD40 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDDAFDE0 00007FFE0CB1DDE8 0000000000000000 000001710000000D 00000171523654A0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDDAFE40 00007FFE0CB1DD0F 00000171523654A0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDDAFE70 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDDAFEA0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDDAFED0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 866C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDDBF898 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDDBF8A0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDDBF940 00007FFE0CB1DDE8 0000000000000000 000001710000000E 0000017152365530 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDDBF9A0 00007FFE0CB1DD0F 0000017152365530 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDDBF9D0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDDBFA00 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDDBFA30 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread ED48:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDDCF888 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDDCF890 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDDCF930 00007FFE0CB1DDE8 0000000000000000 000001710000000F 000001715230E3F0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDDCF990 00007FFE0CB1DD0F 000001715230E3F0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDDCF9C0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDDCF9F0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDDCFA20 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread E73C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDDDF928 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDDDF930 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDDDF9D0 00007FFE0CB1DDE8 0000000000000000 0000017100000010 000001715230E150 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDDDFA30 00007FFE0CB1DD0F 000001715230E150 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDDDFA60 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDDDFA90 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDDDFAC0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 6F28:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDDEF8D8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDDEF8E0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDDEF980 00007FFE0CB1DDE8 0000000000000000 0000017100000011 000001715230E180 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDDEF9E0 00007FFE0CB1DD0F 000001715230E180 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDDEFA10 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDDEFA40 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDDEFA70 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 63C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDDFFA08 00007FFE227DF874 0000017151E992D0 0000017151E98A80 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDDFFA10 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000017100000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDDFFAB0 00007FFE0CB1DDE8 0000000000000000 0000017100000012 000001715230E240 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDDFFB10 00007FFE0CB1DD0F 000001715230E240 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDDFFB40 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDDFFB70 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDDFFBA0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 472C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE0F9B8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE0F9C0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE0FA60 00007FFE0CB1DDE8 0000000000000000 0000017100000013 000001715230E270 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE0FAC0 00007FFE0CB1DD0F 000001715230E270 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE0FAF0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE0FB20 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE0FB50 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 96D8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE1FD88 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE1FD90 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE1FE30 00007FFE0CB1DDE8 0000000000000000 0000017100000014 000001715230E1B0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE1FE90 00007FFE0CB1DD0F 000001715230E1B0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE1FEC0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE1FEF0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE1FF20 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C338:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE2F888 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE2F890 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE2F930 00007FFE0CB1DDE8 0000000000000000 0000017100000015 000001715230E450 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE2F990 00007FFE0CB1DD0F 000001715230E450 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE2F9C0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE2F9F0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE2FA20 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 9DC8:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE3FAB8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE3FAC0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE3FB60 00007FFE0CB1DDE8 0000000000000000 0000017100000016 000001715230E2A0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE3FBC0 00007FFE0CB1DD0F 000001715230E2A0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE3FBF0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE3FC20 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE3FC50 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 9E68:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE4F898 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE4F8A0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE4F940 00007FFE0CB1DDE8 0000000000000000 0000017100000017 000001715230E330 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE4F9A0 00007FFE0CB1DD0F 000001715230E330 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE4F9D0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE4FA00 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE4FA30 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread DC94:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE5F8F8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE5F900 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE5F9A0 00007FFE0CB1DDE8 0000000000000000 0000017100000018 000001715230E3C0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE5FA00 00007FFE0CB1DD0F 000001715230E3C0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE5FA30 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE5FA60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE5FA90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread EB88:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE6F8C8 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE6F8D0 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE6F970 00007FFE0CB1DDE8 0000000000000000 0000017100000019 000001715230E420 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE6F9D0 00007FFE0CB1DD0F 000001715230E420 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE6FA00 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE6FA30 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE6FA60 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 20B0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE7F968 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE7F970 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE7FA10 00007FFE0CB1DDE8 0000000000000000 000001710000001A 00000171523B19F0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE7FA70 00007FFE0CB1DD0F 00000171523B19F0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE7FAA0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE7FAD0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE7FB00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 1314:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE8FC18 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE8FC20 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE8FCC0 00007FFE0CB1DDE8 0000000000000000 000001710000001B 00000171523B19C0 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE8FD20 00007FFE0CB1DD0F 00000171523B19C0 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE8FD50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE8FD80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE8FDB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A00C:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDE9F838 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDE9F840 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDE9F8E0 00007FFE0CB1DDE8 0000000000000000 000001710000001C 00000171523B1A50 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDE9F940 00007FFE0CB1DD0F 00000171523B1A50 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDE9F970 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDE9F9A0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDE9F9D0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread E5DC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDEAFA18 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDEAFA20 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDEAFAC0 00007FFE0CB1DDE8 0000000000000000 000001710000001D 00000171523B1A80 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDEAFB20 00007FFE0CB1DD0F 00000171523B1A80 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDEAFB50 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDEAFB80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDEAFBB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 8550:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDEBF948 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDEBF950 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDEBF9F0 00007FFE0CB1DDE8 0000000000000000 000001710000001E 00000171523B1A20 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDEBFA50 00007FFE0CB1DD0F 00000171523B1A20 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDEBFA80 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDEBFAB0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDEBFAE0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 5A90:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDECFA68 00007FFE227DF874 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDECFA70 00007FFE1FA1CE4F 0000000000000000 0000017151E98A88 0000000000000000 0000000000000978 kernelbase.dll!0x7ffe1fa1ce4f
00000068EDECFB10 00007FFE0CB1DDE8 0000000000000000 000001710000001F 00000171523B1960 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dde8
00000068EDECFB70 00007FFE0CB1DD0F 00000171523B1960 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0cb1dd0f
00000068EDECFBA0 00007FFE0D1C42FA 0000000000000000 0000000000000000 0000000000000000 0000000000000000 nvwgf2umx.dll!0x7ffe0d1c42fa
00000068EDECFBD0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDECFC00 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 4E0: video-io: video thread
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EDFCF698 00007FFE227DF874 000001714F5F9888 00007FFDDCFDF21D 0000000000000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EDFCF6A0 00007FFE1FA1CE4F 00000171523B3EC0 00007FFDDD0AA8A8 00007FFD00000000 0000000000000E9C kernelbase.dll!0x7ffe1fa1ce4f
00000068EDFCF740 00007FFDDD065368 0000000000000000 FFFFFFFFFFFFFFFF 00000171598743E0 0000000000000000 obs.dll!video_thread+0xa8
00000068EDFCF830 00007FFE1C171461 0000017159DD38F0 0000000000000000 0000000000000000 0000000000000000 w32-pthreads.dll!ptw32_threadStart+0x171
00000068EDFCF8A0 00007FFE1F8837B0 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ucrtbase.dll!0x7ffe1f8837b0
00000068EDFCF8D0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EDFCF900 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 3BD4: scripting: defer
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE1CFAB8 00007FFE227DF874 0000000000000000 00007FFE22690647 0000017100000000 0000000000000000 ntdll.dll!0x7ffe227df874
00000068EE1CFAC0 00007FFE1FA1CE4F FFFFFFFFFFFFFFFF 00007FFDF03D7AC8 0000000000000000 0000000000000F24 kernelbase.dll!0x7ffe1fa1ce4f
00000068EE1CFB60 00007FFDDD091EBE 000001715A08ADC0 FFFFFFFFFFFFFFFF 000001715A05A230 00007FFE1F89E0FB obs.dll!os_sem_wait+0x1e
00000068EE1CFB90 00007FFDF03D1B8C FFFFFFFFFFFFFFFF 000001715A1CC950 00000068EE1CFC40 000001715A05A230 obs-scripting.dll!defer_thread+0x2c
00000068EE1CFBE0 00007FFE1C171461 000001715A1CC950 0000000000000000 0000000000000000 0000000000000000 w32-pthreads.dll!ptw32_threadStart+0x171
00000068EE1CFC50 00007FFE1F8837B0 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ucrtbase.dll!0x7ffe1f8837b0
00000068EE1CFC80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE1CFCB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread EA58:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068ED3FFDE8 00007FFE1FE5AC94 0000000000000000 000001715A2F0AC0 0000000000000000 0000000000000001 win32u.dll!0x7ffe1fe5ac94
00000068ED3FFDF0 00007FFE0A57D47B 0000000000000000 0000000000000720 0000000000000000 0000000000000000 gdiplus.dll!0x7ffe0a57d47b
00000068ED3FFE60 00007FFE0A5AA4DE 0000000000000000 0000000000000000 0000000000000000 0000000000000000 gdiplus.dll!0x7ffe0a5aa4de
00000068ED3FFE90 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068ED3FFEC0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 6BA0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE3CFB18 00007FFE227DF914 0000000000000000 00000068EE3CFB60 0000000000000001 00007FFE1ECD0000 ntdll.dll!0x7ffe227df914
00000068EE3CFB20 00007FFE1ECEE23E 000001716A9D8F20 00007FFE1ECE35E0 0000000000000000 0000000000000000 mswsock.dll!0x7ffe1ecee23e
00000068EE3CFB80 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE3CFBB0 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B3F0:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE6CF5D8 00007FFE227E34A4 0000000000000000 0000000000000000 00007FFE226E5190 00000171523C6D50 ntdll.dll!0x7ffe227e34a4
00000068EE6CF5E0 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068EE6CF940 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE6CF970 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread C6DC:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE7CF6C8 00007FFE227E34A4 0000000000000000 0000000000000000 00007FFE226E5190 00000171523C6D50 ntdll.dll!0x7ffe227e34a4
00000068EE7CF6D0 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068EE7CFA30 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE7CFA60 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 5F10:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE8CF538 00007FFE227E0344 000001714F6A02A8 0000000000000001 00000068EE8CF6C9 0000000000000020 ntdll.dll!0x7ffe227e0344
00000068EE8CF540 00007FFE1FA1E1D3 0000000000000000 00007FFE13FA0928 0000000000000000 000001715F4C3580 kernelbase.dll!0x7ffe1fa1e1d3
00000068EE8CF830 00007FFE13E7B380 00007FFE13F68500 000001715F4C3580 0000000000000000 00000000000018DC audioses.dll!Ordinal4+0xf9b0
00000068EE8CF9C0 00007FFE13E77024 0000000000000000 000000000000136F 0000000000000000 0000000000000000 audioses.dll!Ordinal4+0xb654
00000068EE8CF9F0 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE8CFA20 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread 3960:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE2CF498 00007FFE227E34A4 0000000000000000 0000000000000000 00007FFE226E5190 00000171523C6D50 ntdll.dll!0x7ffe227e34a4
00000068EE2CF4A0 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068EE2CF800 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE2CF830 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread A914:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE4CFBE8 00007FFE227E34A4 0000000000000000 0000000000000000 00007FFE226E5190 00000171523C6D50 ntdll.dll!0x7ffe227e34a4
00000068EE4CFBF0 00007FFE226E5D2E 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe226e5d2e
00000068EE4CFF50 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE4CFF80 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Thread B080:
Stack            EIP              Arg0             Arg1             Arg2             Arg3             Address
00000068EE5CEA98 00007FFE227E0344 0000000000000000 00007FFE2269BE86 0000000000000002 0000000000000000 ntdll.dll!0x7ffe227e0344
00000068EE5CEAA0 00007FFE1FA1E1D3 0000000000000029 0000000000000000 00000000000003E8 0000000000000000 kernelbase.dll!0x7ffe1fa1e1d3
00000068EE5CED90 00007FFDEBCAAB54 0000000000099F79 0000006800008E40 0000000000000000 000001715A022F30 libcurl.dll!0x7ffdebcaab54
00000068EE5CEF70 00007FFDEBCA852D 000001715A022F30 FFFFFFFF00000006 0000000000000000 0000000000000000 libcurl.dll!0x7ffdebca852d
00000068EE5CEFC0 00007FFDEBC87611 0000017100000000 000000000000272D 000001716AB7EED0 00000000000000AB libcurl.dll!0x7ffdebc87611
00000068EE5CF010 00007FF7BDBD527C 000001716A9D8AA0 000001716AB7EC30 000001716AB7EEB0 00000068EE5CF3A0 obs64.exe!GetRemoteFile+0x48c
00000068EE5CF2A0 00007FF7BDE04EB0 00007FF7BDBB0000 0000000000000000 000001715F772028 00000068EE5CF7B0 obs64.exe!FetchAndVerifyFile+0x4a0
00000068EE5CF4B0 00007FF7BDE07A21 0000000000000000 000001715F772028 0000017164CD6A90 0000000000000000 obs64.exe!AutoUpdateThread::run+0x441
00000068EE5CFA20 00007FFD7711E7F2 0000000000000000 0000000000000000 0000000000000000 0000000000000000 qt6core.dll!0x7ffd7711e7f2
00000068EE5CFA60 00007FFE208CE8D7 0000000000000000 0000000000000000 000004F0FFFFFB30 000004D0FFFFFB30 kernel32.dll!0x7ffe208ce8d7
00000068EE5CFA90 00007FFE2273BF6C 0000000000000000 0000000000000000 0000000000000000 0000000000000000 ntdll.dll!0x7ffe2273bf6c

Loaded modules:
Base Address                      Module
00007FF7BDBB0000-00007FF7BE09C000 C:\Program Files\obs-studio\bin\64bit\obs64.exe
00007FFE22680000-00007FFE228E3000 C:\WINDOWS\SYSTEM32\ntdll.dll
00007FFE208A0000-00007FFE20967000 C:\WINDOWS\System32\KERNEL32.DLL
00007FFE1F9D0000-00007FFE1FD99000 C:\WINDOWS\System32\KERNELBASE.dll
00007FFE1C9C0000-00007FFE1CA5C000 C:\WINDOWS\SYSTEM32\apphelp.dll
00007FFE218C0000-00007FFE21FD5000 C:\WINDOWS\System32\SHELL32.dll
00007FFDEBC70000-00007FFDEBD02000 C:\Program Files\obs-studio\bin\64bit\libcurl.dll
00007FFD87370000-00007FFD875B4000 C:\Program Files\obs-studio\bin\64bit\avformat-61.dll
00007FFE1FDA0000-00007FFE1FE43000 C:\WINDOWS\System32\msvcp_win.dll
00007FFE22020000-00007FFE22094000 C:\WINDOWS\System32\WS2_32.dll
00007FFE20B60000-00007FFE20C76000 C:\WINDOWS\System32\RPCRT4.dll
00007FFE1F880000-00007FFE1F9CC000 C:\WINDOWS\System32\ucrtbase.dll
00007FFD7EFA0000-00007FFD81887000 C:\Program Files\obs-studio\bin\64bit\avcodec-61.dll
00007FFE209D0000-00007FFE20A43000 C:\WINDOWS\System32\WLDAP32.dll
00007FFE215B0000-00007FFE21745000 C:\WINDOWS\System32\ole32.dll
00007FFE206D0000-00007FFE2089A000 C:\WINDOWS\System32\USER32.dll
00007FFE21150000-00007FFE21202000 C:\WINDOWS\System32\ADVAPI32.dll
00007FFE1FE50000-00007FFE1FE77000 C:\WINDOWS\System32\win32u.dll
00007FFE21750000-00007FFE2177A000 C:\WINDOWS\System32\GDI32.dll
00007FFDDCFD0000-00007FFDDD0E1000 C:\Program Files\obs-studio\bin\64bit\obs.dll
00007FFE220A0000-00007FFE22149000 C:\WINDOWS\System32\msvcrt.dll
00007FFE1FE80000-00007FFE1FFB1000 C:\WINDOWS\System32\gdi32full.dll
00007FFE1FFC0000-00007FFE20128000 C:\WINDOWS\System32\wintypes.dll
00007FFE204C0000-00007FFE20566000 C:\WINDOWS\System32\sechost.dll
00007FFE20CD0000-00007FFE21052000 C:\WINDOWS\System32\combase.dll
00007FFE20260000-00007FFE203D7000 C:\WINDOWS\System32\CRYPT32.dll
00007FFE205F0000-00007FFE206C6000 C:\WINDOWS\System32\OLEAUT32.dll
00007FFE1C8C0000-00007FFE1C8CD000 C:\Program Files\obs-studio\bin\64bit\obs-frontend-api.dll
00007FFE1C170000-00007FFE1C182000 C:\Program Files\obs-studio\bin\64bit\w32-pthreads.dll
00007FFDEB210000-00007FFDEB26F000 C:\Program Files\obs-studio\bin\64bit\Qt6Svg.dll
00007FFE088F0000-00007FFE08917000 C:\Program Files\obs-studio\bin\64bit\Qt6Xml.dll
00007FFD8E8E0000-00007FFD8EA4D000 C:\Program Files\obs-studio\bin\64bit\Qt6Network.dll
00007FFE1CDC0000-00007FFE1CEEE000 C:\WINDOWS\SYSTEM32\dxgi.dll
00007FFD7E980000-00007FFD7EF93000 C:\Program Files\obs-studio\bin\64bit\Qt6Widgets.dll
00007FFD7E220000-00007FFD7E97D000 C:\Program Files\obs-studio\bin\64bit\Qt6Gui.dll
00007FFD72850000-00007FFD73943000 C:\Program Files\obs-studio\bin\64bit\avutil-59.dll
00007FFD76F00000-00007FFD774F2000 C:\Program Files\obs-studio\bin\64bit\Qt6Core.dll
00007FFE1BEC0000-00007FFE1BED9000 C:\Program Files\obs-studio\bin\64bit\zlib.dll
00007FFDDEA70000-00007FFDDEAAD000 C:\Program Files\obs-studio\bin\64bit\librist.dll
00007FFE1C9B0000-00007FFE1C9BD000 C:\WINDOWS\SYSTEM32\Secur32.dll
00007FFDDE780000-00007FFDDE82D000 C:\Program Files\obs-studio\bin\64bit\srt.dll
00007FFE1F1B0000-00007FFE1F1D6000 C:\WINDOWS\SYSTEM32\bcrypt.dll
00007FFE08830000-00007FFE08853000 C:\Program Files\obs-studio\bin\64bit\swresample-5.dll
00007FFE02BC0000-00007FFE02BDE000 C:\WINDOWS\SYSTEM32\VCRUNTIME140.dll
00007FFD7DF00000-00007FFD7E217000 C:\Program Files\obs-studio\bin\64bit\libx264-164.dll
00007FFDDCD90000-00007FFDDCE2C000 C:\Program Files\obs-studio\bin\64bit\swscale-8.dll
00007FFE1A0D0000-00007FFE1A0DB000 C:\WINDOWS\SYSTEM32\AVRT.dll
00007FFE1CF20000-00007FFE1CF55000 C:\WINDOWS\SYSTEM32\dwmapi.dll
00007FFE13640000-00007FFE13676000 C:\WINDOWS\SYSTEM32\WINMM.dll
00007FFE02AB0000-00007FFE02B3D000 C:\WINDOWS\SYSTEM32\MSVCP140.dll
00007FFE029C0000-00007FFE029CC000 C:\WINDOWS\SYSTEM32\VCRUNTIME140_1.dll
00007FFE1BDD0000-00007FFE1BDD9000 C:\WINDOWS\SYSTEM32\MSVCP140_1.dll
00007FFE1E160000-00007FFE1E193000 C:\WINDOWS\SYSTEM32\IPHLPAPI.DLL
00007FFE1E1A0000-00007FFE1E2C5000 C:\WINDOWS\SYSTEM32\DNSAPI.dll
00007FFE199A0000-00007FFE19ABE000 C:\WINDOWS\SYSTEM32\WINHTTP.dll
00007FFE1CB80000-00007FFE1CC2E000 C:\WINDOWS\SYSTEM32\UxTheme.dll
00007FFE1B520000-00007FFE1B786000 C:\WINDOWS\SYSTEM32\d3d11.dll
00007FFE06FA0000-00007FFE06FC3000 C:\WINDOWS\SYSTEM32\d3d12.dll
00007FFE1B2B0000-00007FFE1B519000 C:\WINDOWS\SYSTEM32\DWrite.dll
00007FFDDEA20000-00007FFDDEA61000 C:\WINDOWS\SYSTEM32\MSVCP140_2.dll
00007FFE00B20000-00007FFE00B41000 C:\WINDOWS\SYSTEM32\MPR.dll
00007FFE1ED70000-00007FFE1ED9B000 C:\WINDOWS\SYSTEM32\USERENV.dll
00007FFE1E330000-00007FFE1E380000 C:\WINDOWS\SYSTEM32\AUTHZ.dll
00007FFE02990000-00007FFE029AA000 C:\WINDOWS\SYSTEM32\NETAPI32.dll
00007FFE1A0C0000-00007FFE1A0CB000 C:\WINDOWS\SYSTEM32\VERSION.dll
00007FFE1EA50000-00007FFE1EA98000 C:\WINDOWS\SYSTEM32\SSPICLI.DLL
00007FFE1EF70000-00007FFE1EF7C000 C:\WINDOWS\SYSTEM32\CRYPTBASE.DLL
00007FFE06780000-00007FFE067A9000 C:\WINDOWS\SYSTEM32\SRVCLI.DLL
00007FFE1E2D0000-00007FFE1E2DD000 C:\WINDOWS\SYSTEM32\NETUTILS.DLL
00007FFE21FE0000-00007FFE2200F000 C:\WINDOWS\System32\IMM32.DLL
00007FFE1CD10000-00007FFE1CD6D000 C:\WINDOWS\SYSTEM32\directxdatabasehelper.dll
00007FFE1E7B0000-00007FFE1E7CA000 C:\WINDOWS\SYSTEM32\kernel.appcore.dll
00007FFE201C0000-00007FFE20259000 C:\WINDOWS\System32\bcryptPrimitives.dll
00007FFE21560000-00007FFE2156A000 C:\WINDOWS\System32\NSI.dll
00007FFE21060000-00007FFE2114B000 C:\WINDOWS\System32\shcore.dll
00007FFE0B550000-00007FFE0B593000 C:\WINDOWS\SYSTEM32\RTWorkQ.dll
00007FFE1D5A0000-00007FFE1DDE5000 C:\WINDOWS\SYSTEM32\windows.storage.dll
00007FFE20970000-00007FFE209CD000 C:\WINDOWS\System32\shlwapi.dll
00007FFE1F790000-00007FFE1F7BF000 C:\WINDOWS\SYSTEM32\profapi.dll
00007FFD90440000-00007FFD90519000 C:\Program Files\obs-studio\bin\64bit\platforms\qwindows.dll
00007FFE22150000-00007FFE225D6000 C:\WINDOWS\System32\SETUPAPI.dll
00007FFE20A50000-00007FFE20B41000 C:\WINDOWS\System32\COMDLG32.dll
00007FFE1CA60000-00007FFE1CA76000 C:\WINDOWS\SYSTEM32\WTSAPI32.dll
00007FFE09DE0000-00007FFE09E94000 C:\WINDOWS\WinSxS\amd64_microsoft.windows.common-controls_6595b64144ccf1df_5.82.26100.1882_none_87f34cef7a28f535\COMCTL32.dll
00007FFDCEFA0000-00007FFDCF14C000 C:\WINDOWS\SYSTEM32\d3d9.dll
00007FFE1CD70000-00007FFE1CDB7000 C:\WINDOWS\SYSTEM32\dxcore.dll
00007FFE1E530000-00007FFE1E57E000 C:\WINDOWS\SYSTEM32\powrprof.dll
00007FFE1E510000-00007FFE1E524000 C:\WINDOWS\SYSTEM32\UMPDC.dll
00007FFE21400000-00007FFE2155B000 C:\WINDOWS\System32\MSCTF.dll
00007FFE1F4E0000-00007FFE1F50D000 C:\WINDOWS\SYSTEM32\DEVOBJ.dll
00007FFE1F530000-00007FFE1F58F000 C:\WINDOWS\SYSTEM32\cfgmgr32.dll
00007FFE21780000-00007FFE21828000 C:\WINDOWS\System32\clbcatq.dll
00007FFE0B5A0000-00007FFE0B6F6000 C:\Windows\System32\Windows.UI.dll
00007FFDF48F0000-00007FFDF4A3C000 C:\Windows\System32\Windows.UI.Immersive.dll
00007FFE035A0000-00007FFE035C6000 C:\Program Files\obs-studio\bin\64bit\styles\qwindowsvistastyle.dll
00007FFE1A020000-00007FFE1A02E000 C:\Program Files\obs-studio\bin\64bit\imageformats\qgif.dll
00007FFE0A7C0000-00007FFE0A7CF000 C:\Program Files\obs-studio\bin\64bit\imageformats\qicns.dll
00007FFE0A7A0000-00007FFE0A7AE000 C:\Program Files\obs-studio\bin\64bit\imageformats\qico.dll
00007FFDDCF40000-00007FFDDCFCD000 C:\Program Files\obs-studio\bin\64bit\imageformats\qjpeg.dll
00007FFE09B00000-00007FFE09B0C000 C:\Program Files\obs-studio\bin\64bit\imageformats\qsvg.dll
00007FFE096D0000-00007FFE096DC000 C:\Program Files\obs-studio\bin\64bit\imageformats\qtga.dll
00007FFDDE460000-00007FFDDE4CA000 C:\Program Files\obs-studio\bin\64bit\imageformats\qtiff.dll
00007FFE09400000-00007FFE0940C000 C:\Program Files\obs-studio\bin\64bit\imageformats\qwbmp.dll
00007FFDDA330000-00007FFDDA3BA000 C:\Program Files\obs-studio\bin\64bit\imageformats\qwebp.dll
00007FFE14DC0000-00007FFE14E5A000 C:\WINDOWS\System32\MMDevApi.dll
00007FFE13E10000-00007FFE13FCB000 C:\WINDOWS\SYSTEM32\AUDIOSES.DLL
00007FFE0A230000-00007FFE0A4C0000 C:\WINDOWS\WinSxS\amd64_microsoft.windows.common-controls_6595b64144ccf1df_6.0.26100.3323_none_3e088096e3344490\comctl32.dll
00007FFE1B070000-00007FFE1B29E000 C:\WINDOWS\SYSTEM32\WindowsCodecs.dll
00007FFDF2E70000-00007FFDF2EE0000 C:\Windows\System32\thumbcache.dll
00007FFE1AAE0000-00007FFE1ABA2000 C:\WINDOWS\SYSTEM32\policymanager.dll
00007FFE1AA40000-00007FFE1AAD1000 C:\WINDOWS\SYSTEM32\msvcp110_win.dll
00007FFE046B0000-00007FFE0470A000 C:\WINDOWS\system32\dataexchange.dll
00007FFE13400000-00007FFE1363B000 C:\WINDOWS\system32\twinapi.appcore.dll
00007FFE03600000-00007FFE03631000 C:\WINDOWS\SYSTEM32\winmmbase.dll
00007FFDDE260000-00007FFDDE2A8000 C:\WINDOWS\SYSTEM32\wdmaud.drv
00007FFE05B60000-00007FFE05CA6000 C:\WINDOWS\SYSTEM32\textinputframework.dll
00007FFE1C5E0000-00007FFE1C706000 C:\WINDOWS\SYSTEM32\CoreMessaging.dll
00007FFE19420000-00007FFE19703000 C:\WINDOWS\SYSTEM32\CoreUIComponents.dll
00007FFE09160000-00007FFE0916F000 C:\WINDOWS\SYSTEM32\msacm32.drv
00007FFDFFAB0000-00007FFDFFAD1000 C:\WINDOWS\SYSTEM32\MSACM32.dll
00007FFE088E0000-00007FFE088EC000 C:\WINDOWS\SYSTEM32\midimap.dll
00007FFE1D1B0000-00007FFE1D1C4000 C:\WINDOWS\SYSTEM32\resourcepolicyclient.dll
00007FFE08920000-00007FFE08966000 C:\WINDOWS\SYSTEM32\wscapi.dll
00007FFE07360000-00007FFE07374000 C:\Program Files\obs-studio\bin\64bit\iconengines\qsvgicon.dll
00007FFE1E8D0000-00007FFE1E905000 C:\WINDOWS\SYSTEM32\ntmarta.dll
00007FFDDA770000-00007FFDDA7A4000 C:\Program Files\obs-studio\bin\64bit\libobs-d3d11.dll
00007FFE1ABC0000-00007FFE1B03F000 C:\WINDOWS\SYSTEM32\D3DCOMPILER_47.dll
00007FFE1EF80000-00007FFE1EF9C000 C:\WINDOWS\SYSTEM32\cryptsp.dll
00007FFE18BD0000-00007FFE18C92000 C:\WINDOWS\System32\DriverStore\FileRepository\nvmdi.inf_amd64_a2eeb2756802bbd3\nvldumdx.dll
00007FFE1F090000-00007FFE1F0A3000 C:\WINDOWS\SYSTEM32\msasn1.dll
00007FFE190A0000-00007FFE190DB000 C:\WINDOWS\SYSTEM32\cryptnet.dll
00007FFE1F030000-00007FFE1F08D000 C:\WINDOWS\SYSTEM32\wldp.dll
00007FFE18D30000-00007FFE18EAB000 C:\WINDOWS\SYSTEM32\drvstore.dll
00007FFE20130000-00007FFE201AF000 C:\WINDOWS\System32\wintrust.dll
00007FFE21580000-00007FFE215A0000 C:\WINDOWS\System32\imagehlp.dll
00007FFE1E710000-00007FFE1E74A000 C:\WINDOWS\system32\rsaenh.dll
00007FFE14E60000-00007FFE185FB000 C:\WINDOWS\System32\DriverStore\FileRepository\nvmdi.inf_amd64_a2eeb2756802bbd3\nvgpucomp64.dll
00007FFE13680000-00007FFE13757000 C:\WINDOWS\System32\DriverStore\FileRepository\nvmdi.inf_amd64_a2eeb2756802bbd3\NvMemMapStoragex.dll
00007FFE0C250000-00007FFE11563000 C:\WINDOWS\System32\DriverStore\FileRepository\nvmdi.inf_amd64_a2eeb2756802bbd3\nvwgf2umx.dll
00007FFE04920000-00007FFE04937000 C:\Program Files\obs-studio\bin\64bit\libobs-winrt.dll
00007FFD76960000-00007FFD76B6C000 C:\Program Files\obs-studio\obs-plugins\64bit\aja-output-ui.dll
00007FFD76710000-00007FFD76952000 C:\Program Files\obs-studio\obs-plugins\64bit\aja.dll
00007FFE03C00000-00007FFE03C13000 C:\Program Files\obs-studio\obs-plugins\64bit\coreaudio-encoder.dll
00007FFE088D0000-00007FFE088DF000 C:\Program Files\obs-studio\obs-plugins\64bit\decklink-captions.dll
00007FFDDA290000-00007FFDDA2C5000 C:\Program Files\obs-studio\obs-plugins\64bit\decklink-output-ui.dll
00007FFE03B00000-00007FFE03B16000 C:\Program Files\obs-studio\obs-plugins\64bit\decklink.dll
00007FFDCE4C0000-00007FFDCE51E000 C:\Program Files\obs-studio\obs-plugins\64bit\frontend-tools.dll
00007FFDF03C0000-00007FFDF03E4000 C:\Program Files\obs-studio\bin\64bit\obs-scripting.dll
00007FFD8EFA0000-00007FFD8F034000 C:\Program Files\obs-studio\bin\64bit\lua51.dll
00007FFE034F0000-00007FFE03501000 C:\Program Files\obs-studio\obs-plugins\64bit\image-source.dll
00007FFDE2630000-00007FFDE2642000 C:\Program Files\obs-studio\obs-plugins\64bit\nv-filters.dll
00007FFD8E7D0000-00007FFD8E898000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-browser.dll
00007FFD3F750000-00007FFD4C58A000 C:\Program Files\obs-studio\obs-plugins\64bit\libcef.dll
00007FFE11570000-00007FFE117B1000 C:\WINDOWS\SYSTEM32\dbghelp.dll
00007FFD8B4F0000-00007FFD8B646000 C:\Program Files\obs-studio\obs-plugins\64bit\chrome_elf.dll
00007FFE0A170000-00007FFE0A22A000 C:\WINDOWS\SYSTEM32\WINSPOOL.DRV
00007FFE193F0000-00007FFE19415000 C:\WINDOWS\SYSTEM32\dhcpcsvc.DLL
00007FFE1F5A0000-00007FFE1F5AA000 C:\WINDOWS\SYSTEM32\DPAPI.DLL
00007FFDCE480000-00007FFDCE4BF000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-ffmpeg.dll
00007FFDDEE90000-00007FFDDEEA7000 C:\Program Files\obs-studio\bin\64bit\avdevice-61.dll
00007FFD721C0000-00007FFD72845000 C:\Program Files\obs-studio\bin\64bit\avfilter-10.dll
00007FFDDEA00000-00007FFDDEA1B000 C:\WINDOWS\SYSTEM32\AVICAP32.dll
00007FFDDE750000-00007FFDDE77C000 C:\WINDOWS\SYSTEM32\MSVFW32.dll
00007FFD93780000-00007FFD937CD000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-filters.dll
00007FFDDE240000-00007FFDDE255000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-nvenc.dll
00007FFD87260000-00007FFD87361000 C:\WINDOWS\SYSTEM32\nvEncodeAPI64.dll
00007FFD8E710000-00007FFD8E7CA000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-outputs.dll
00007FFD8FD60000-00007FFD8FDB5000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-qsv11.dll
00007FFE08760000-00007FFE0876F000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-text.dll
00007FFE0A4D0000-00007FFE0A6A6000 C:\WINDOWS\WinSxS\amd64_microsoft.windows.gdiplus_6595b64144ccf1df_1.1.26100.3323_none_6ef3737c3dc6be5e\gdiplus.dll
00007FFE08720000-00007FFE0872E000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-transitions.dll
00007FFDDDF30000-00007FFDDDF43000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-vst.dll
00007FFDDA020000-00007FFDDA03C000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-webrtc.dll
00007FFD71E00000-00007FFD71FF3000 C:\Program Files\obs-studio\bin\64bit\datachannel.dll
00007FFD84190000-00007FFD842B0000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-websocket.dll
00007FFE1ECD0000-00007FFE1ED3A000 C:\WINDOWS\SYSTEM32\MSWSOCK.dll
00007FFE070F0000-00007FFE070FC000 C:\Program Files\obs-studio\obs-plugins\64bit\obs-x264.dll
00007FFDD92C0000-00007FFDD92D5000 C:\Program Files\obs-studio\obs-plugins\64bit\rtmp-services.dll
00007FFD8E310000-00007FFD8E3B0000 C:\Program Files\obs-studio\obs-plugins\64bit\text-freetype2.dll
00007FFE18810000-00007FFE1881B000 C:\Windows\System32\rasadhlp.dll
00007FFE06D30000-00007FFE06D3D000 C:\Program Files\obs-studio\obs-plugins\64bit\vlc-video.dll
00007FFE18CA0000-00007FFE18D25000 C:\WINDOWS\System32\fwpuclnt.dll
00007FFDCE6B0000-00007FFDCE6D1000 C:\Program Files\obs-studio\obs-plugins\64bit\win-capture.dll
00007FFDCE3C0000-00007FFDCE3F6000 C:\Program Files\obs-studio\obs-plugins\64bit\win-dshow.dll
00007FFE1DDF0000-00007FFE1DDFF000 C:\WINDOWS\SYSTEM32\HID.DLL
00007FFE09130000-00007FFE09151000 C:\Windows\System32\devenum.dll
00007FFE03710000-00007FFE0371E000 C:\WINDOWS\SYSTEM32\msdmo.dll
00007FFDD8D70000-00007FFDD8D86000 C:\Program Files\obs-studio\obs-plugins\64bit\win-wasapi.dll
00007FFD8E2B0000-00007FFD8E30F000 C:\ProgramData\obs-studio\plugins\obs-multi-rtmp\bin\64bit\obs-multi-rtmp.dll
00007FFE05D70000-00007FFE05D7A000 C:\ProgramData\obs-studio\plugins\playlist\bin\64bit\playlist.dll
00007FFE0BA10000-00007FFE0BBFB000 C:\WINDOWS\System32\audioeng.dll
00007FFE1A430000-00007FFE1A52C000 C:\WINDOWS\System32\PROPSYS.dll
00007FFE1E5F0000-00007FFE1E6BA000 C:\WINDOWS\system32\schannel.DLL
00007FFE1F170000-00007FFE1F1A0000 C:\WINDOWS\SYSTEM32\ncrypt.dll
00007FFE1F120000-00007FFE1F15F000 C:\WINDOWS\SYSTEM32\NTASN1.dll
00007FFE020E0000-00007FFE0210D000 C:\WINDOWS\system32\ncryptsslp.dll
00007FFDE5FB0000-00007FFDE5FE9000 C:\WINDOWS\SYSTEM32\dbgcore.DLL
00007FFDC97E0000-00007FFDCB8A7000 C:\WINDOWS\System32\DriverStore\FileRepository\nvmdi.inf_amd64_a2eeb2756802bbd3\nvd3dumx.dll
00007FFE1BF20000-00007FFE1C144000 C:\WINDOWS\SYSTEM32\dcomp.dll
00007FFE1B050000-00007FFE1B069000 C:\WINDOWS\SYSTEM32\Microsoft.Internal.WarpPal.dll
*/