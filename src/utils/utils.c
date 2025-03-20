#pragma region Main

#include "../include/utils/utils.h"

#pragma region Media Array Utils
static char *obs_array_to_string(obs_data_array_t *array)
{
	size_t array_size = obs_data_array_count(array);

	if (array == NULL || array_size == 0) {
		return "[]";
	}

	// Estimate maximum size for the string (2 chars per element + commas + brackets)
	size_t estimated_size = array_size * 2 + array_size - 1 + 2; // Two chars per element + commas + brackets
	char *result = (char *)malloc(estimated_size * sizeof(char));

	// Start the string with the opening bracket
	strcpy(result, "[");

	// Loop through the array and append each element
	for (size_t i = 0; i < array_size; ++i) {
		// Convert element to string (single character)
		obs_data_t *data = obs_data_array_item(array, i);
		const char *element[2] = {obs_data_get_string(data, "value"), '\0'}; // Create a single char string

		strcat(result, *element);

		// Add a comma if not the last element
		if (i < array_size - 1) {
			strcat(result, ", ");
		}
	}

	// End the string with the closing bracket
	strcat(result, "]");

	return result;
	return "[]";
}

static void push_media_back(MediaFileDataArray *media_array, const char *path)
{
	push_media_at(media_array, path, media_array->num);
}

static void push_media_front(MediaFileDataArray *media_array, const char *path)
{
	push_media_at(media_array, path, 0);
}

static void push_media_at(MediaFileDataArray *media_array, const char *path, size_t index)
{
	const MediaFileData new_entry = create_media_file_data_from_path(path, index);
	da_insert(*media_array, index, &new_entry);
}

static const MediaFileData create_media_file_data_from_path(const char *path, size_t index)
{
	// Create and insert new MediaFileData
	const char *last_slash = strrchr(path, '/');
	const char *temp_file_name = last_slash ? last_slash + 1 : path;

	char *file_name = strdup(temp_file_name);

	const char *dot_pos = strrchr(temp_file_name, '.');

	char *ext = strdup(dot_pos ? dot_pos + 1 : "None");

	char *name;
	if (dot_pos != NULL) {
		// Calculate the length of the name (without the extension)
		size_t name_len = dot_pos - temp_file_name; // Length of name without the extension
		name = malloc(name_len + 1);                // Allocate memory for the name part

		if (name != NULL) {
			memcpy(name, temp_file_name, name_len); // Copy the name part
			name[name_len] = '\0';                  // Null-terminate the name
		}
	} else {
		name = strdup(file_name); // If no extension, the full name is used
	}

	const MediaFileData new_entry = {.path = strdup(path),
					 .filename = file_name,
					 .name = name,
					 .ext = ext,
					 .index = index};
	return new_entry;
}

static const MediaFileData create_media_file_data_from_path_and_file_name(const char *path, const char *file_name,
									  size_t index)
{
	// Create and insert new MediaFileData
	const char *dot_pos = strrchr(file_name, '.');

	char *ext = strdup(dot_pos ? dot_pos + 1 : "None");

	char *name;
	if (dot_pos != NULL) {
		// Calculate the length of the name (without the extension)
		size_t name_len = dot_pos - file_name; // Length of name without the extension
		name = malloc(name_len + 1);           // Allocate memory for the name part

		if (name != NULL) {
			memcpy(name, file_name, name_len); // Copy the name part
			name[name_len] = '\0';             // Null-terminate the name
		}
	} else {
		name = strdup(file_name); // If no extension, the full name is used
	}

	const MediaFileData new_entry = {.path = strdup(path),
					 .filename = strdup(file_name),
					 .name = name,
					 .ext = ext,
					 .index = index};
	return new_entry;
}

static const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index)
{
	if (index >= media_array->num)
		return NULL; // Out of bounds
	return &media_array->array[index];
}

void clear_media_array(MediaFileDataArray *media_array)
{
	if (media_array != NULL) {
		if (media_array->num > 0) {
			for (size_t i = 0; i < media_array->num; i++) {
				free(get_media(media_array, i)->path);
				free(get_media(media_array, i)->filename);
				free(get_media(media_array, i)->name);
				free(get_media(media_array, i)->ext);
			}
		}
		da_clear(*media_array);
	}
}

void free_media_array(MediaFileDataArray *media_array)
{
	if (media_array != NULL) {
		if (media_array->num > 0) {
			for (size_t i = 0; i < media_array->num; i++) {
				free(get_media(media_array, i)->path);
				free(get_media(media_array, i)->filename);
				free(get_media(media_array, i)->name);
				free(get_media(media_array, i)->ext);
			}
		}
		da_free(*media_array);
	}
}

static bool valid_extension(const char *ext)
{
	struct dstr haystack = {0};
	struct dstr needle = {0};
	bool valid = false;

	if (!ext || !*ext)
		return false;

	dstr_copy(&haystack, media_filter);
	dstr_cat(&haystack, video_filter);
	dstr_cat(&haystack, audio_filter);

	dstr_cat_ch(&needle, '*');
	dstr_cat(&needle, ext);

	valid = dstr_find_i(&haystack, needle.array);

	dstr_free(&haystack);
	dstr_free(&needle);
	return valid;
}

