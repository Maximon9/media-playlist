#pragma region Main

#include "../include/utils/utils.h"

#pragma region Media Array Utils

void push_media_back(MediaFileDataArray *media_array, const char *path)
{
	push_media_at(media_array, path, media_array->num);
}

void push_media_front(MediaFileDataArray *media_array, const char *path)
{
	push_media_at(media_array, path, 0);
}

void push_media_at(MediaFileDataArray *media_array, const char *path, size_t index)
{
	const MediaFileData new_entry = create_media_file_data_from_path(path, index);
	da_insert(*media_array, index, &new_entry);
}

void pop_media_back(MediaFileDataArray *media_array)
{
	pop_media_at(media_array, media_array->num);
}

void pop_media_front(MediaFileDataArray *media_array)
{
	pop_media_at(media_array, 0);
}

void pop_media_at(MediaFileDataArray *media_array, size_t index)
{
	const MediaFileData *data = get_media(media_array, index);
	if (!data)
		return;
	free(data->path);
	free(data->filename);
	free(data->name);
	free(data->ext);
	da_erase(*media_array, index);
}

const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index)
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
				const MediaFileData *data = get_media(media_array, i);
				if (!data)
					return;
				free(data->path);
				free(data->filename);
				free(data->name);
				free(data->ext);
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
				const MediaFileData *data = get_media(media_array, i);
				if (!data)
					return;
				free(data->path);
				free(data->filename);
				free(data->name);
				free(data->ext);
			}
		}
		da_free(*media_array);
	}
}

const MediaFileData create_media_file_data_from_path(const char *path, size_t index)
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

