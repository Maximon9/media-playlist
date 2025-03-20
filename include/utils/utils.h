#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <plugin-support.h>
#include <util/darray.h>
#include <util/dstr.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <util/platform.h>

static const char *media_filter =
	" (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm *.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";
static const char *video_filter = " (*.mp4 *.mpg *.m4v *.ts *.mov *.mxf *.flv *.mkv *.avi *.gif *.webm);;";
static const char *audio_filter = " (*.mp3 *.m4a *.mka *.ogg *.aac *.wav *.opus *.flac);;";

typedef struct {
	char *path;
	char *filename;
	char *name;
	char *ext;
	size_t index;
} MediaFileData;

typedef DARRAY(MediaFileData) MediaFileDataArray;

#pragma region Media Array Utils
static char *obs_array_to_string(obs_data_array_t *array);

static void push_media_back(MediaFileDataArray *media_array, const char *path);

static void push_media_front(MediaFileDataArray *media_array, const char *path);

static void push_media_at(MediaFileDataArray *media_array, const char *path, size_t index);

static const MediaFileData create_media_file_data_from_path(const char *path, size_t index);

static const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index);

void clear_media_array(MediaFileDataArray *media_array);

void free_media_array(MediaFileDataArray *media_array);

/*  */
// Returns true if extension is valid
static bool valid_extension(const char *ext);

// Turns an obs_data_array_t into a MediaFileDataArray
void obs_data_media_array_retain(MediaFileDataArray *media_file_data_array, obs_data_array_t *obs_playlist);

// Turns the media array into a string
static char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const char *indent,
				   bool only_file_name);

// Logs the media array in the obs log files
void obs_log_media_array(int log_level, char *format, const MediaFileDataArray *media_array, size_t threshold,
			 const char *indent, bool only_file_name);
#pragma endregion

#pragma region Utils
char *screaming_snake_case_to_title_case(const char *name);

void add_enums_to_property_list(obs_property_t *property, const char *StartBehavior[]);
#endif; // UTILS_H
