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
	// bool is_folder;
	// const MediaFileData *parent;
	size_t index;
} MediaFileData;

// typedef struct {
// 	MediaFileData *data;
// 	size_t size;
// 	size_t capacity;
// } MediaFileDataArray;
typedef DARRAY(MediaFileData) MediaFileDataArray;

static char *obs_array_to_string(obs_data_array_t *array);

// Function to initialize the dynamic string array
// void init_media_array(MediaFileDataArray *media_array, size_t initial_capacity);

// Function to add a media from a string path to the end of array
static void push_media_back(MediaFileDataArray *media_array, const char *path);

// Function to add a media from a string path to the front of array
static void push_media_front(MediaFileDataArray *media_array, const char *path);

// Function to add a media from a string path to the array at a specific index
static void push_media_at(MediaFileDataArray *media_array, const char *path, size_t index);

static const MediaFileData create_media_file_data_from_path(const char *path, size_t index);
/*
// Function to add a media from a MediaFileData to the end of array
void push_media_file_data_back(MediaFileDataArray *media_array, MediaFileData media_file_data);

// Function to add a media from a MediaFileData to the front of array
void push_media_file_data_front(MediaFileDataArray *media_array, MediaFileData media_file_data);

// Function to add a media from a MediaFileData to the array at a specific index
void push_media_file_data_at(MediaFileDataArray *media_array, MediaFileData media_file_data, size_t index);

// Function to removes a media from the end of the array
void pop_media_back(MediaFileDataArray *media_array);

// Function to removes a media from the start of the array
void pop_media_front(MediaFileDataArray *media_array);

// Function to removes a media from an index of the array
void pop_media_at(MediaFileDataArray *media_array, size_t index);

// Function to get a media by index
const MediaFileData *get_media(MediaFileDataArray *media_array, size_t index);

Function to free the dynamic media array
void free_media_array(MediaFileDataArray *media_array);
 */

static const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index);

char *upper_snake_case_to_title_case(const char *name);

// Function to clear the dynamic media file data array
void clear_media_array(MediaFileDataArray *media_array);

// Function to free the dynamic media file data array
void free_media_array(MediaFileDataArray *media_array);

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

#endif; // UTILS_H