const MediaFileData create_media_file_data_from_path_and_file_name(const char *path, const char *file_name,
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

const MediaFileData create_media_file_data_with_all_info(const char *path, const char *file_name, const char *name,
							 const char *ext, size_t index)
{
	// Create and insert new MediaFileData
	const MediaFileData new_entry = {.path = strdup(path),
					 .filename = strdup(file_name),
					 .name = strdup(name),
					 .ext = strdup(ext),
					 .index = index};
	return new_entry;
}

char *obs_array_to_string(obs_data_array_t *array)
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

bool valid_extension(const char *ext)
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

char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const char *indent,
			    e_MediaStringifyTYPE media_stringify_type)
{
	if (media_array == NULL || media_array->num <= 0) {
		return strdup("[]"); // Return empty brackets if no elements
	}
	// Calculate the initial length of the compact format
	size_t total_length = 3; // For "[" and "]\0"
	for (size_t i = 0; i < media_array->num; i++) {
		const MediaFileData *media_file_data = get_media(media_array, i);

		switch (media_stringify_type) {
		case MEDIA_STRINGIFY_TYPE_PATH:
			total_length += strlen(media_file_data->path) + 4; // Quotes, comma, space
			break;
		case MEDIA_STRINGIFY_TYPE_FILENAME:
			total_length += strlen(media_file_data->filename) + 4; // Quotes, comma, space
			break;
		case MEDIA_STRINGIFY_TYPE_NAME:
			total_length += strlen(media_file_data->name) + 4; // Quotes, comma, space
			break;
		default:
			break;
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

		const MediaFileData *media_file_data = get_media(media_array, i);

		switch (media_stringify_type) {
		case MEDIA_STRINGIFY_TYPE_PATH:
			strcat(result, media_file_data->path);
			break;
		case MEDIA_STRINGIFY_TYPE_FILENAME:
			strcat(result, media_file_data->filename);
			break;
		case MEDIA_STRINGIFY_TYPE_NAME:
			strcat(result, media_file_data->name);
			break;
		default:
			break;
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

			const MediaFileData *media_file_data = get_media(media_array, i);

			switch (media_stringify_type) {
			case MEDIA_STRINGIFY_TYPE_PATH:
				strcat(prettified_result, media_file_data->path);
				break;
			case MEDIA_STRINGIFY_TYPE_FILENAME:
				strcat(prettified_result, media_file_data->filename);
				break;
			case MEDIA_STRINGIFY_TYPE_NAME:
				strcat(prettified_result, media_file_data->name);
				break;
			default:
				break;
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
char *stringify_media_queue_array(const MediaFileDataArray *media_array, int queue_limit, const char *indent,
				  e_MediaStringifyTYPE media_stringify_type)
{
	size_t queue_size_list = (size_t)(queue_limit);

	if (media_array == NULL || media_array->num <= 0) {
		return strdup("[]"); // Return empty brackets if no elements
	}

	size_t min_size = min(media_array->num, queue_size_list);
	size_t prettified_length = 3; // For "[\n" and "]\n"
	for (size_t i = 0; i < min_size; i++) {
		prettified_length +=
			strlen(indent) + strlen(get_media(media_array, i)->path) + 4; // Indent, quotes, and comma
	}

	// Allocate memory for the prettified string
	char *prettified_result = (char *)malloc(prettified_length * sizeof(char));
	if (!prettified_result) {
		return NULL;
	}

	// Construct the prettified string
	strcpy(prettified_result, "[\n");

	for (size_t i = 0; i < min_size; i++) {
		if (i >= (size_t)queue_limit) {
			break;
		}
		strcat(prettified_result, indent); // Add indentation before each element
		strcat(prettified_result, "\"");

		const MediaFileData *media_file_data = get_media(media_array, i);

		switch (media_stringify_type) {
		case MEDIA_STRINGIFY_TYPE_PATH:
			strcat(prettified_result, media_file_data->path);
			break;
		case MEDIA_STRINGIFY_TYPE_FILENAME:
			strcat(prettified_result, media_file_data->filename);
			break;
		case MEDIA_STRINGIFY_TYPE_NAME:
			strcat(prettified_result, media_file_data->name);
			break;
		default:
			break;
		}
		strcat(prettified_result, "\"");

		if (i < (min_size - 1)) {
			strcat(prettified_result, ",\n");
		}
	}

	strcat(prettified_result, "\n]");

	return prettified_result;
}

void obs_log_media_array(int log_level, char *format, const MediaFileDataArray *media_array, size_t threshold,
			 const char *indent, e_MediaStringifyTYPE media_stringify_type)
{
	char *result = stringify_media_array(media_array, threshold, indent, media_stringify_type);

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

bool compare_media_file_data(const MediaFileData *data_1, const MediaFileData *data_2)
{
	if (strcmp(data_1->path, data_2->path) == 0 && data_1->index == data_2->index) {
		return true;
	}
	return false;
}

bool compare_media_file_data_arrays(MediaFileDataArray *array_1, MediaFileDataArray *array_2)
{
	if (array_1->num == array_2->num) {
		for (size_t i = 0; i < array_1->num; i++) {
			const MediaFileDataArray *data_1 = get_media(array_1, i);
			const MediaFileDataArray *data_2 = get_media(array_2, i);
			if (compare_media_file_data(data_1, data_2) == false) {
				return false;
			}
		}
		return true;
	}
	return false;
}

#pragma endregion

#pragma region Utils

char *concat_mem_string(char *_Destination, const char *_Source)
{
	size_t total_length = strlen(_Destination) + strlen(_Source) + 1;
	char *concat_result = malloc(total_length);

	if (!concat_result) {
		perror("Failed to allocate memory");
		return NULL;
	}

	strcpy(concat_result, _Destination);
	strcat(concat_result, _Source);
	return concat_result;
}

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

void remove_front_words(char *str, int count)
{
	if (!str || !count)
		return;

	char *token;
	int w_count = 0;
	size_t result_size = 256;           // Initial size of the result string
	char *result = malloc(result_size); // Allocate memory for the result string
	if (result == NULL) {
		printf("Memory allocation failed\n");
		return;
	}
	result[0] = '\0'; // Initialize the result string to empty

	// Get the first token (word)
	token = strtok(str, " ");

	// Skip the first 'num_words' words
	while (token != NULL && w_count < count) {
		token = strtok(NULL, " ");
		w_count++;
	}

	// After skipping 'num_words', append the rest of the words to the result
	while (token != NULL) {
		size_t token_len = strlen(token);
		if (strlen(result) > 0) {
			// Ensure there's enough space to append a space and token
			if (strlen(result) + 1 + token_len >= result_size) {
				result_size *= 2; // Double the size of the buffer if needed
				result = realloc(result, result_size);
				if (result == NULL) {
					printf("Memory allocation failed\n");
					return;
				}
			}
			strcat(result, " "); // Add a space between words
		}
		strcat(result, token);
		token = strtok(NULL, " ");
	}

	// Copy the result back into the original string
	strcpy(str, result);

	// Free the dynamically allocated memory for result
	free(result);
}

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove)
{
	long long i = 0;
	const char *name = Enum[i];
	while (name != "") {
		char *display_name = screaming_snake_case_to_title_case(name);
		remove_front_words(display_name, word_count_to_remove);
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