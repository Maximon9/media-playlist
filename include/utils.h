#pragma region Main
#include <obs-module.h>
#include <plugin-support.h>

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

typedef struct {
	MediaFileData *data;
	size_t size;
	size_t capacity;
} MediaFileDataArray;

char *obs_array_to_string(obs_data_array_t *array);

// Function to initialize the dynamic string array
void init_media_array(MediaFileDataArray *media_array, size_t initial_capacity);

// Function to add a media from a string path to the array
void add_media_at(MediaFileDataArray *media_array, const char *path, size_t index);

// Function to add a media from a MediaFileData to the array
void add_media_file_data_at(MediaFileDataArray *media_array, MediaFileData media_file_data, size_t index);

void remove_media_at(MediaFileDataArray *media_array, size_t index);

// Function to get a media by index
const MediaFileData *get_media(MediaFileDataArray *media_array, size_t index);

// Function to free the dynamic media array
void free_media_array(MediaFileDataArray *media_array);

// Turns the media array into a string
char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const char *indent);

// Logs the media array in the obs log files
void obs_log_media_array(int log_level, const MediaFileDataArray *media_array, size_t threshold, const char *indent);
#pragma endregion