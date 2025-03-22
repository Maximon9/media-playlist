#pragma region Main

#include "../include/utils/utils.hpp"

#pragma region Media Array Utils

/* void init_media_array(MediaFileDataArray *media_array, size_t initial_capacity)
{
	media_array->size() = 0;
	media_array->capacity = initial_capacity;
	media_array->data = (MediaFileData *)malloc(media_array->capacity * sizeof(MediaFileData));
}

void push_media_back(MediaFileDataArray *media_array, const string *path)
{
	push_media_at(media_array, path, media_array->size());
}

void push_media_front(MediaFileDataArray *media_array, const string *path)
{
	push_media_at(media_array, path, 0);
}

void push_media_at(MediaFileDataArray *media_array, const string *path, size_t index)
{
	push_media_file_data_at(media_array, create_media_file_data_from_path(path, index), index);
}

void push_media_file_data_back(MediaFileDataArray *media_array, MediaFileData media_file_data)
{
	push_media_file_data_at(media_array, media_file_data, media_array->size());
}

void push_media_file_data_front(MediaFileDataArray *media_array, MediaFileData media_file_data)
{
	push_media_file_data_at(media_array, media_file_data, 0);
}

void push_media_file_data_at(MediaFileDataArray *media_array, MediaFileData media_file_data, size_t index)
{
	// Ensure index is within bounds
	if (index > media_array->size()) {
		index = media_array->size(); // Append to the end if index is out of range
	}

	// Resize if needed
	if (media_array->size() >= media_array->capacity) {
		size_t new_capacity = (media_array->capacity == 0) ? 1 : media_array->capacity * 2;
		MediaFileData *new_data =
			(MediaFileData *)realloc(media_array->data, new_capacity * sizeof(MediaFileData));
		if (!new_data)
			return; // Memory allocation failed
		media_array->data = new_data;
		media_array->capacity = new_capacity;
	}

	// Shift elements to the right
	memmove(&media_array->data[index + 1], &media_array->data[index],
		(media_array->size() - index) * sizeof(MediaFileData));

	// Insert into array
	media_array->data[index] = media_file_data;
	media_array->size()++;
}

void pop_media_back(MediaFileDataArray *media_array)
{
	pop_media_at(media_array, media_array->size());
}

void pop_media_front(MediaFileDataArray *media_array)
{
	pop_media_at(media_array, 0);
}

void pop_media_at(MediaFileDataArray *media_array, size_t index)
{
	if (index >= media_array->size())
		return;

	free(media_array->data[index].path);
	free(media_array->data[index].filename);
	free(media_array->data[index].name);
	free(media_array->data[index].ext);

	for (size_t i = index; i < media_array->size() - 1; i++) {
		media_array->data[i] = media_array->data[i + 1];
	}

	media_array->size()--;

	if (media_array->size() > 0 && media_array->size() <= media_array->capacity / 4) {
		media_array->capacity /= 2;
		media_array->data =
			(MediaFileData *)realloc(media_array->data, media_array->capacity * sizeof(MediaFileData));
	}
}

void move_media_at(MediaFileDataArray *media_array, size_t from_index, size_t to_index)
{
	if (!media_array || from_index >= media_array->size() || to_index >= media_array->size()) {
		return; // Out-of-bounds check
	}

	if (from_index == to_index) {
		return; // No need to move if indexes are the same
	}

	// Swap elements
	MediaFileData temp = media_array->data[from_index];
	if (from_index < to_index) {
		// Shift left
		memmove(&media_array->data[from_index], &media_array->data[from_index + 1],
			(to_index - from_index) * sizeof(MediaFileData));
	} else {
		// Shift right
		memmove(&media_array->data[to_index + 1], &media_array->data[to_index],
			(from_index - to_index) * sizeof(MediaFileData));
	}

	// Place the moved item in its new position
	media_array->data[to_index] = temp;
}

void move_media_array(MediaFileDataArray *destination, MediaFileDataArray *source)
{
	assert(destination != NULL);
	assert(source != NULL);

	// Free the existing data in the destination array
	free_media_array(destination);

	// Move ownership from source to destination
	*destination = *source;

	// Reset the source array to avoid accidental double-free
	source->data = NULL;
	source->size() = 0;
	source->capacity = 0;
}

// Function to get a string by index
const MediaFileData *get_media(const MediaFileDataArray *media_array, size_t index)
{
	if (index >= media_array->size())
		return NULL; // Out of bounds
	return &media_array->data[index];
}

void clear_media_array(MediaFileDataArray *media_array)
{
	if (!media_array || !media_array->data)
	return;
	
	// Free all elements
	for (size_t i = 0; i < media_array->size(); i++) {
		free(media_array->data[i].path);
		free(media_array->data[i].filename);
		free(media_array->data[i].name);
		free(media_array->data[i].ext);
	}
	
	// Reset size but keep allocated memory
	media_array->size() = 0;
}

// Function to free the dynamic string array
void free_media_array(MediaFileDataArray *media_array)
{
	if (media_array != NULL) {
		if (media_array->size() > 0) {
			for (size_t i = 0; i < media_array->size(); i++) {
				const MediaFileData *data = get_media(media_array, i);
				if (!data)
				return;
				free(data->path);
				free(data->filename);
				free(data->name);
				free(data->ext);
			}
		}
		if (media_array->data != NULL) {
			free(media_array->data);
		}
	}
}
*/

