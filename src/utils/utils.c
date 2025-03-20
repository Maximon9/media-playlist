#pragma region Main

#include "../include/utils/utils.h"

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

// void init_media_array(MediaFileDataArray *media_array, size_t initial_capacity)
// {
// 	media_array->size = 0;
// 	media_array->capacity = initial_capacity;
// 	media_array->data = (MediaFileData *)malloc(media_array->capacity * sizeof(MediaFileData));
// }

// void push_media_back(MediaFileDataArray *media_array, const char *path)
// {
// 	push_media_at(media_array, path, media_array->size);
// }

// void push_media_front(MediaFileDataArray *media_array, const char *path)
// {
// 	push_media_at(media_array, path, 0);
// }

// void push_media_at(MediaFileDataArray *media_array, const char *path, size_t index)
// {
// 	// Create and insert new MediaFileData
// 	MediaFileData new_entry = {0};
// 	new_entry.path = strdup(path);

// 	// Extract filename from path
// 	const char *last_slash = strrchr(path, '/');
// 	new_entry.filename = strdup(last_slash ? last_slash + 1 : path);

// 	// Generate a simple ID (you might replace this with a better approach)
// 	// new_entry.id = strdup(new_entry.filename);

// 	// Set other default values
// 	new_entry.is_url = false;
// 	new_entry.is_folder = false;
// 	// new_entry.parent = NULL;
// 	// new_entry.parent_id = NULL;
// 	new_entry.index = index;

// 	push_media_file_data_at(media_array, new_entry, index);
// }

// void push_media_file_data_back(MediaFileDataArray *media_array, MediaFileData media_file_data)
// {
// 	push_media_file_data_at(media_array, media_file_data, media_array->size);
// }

// void push_media_file_data_front(MediaFileDataArray *media_array, MediaFileData media_file_data)
// {
// 	push_media_file_data_at(media_array, media_file_data, 0);
// }

// void push_media_file_data_at(MediaFileDataArray *media_array, MediaFileData media_file_data, size_t index)
// {
// 	// Ensure index is within bounds
// 	if (index > media_array->size) {
// 		index = media_array->size; // Append to the end if index is out of range
// 	}

// 	// Resize if needed
// 	if (media_array->size >= media_array->capacity) {
// 		size_t new_capacity = (media_array->capacity == 0) ? 1 : media_array->capacity * 2;
// 		MediaFileData *new_data = realloc(media_array->data, new_capacity * sizeof(MediaFileData));
// 		if (!new_data)
// 			return; // Memory allocation failed
// 		media_array->data = new_data;
// 		media_array->capacity = new_capacity;
// 	}

// 	// Shift elements to the right
// 	memmove(&media_array->data[index + 1], &media_array->data[index],
// 		(media_array->size - index) * sizeof(MediaFileData));

// 	// Insert into array
// 	media_array->data[index] = media_file_data;
// 	media_array->size++;
// }

// void pop_media_back(MediaFileDataArray *media_array)
// {
// 	pop_media_at(media_array, media_array->size);
// }

// void pop_media_front(MediaFileDataArray *media_array)
// {
// 	pop_media_at(media_array, 0);
// }

// void pop_media_at(MediaFileDataArray *media_array, size_t index)
// {
// 	if (index >= media_array->size)
// 		return;

// 	free(media_array->data[index].path);
// 	free(media_array->data[index].filename);

// 	for (size_t i = index; i < media_array->size - 1; i++) {
// 		media_array->data[i] = media_array->data[i + 1];
// 	}

// 	media_array->size--;

// 	if (media_array->size > 0 && media_array->size <= media_array->capacity / 4) {
// 		media_array->capacity /= 2;
// 		media_array->data =
// 			(MediaFileData *)realloc(media_array->data, media_array->capacity * sizeof(MediaFileData));
// 	}
// }

// // Function to get a string by index
// const MediaFileData *get_media(MediaFileDataArray *media_array, size_t index)
// {
// 	if (index >= media_array->size)
// 		return NULL; // Out of bounds
// 	return &media_array->data[index];
// }

// // Function to free the dynamic string array
// void free_media_array(MediaFileDataArray *media_array)
// {
// 	if (media_array != NULL) {
// 		if (media_array->size > 0) {
// 			for (size_t i = 0; i < media_array->size; i++) {
// 				free(media_array->data[i].path);
// 				free(media_array->data[i].filename);
// 			}
// 		}
// 		if (media_array->data != NULL) {
// 			free(media_array->data);
// 		}
// 		free(media_array);
// 	}
// }

MediaFileDataArray *obs_data_array_retain(MediaFileDataArray *media_file_data_array, obs_data_array_t *obs_playlist)
{
	size_t array_size = obs_data_array_count(obs_playlist);
	if (array_size == 0) {
		da_free(*media_file_data_array);
		return NULL;
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
		// Use the method call syntax; this passes media_file_data_array as the first parameter.
		da_push_back(*media_file_data_array, element);
		obs_data_release(data);
	}
	obs_data_array_release(obs_playlist);
	return media_file_data_array;
}

char *stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const char *indent)
{
	if (media_array == NULL && media_array->num == 0) {
		return strdup("[]"); // Return empty brackets if no elements
	}
	// Calculate the initial length of the compact format
	size_t total_length = 3; // For "[" and "]\0"
	for (size_t i = 0; i < media_array->num; i++) {
		total_length += strlen(media_array->array[i].path) + 4; // Quotes, comma, space
	}

	// Allocate memory for the final string (before prettification)
	char *result = (char *)malloc(total_length * sizeof(char));
	if (!result)
		return NULL; // Handle allocation failure

	// Apply compact format first
	strcpy(result, "[");
	for (size_t i = 0; i < media_array->num; i++) {
		strcat(result, "\"");
		strcat(result, media_array->array[i].path);
		strcat(result, "\"");
		if (i < media_array->num - 1)
			strcat(result, ", ");
	}
	strcat(result, "]");

	// If the compact string exceeds the threshold, prettify the result
	if (strlen(result) > threshold) {
		size_t prettified_length = 3; // For "[\n" and "]\n"
		for (size_t i = 0; i < media_array->num; i++) {
			prettified_length +=
				strlen(indent) + strlen(media_array->array[i].path) + 4; // Indent, quotes, and comma
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
			strcat(prettified_result, media_array->array[i].path);
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

void obs_log_media_array(int log_level, const MediaFileDataArray *media_array, size_t threshold, const char *indent)
{
	char *result = stringify_media_array(media_array, threshold, indent);
	obs_log(log_level, result);
	free(result);
}

#pragma endregion