void obs_data_media_array_retain(MediaFileDataArray *media_file_data_array, obs_data_array_t *obs_playlist)
{
	size_t array_size = obs_data_array_count(obs_playlist);
	if (array_size == 0) {
		obs_data_array_release(obs_playlist);
		return;
	}

	for (size_t i = 0; i < array_size; ++i) {
		obs_data_t *data = obs_data_array_item(obs_playlist, i);

		if (data == NULL) {
			continue; // Skip if data is NULL (avoid potential crash or issues)
		}

		const char *element = obs_data_get_string(data, "value");
		if (element == NULL) {
			obs_data_release(data); // Release memory for the current element before skipping
			continue;               // Skip if no valid string was found
		}

		os_dir_t *dir = os_opendir(element);

		if (dir) {
			struct os_dirent *entry;
			while (true) {

				const char *ext;

				entry = os_readdir(dir);
				if (!entry)
					break;

				if (entry->directory)
					continue;

				ext = os_get_path_extension(entry->d_name);
				if (!valid_extension(ext))
					continue;

				// Allocate memory dynamically for full path
				size_t path_len =
					strlen(element) + strlen(entry->d_name) + 2; // +2 for '/' and null terminator
				char *full_path = malloc(path_len);
				if (!full_path) {
					obs_log(LOG_INFO, "Memory allocation failed for: %s\n", entry->d_name);
					continue;
				}
				snprintf(full_path, path_len, "%s/%s", element, entry->d_name);

				// obs_log(LOG_INFO, "\nFound file: \n%s\n%s\n%s\n", element, entry->d_name, full_path);

				const MediaFileData new_entry = create_media_file_data_from_path_and_file_name(
					full_path, entry->d_name, media_file_data_array->num);
				da_insert(*media_file_data_array, media_file_data_array->num, &new_entry);

				// Free allocated memory
				free(full_path);
			}

			os_closedir(dir);
		} else {
			push_media_back(media_file_data_array, element);
		}

		obs_data_release(data);
	}
	obs_data_array_release(obs_playlist);
}

static char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const char *indent,
				   bool only_file_name)
{
	if (media_array == NULL || media_array->num <= 0) {
		return strdup("[]"); // Return empty brackets if no elements
	}
	// Calculate the initial length of the compact format
	size_t total_length = 3; // For "[" and "]\0"
	for (size_t i = 0; i < media_array->num; i++) {
		if (only_file_name) {
			total_length += strlen(get_media(media_array, i)->filename) + 4; // Quotes, comma, space
		} else {
			total_length += strlen(get_media(media_array, i)->path) + 4; // Quotes, comma, space
		}
	}

	// Allocate memory for the final string (before prettification)
	char *result = (char *)malloc(total_length * sizeof(char));
	if (!result)
		return NULL; // Handle allocation failure

	// Apply compact format first
	strcpy(result, "[");
	for (size_t i = 0; i < media_array->num; i++) {
		strcat(result, "\"");
		if (only_file_name) {
			strcat(result, get_media(media_array, i)->filename);
		} else {
			strcat(result, get_media(media_array, i)->path);
		}
		strcat(result, "\"");
		if (i < media_array->num - 1)
			strcat(result, ", ");
	}
	strcat(result, "]");

	// If the compact string exceeds the threshold, prettify the result
	if (strlen(result) > threshold) {
		size_t prettified_length = 3; // For "[\n" and "]\n"
		for (size_t i = 0; i < media_array->num; i++) {
			prettified_length += strlen(indent) + strlen(get_media(media_array, i)->path) +
					     4; // Indent, quotes, and comma
		}

		// Allocate memory for the prettified string
		char *prettified_result = (char *)malloc(prettified_length * sizeof(char));
		if (!prettified_result) {
			free(result); // Free the compact string in case of failure
			return NULL;
		}

		// Construct the prettified string
		strcpy(prettified_result, "[\n");
		for (size_t i = 0; i < media_array->num; i++) {
			strcat(prettified_result, indent); // Add indentation before each element
			strcat(prettified_result, "\"");
			if (only_file_name) {
				strcat(prettified_result, get_media(media_array, i)->filename);
			} else {
				strcat(prettified_result, get_media(media_array, i)->path);
			}
			strcat(prettified_result, "\"");

			if (i < media_array->num - 1) {
				strcat(prettified_result, ",\n");
			}
		}

		strcat(prettified_result, "\n]");

		free(result); // Free the original compact string

		return prettified_result;
	}

	return result; // Return the compact string if it's within the threshold
}

void obs_log_media_array(int log_level, char *format, const MediaFileDataArray *media_array, size_t threshold,
			 const char *indent, bool only_file_name)
{
	char *result = stringify_media_array(media_array, threshold, indent, only_file_name);

	size_t total_length = strlen(format) + strlen(result) + 1;
	char *concat_result = malloc(total_length);

	if (!concat_result) {
		perror("Failed to allocate memory");
		free(result);
		return;
	}

	strcpy(concat_result, format);
	strcat(concat_result, result);

	obs_log(log_level, concat_result);

	free(result);
	free(concat_result);
}
#pragma endregion

#pragma region Utils
char *screaming_snake_case_to_title_case(const char *name)
{
	if (!name)
		return NULL;

	size_t len = strlen(name);
	char *output = malloc(len + 1); // Allocate memory for new string
	if (!output)
		return NULL;

	int capitalize_next = 1; // Ensure first letter is uppercase

	for (size_t i = 0; i < len; i++) {
		if (name[i] == '_') {
			output[i] = ' ';     // Replace underscore with space
			capitalize_next = 1; // Next letter should be capitalized
		} else {
			output[i] = capitalize_next ? toupper(name[i]) : tolower(name[i]);
			capitalize_next = 0;
		}
	}

	output[len] = '\0'; // Null-terminate the string
	return output;
}

void add_enums_to_property_list(obs_property_t *property, const char *Enum[])
{
	long long i = 0;
	const char *name = Enum[i];
	while (name != "") {
		char *display_name = screaming_snake_case_to_title_case(name);
		if (!display_name) {
			continue;
		}
		obs_property_list_add_int(property, display_name, i);
		i++;
		name = Enum[i];
		free(display_name);
	}
}
#pragma endregion

#pragma endregion