MediaFileData create_media_file_data_from_path(const string path, size_t index)
{
	// Create and insert new MediaFileData
	fs::path file_path = path;

	// Extract the filename (with extension)
	string filename = file_path.filename().string();

	// Extract the file name without extension
	string name = file_path.stem().string();

	// Extract the file extension (including the dot)
	string ext = file_path.extension().string();

	MediaFileData new_entry = {path, filename, name, ext, index};
	return new_entry;
}

/* MediaFileData create_media_file_data_from_path_and_file_name(const string path, const string filename, size_t index)
{
	// Create and insert new MediaFileData
	fs::path file_path = filename;

	// Extract the file name without extension
	string name = file_path.stem().string();

	// Extract the file extension (including the dot)
	string ext = file_path.extension().string();

	MediaFileData new_entry = {path, filename, name, ext, index};
	return new_entry;
} */

MediaFileData create_media_file_data_with_all_info(const string path, const string filename, const string name,
						   const string ext, size_t index)
{
	// Create and insert new MediaFileData
	MediaFileData new_entry = {path, filename, name, ext, index};
	return new_entry;
}

string obs_array_to_string(obs_data_array_t *array)
{
	size_t array_size = obs_data_array_count(array);

	if (array == NULL || array_size == 0) {
		return "[]";
	}

	// Estimate maximum size for the string (2 strings per element + commas + brackets)
	size_t estimated_size = array_size * 2 + array_size - 1 + 2; // Two strings per element + commas + brackets
	string result = "";

	// Start the string with the opening bracket
	result += "[";

	// Loop through the array and append each element
	for (size_t i = 0; i < array_size; ++i) {
		// Convert element to string (single stringacter)
		obs_data_t *data = obs_data_array_item(array, i);
		char *element[2] = {(char *)obs_data_get_string(data, "value"),
				    (char *)'\0'}; // Create a single string string

		result += *element;

		// Add a comma if not the last element
		if (i < array_size - 1) {
			result += ", ";
		}
	}

	// End the string with the closing bracket
	result += "]";

	return result;
}

