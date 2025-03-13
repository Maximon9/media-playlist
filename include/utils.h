#pragma region Main

#include <obs-module.h>
#include <plugin-support.h>

typedef struct {
	char **data;     // Array of string pointers
	size_t size;     // Number of strings stored
	size_t capacity; // Total allocated slots
} StringArray;

char *obs_array_to_string(obs_data_array_t *array);

// Function to initialize the dynamic string array
void init_string_array(StringArray *string_array, size_t initial_capacity);

// Function to add a string to the array
void add_string(StringArray *string_array, const char *str);

// Function to get a string by index
const char *get_string(const StringArray *string_array, size_t index);

// Function to free the dynamic string array
void free_string_array(StringArray *string_array);

char *stringify_string_array(const StringArray *string_array, size_t threshold, const char *indent);

void obs_log_string_array(int log_level, const StringArray *string_array, size_t threshold, const char *indent);

#pragma endregion