#ifndef UTILS_H
#define UTILS_H

#include <obs-module.h>
#include <plugin-support.h>
#include <util/darray.h>

typedef struct {
	char *path;
	char *filename; // filename with ext, ONLY for folder item checking
	// char *id;
	bool is_url;
	bool is_folder;
	// MediaFileDataArray folder_items;
	// struct MediaFileData *parent;
	// const char *parent_id; // for folder items
	size_t index; // makes it easier to switch back to non-shuffle mode
} MediaFileData;

// typedef struct {
// 	MediaFileData *data;
// 	size_t size;
// 	size_t capacity;
// } MediaFileDataArray;
typedef DARRAY(MediaFileData) MediaFileDataArray;

char *obs_array_to_string(obs_data_array_t *array);

// Function to initialize the dynamic string array
// void init_media_array(MediaFileDataArray *media_array, size_t initial_capacity);

// Function to add a media from a string path to the end of array
void push_media_back(MediaFileDataArray *media_array, const char *path);

// Function to add a media from a string path to the front of array
void push_media_front(MediaFileDataArray *media_array, const char *path);

// Function to add a media from a string path to the array at a specific index
void push_media_at(MediaFileDataArray *media_array, const char *path, size_t index);

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

const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index);
// Function to free the dynamic string array
void free_media_array(MediaFileDataArray *media_array);

// Turns an obs_data_array_t into a MediaFileDataArray
MediaFileDataArray *obs_data_array_retain(MediaFileDataArray *media_file_data_array, obs_data_array_t *obs_playlist);

// Turns the media array into a string
char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const char *indent,
			    bool only_file_name);

// Logs the media array in the obs log files
void obs_log_media_array(int log_level, const MediaFileDataArray *media_array, size_t threshold, const char *indent,
			 bool only_file_name);

#endif // UTILS_H