bool valid_extension(const string *ext)
{
	struct dstr haystack = {0};
	struct dstr needle = {0};
	bool valid = false;

	if (!ext || *ext == "")
		return false;

	dstr_copy(&haystack, media_filter);
	dstr_cat(&haystack, video_filter);
	dstr_cat(&haystack, audio_filter);

	dstr_cat_ch(&needle, '*');
	dstr_cat(&needle, ext->c_str());

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
		fs::path file_path = element;

		if (fs::is_directory(file_path)) {
			for (const fs::directory_entry &entry : fs::recursive_directory_iterator(file_path)) {
				// Print the path of each entry (file or directory)
				if (entry.is_directory())
					continue;

				fs::path entry_path = entry.path();

				// Get the extension of the file or directory
				string extension = entry_path.extension().string();
				if (!valid_extension(&extension))
					continue;
				const MediaFileData new_entry = create_media_file_data_from_path(
					entry.path().string(), media_file_data_array->size());
				media_file_data_array->push_back(new_entry);
				// push_media_file_data_back(media_file_data_array, new_entry);
			}
			/* struct os_dirent *entry;
			while (true) {

				const char *ext;

				entry = os_readdir(dir);
				if (!entry)
					break;

				ext = os_get_path_extension(entry->d_name);

				// Allocate memory dynamically for full path

				// obs_log(LOG_INFO, "\nFound file: \n%s\n%s\n%s\n", element, entry->d_name, full_path);

				const MediaFileData new_entry = create_media_file_data_from_path_and_file_name(
					full_path, entry->d_name, media_file_data_array->size());
				push_media_file_data_back(media_file_data_array, new_entry);

				// Free allocated memory
				free(full_path);
			}

			os_closedir(dir); */
		} else {
			media_file_data_array->push_back(
				create_media_file_data_from_path(file_path.string(), media_file_data_array->size()));
		}

		obs_data_release(data);
	}
	obs_data_array_release(obs_playlist);
}

string stringify_media_array(const MediaFileDataArray *media_array, size_t threshold, const string indent,
			     e_MediaStringifyTYPE media_stringify_type)
{
	if (media_array == NULL || media_array->size() <= 0) {
		return "[]"; // Return empty brackets if no elements
	}
	// Calculate the initial length of the compact format
	// size_t total_length = 3; // For "[" and "]\0"
	// for (size_t i = 0; i < media_array->size(); i++) {
	// 	const MediaFileData *media_file_data = &media_array.at(i);

	// 	switch (media_stringify_type) {
	// 	case MEDIA_STRINGIFY_TYPE_PATH:
	// 		total_length += media_file_data->path.size() + 4; // Quotes, comma, space
	// 		break;
	// 	case MEDIA_STRINGIFY_TYPE_FILENAME:
	// 		total_length += media_file_data->filename.size() + 4; // Quotes, comma, space
	// 		break;
	// 	case MEDIA_STRINGIFY_TYPE_NAME:
	// 		total_length += media_file_data->name.size() + 4; // Quotes, comma, space
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// }

	// Allocate memory for the final string (before prettification)
	string result = "";

	// Apply compact format first
	result += "[";
	for (size_t i = 0; i < media_array->size(); i++) {
		result += "\"";

		const MediaFileData *media_file_data = &media_array->at(i);

		switch (media_stringify_type) {
		case MEDIA_STRINGIFY_TYPE_PATH:
			result += media_file_data->path;
			break;
		case MEDIA_STRINGIFY_TYPE_FILENAME:
			result += media_file_data->filename;
			break;
		case MEDIA_STRINGIFY_TYPE_NAME:
			result += media_file_data->name;
			break;
		default:
			break;
		}
		result += "\"";
		if (i < media_array->size() - 1)
			result += ", ";
	}
	result += "]";

	// If the compact string exceeds the threshold, prettify the result
	if (result.size() > threshold) {
		// Allocate memory for the prettified string
		string prettified_result = "";

		// Construct the prettified string
		prettified_result += "[\n";
		for (size_t i = 0; i < media_array->size(); i++) {
			prettified_result += indent; // Add indentation before each element
			prettified_result += "\"";

			const MediaFileData *media_file_data = &media_array->at(i);

			switch (media_stringify_type) {
			case MEDIA_STRINGIFY_TYPE_PATH:
				prettified_result += media_file_data->path;
				break;
			case MEDIA_STRINGIFY_TYPE_FILENAME:
				prettified_result += media_file_data->filename;
				break;
			case MEDIA_STRINGIFY_TYPE_NAME:
				prettified_result += media_file_data->name;
				break;
			default:
				break;
			}
			prettified_result += "\"";

			if (i < media_array->size() - 1) {
				prettified_result += ",\n";
			}
		}

		prettified_result += "\n]";

		return prettified_result;
	}

	return result; // Return the compact string if it's within the threshold
}
string stringify_media_queue_array(const MediaFileDataArray *media_array, int queue_limit, const string indent,
				   e_MediaStringifyTYPE media_stringify_type)
{
	size_t queue_size_list = (size_t)(queue_limit);

	if (media_array == NULL || media_array->size() <= 0) {
		return "[]"; // Return empty brackets if no elements
	}

	size_t min_size = min(media_array->size(), queue_size_list);

	if (min_size <= 0) {
		return "[]"; // Return empty brackets if no elements
	}

	// size_t prettified_length = 3; // For "[\n" and "]\n"
	// for (size_t i = 0; i < min_size; i++) {
	// 	prettified_length +=
	// 		strlen(indent) + strlen(get_media(media_array, i)->path) + 4; // Indent, quotes, and comma
	// }

	// Allocate memory for the prettified string
	string prettified_result = "";

	// Construct the prettified string
	prettified_result += "[\n";

	for (size_t i = 0; i < min_size; i++) {
		if (i >= (size_t)queue_limit) {
			break;
		}
		prettified_result += indent; // Add indentation before each element
		prettified_result += "\"";

		const MediaFileData *media_file_data = &media_array->at(i);

		switch (media_stringify_type) {
		case MEDIA_STRINGIFY_TYPE_PATH:
			prettified_result += media_file_data->path;
			break;
		case MEDIA_STRINGIFY_TYPE_FILENAME:
			prettified_result += media_file_data->filename;
			break;
		case MEDIA_STRINGIFY_TYPE_NAME:
			prettified_result += media_file_data->name;
			break;
		default:
			break;
		}
		prettified_result += "\"";

		if (i < (min_size - 1)) {
			prettified_result += ",\n";
		}
	}

	prettified_result += "\n]";

	return prettified_result;
}

void obs_log_media_array(int log_level, string format, const MediaFileDataArray *media_array, size_t threshold,
			 const string indent, e_MediaStringifyTYPE media_stringify_type)
{
	string result = stringify_media_array(media_array, threshold, indent, media_stringify_type);

	obs_log(log_level, (format + result).c_str());
}

bool compare_media_file_data(const MediaFileData *data_1, const MediaFileData *data_2)
{
	if (data_1->path == data_2->path && data_1->index == data_2->index) {
		return true;
	}
	return false;
}

bool compare_media_file_data_arrays(const MediaFileDataArray *array_1, const MediaFileDataArray *array_2)
{
	if (array_1->size() == array_2->size()) {
		for (size_t i = 0; i < array_1->size(); i++) {
			const MediaFileData *data_1 = &array_1->at(i);
			const MediaFileData *data_2 = &array_2->at(i);
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

string screaming_snake_case_to_title_case(const string *name)
{
	if (!name)
		return NULL;

	size_t len = name->size();
	string output = ""; // Allocate memory for new string

	int capitalize_next = 1; // Ensure first letter is uppercase

	for (size_t i = 0; i < len; i++) {
		if ((*name)[i] == '_') {
			output[i] = ' ';     // Replace underscore with space
			capitalize_next = 1; // Next letter should be capitalized
		} else {
			output[i] = capitalize_next ? toupper((*name)[i]) : tolower((*name)[i]);
			capitalize_next = 0;
		}
	}

	output[len] = '\0'; // Null-terminate the string
	return output;
}

void remove_front_words(string *str, int count)
{
	size_t pos = 0;

	// Loop to find and remove the first `num_words` from the front
	while (count > 0 && pos != std::string::npos) {
		// Find the space after each word
		pos = str->find(' ', pos);

		// If we find a space, move past it, otherwise we are at the end of the string
		if (pos != std::string::npos) {
			pos++; // Move past the space
		}
		*str = str->substr(pos); // Remove everything before `pos`
		count--;
	}

	// Now, if we have removed the desired number of words, keep the rest of the string
	if (count == 0) {
	} else {
		*str = ""; // If we attempted to remove more words than available, set the string to empty
	}
}

void add_enums_to_property_list(obs_property_t *property, const char *Enum[], int word_count_to_remove)
{
	long long i = 0;
	string name = Enum[i];
	while (name != "") {
		string display_name = screaming_snake_case_to_title_case(&name);
		remove_front_words(&display_name, word_count_to_remove);
		obs_property_list_add_int(property, display_name.c_str(), i);
		i++;
		name = Enum[i];
	}
}

#pragma endregion

#pragma endregion