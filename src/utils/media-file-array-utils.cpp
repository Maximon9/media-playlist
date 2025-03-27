#pragma region Main

#include <../../include/utils/media-file-array-utils.hpp>

void push_media_back(MediaDataArray *media_array, const std::string path)
{
	MediaData entry = load_media_data_from_path(path, 0);
	media_array->push_back(entry);
}

void push_media_front(MediaDataArray *media_array, const std::string path)
{
	MediaData entry = load_media_data_from_path(path, 0);
	media_array->push_front(entry);
}

void push_media_at(MediaDataArray *media_array, const std::string path, size_t index)
{
	MediaDataArray::const_iterator it = media_array->cbegin() + index;
	MediaData entry = load_media_data_from_path(path, 0);
	media_array->insert(it, entry);
}

void push_media_media_file_at(MediaDataArray *media_array, MediaData *entry, size_t index)
{
	MediaDataArray::const_iterator it = media_array->cbegin() + index;
	media_array->insert(it, *entry);
}

void pop_media_at(MediaDataArray *media_array, size_t index)
{
	MediaDataArray::const_iterator it = media_array->cbegin() + index;
	media_array->erase(it);
}

MediaData load_media_data_from_path(std::string path, size_t index)
{
	// Create and insert new MediaData
	fs::path file_path = path;

	// Extract the filename (with extension)
	std::string filename = file_path.filename().string();

	// Extract the file name without extension
	std::string name = file_path.stem().string();

	// Extract the file extension (including the dot)
	std::string ext = file_path.extension().string();

	MediaData new_entry = {path, filename, name, ext, index};

	return new_entry;
}

MediaData construct_complete_media_data(const std::string path, const std::string filename, const std::string name,
					const std::string ext, size_t index)
{
	// Create and insert new MediaData
	MediaData new_entry = {path, filename, name, ext, index};
	return new_entry;
}

void push_queue_media_back(QueueMediaDataArray *media_array, const std::string path, PlaylistData *playlist_data)
{
	QueueMediaData entry{};
	init_queue_media_data_from_path(&entry, path, media_array->size() - 1, playlist_data);
	media_array->push_back(entry);
}

void push_queue_media_front(QueueMediaDataArray *media_array, const std::string path, PlaylistData *playlist_data)
{
	QueueMediaData entry{};
	init_queue_media_data_from_path(&entry, path, 0, playlist_data);
	media_array->push_front(entry);
}

void push_queue_media_at(QueueMediaDataArray *media_array, const std::string path, size_t index,
			 PlaylistData *playlist_data)
{
	QueueMediaData entry{};
	init_queue_media_data_from_path(&entry, path, index, playlist_data);

	QueueMediaDataArray::const_iterator it = media_array->cbegin() + index;
	media_array->insert(it, entry);
}

void push_queue_media_media_file_at(QueueMediaDataArray *media_array, QueueMediaData *entry, size_t index)
{
	QueueMediaDataArray::const_iterator it = media_array->cbegin() + index;
	media_array->insert(it, *entry);
}

void pop_queue_media_back(QueueMediaDataArray *media_array)
{
	QueueMediaData *new_entry = &media_array->at(media_array->size() - 1);
	new_entry->media_widget->remove_widget();
	media_array->pop_front();
}

void pop_queue_media_front(QueueMediaDataArray *media_array)
{
	QueueMediaData *new_entry = &media_array->at(0);
	new_entry->media_widget->remove_widget();
	media_array->pop_front();
}

void pop_queue_media_at(QueueMediaDataArray *media_array, size_t index)
{
	QueueMediaData *new_entry = &media_array->at(index);
	new_entry->media_widget->remove_widget();
	QueueMediaDataArray::const_iterator it = media_array->cbegin() + index;
	media_array->erase(it);
}

void init_queue_media_data_from_path(QueueMediaData *new_entry, std::string path, size_t index,
				     PlaylistData *playlist_data)
{
	// Create and insert new MediaData
	fs::path file_path = path;

	// Extract the filename (with extension)
	std::string filename = file_path.filename().string();

	// Extract the file name without extension
	std::string name = file_path.stem().string();

	// Extract the file extension (including the dot)
	std::string ext = file_path.extension().string();

	new_entry->media_data = {path, filename, name, ext, index};

	new_entry->media_widget = new MediaWidget(&new_entry->media_data, playlist_data->playlist_widget);

	playlist_data->playlist_widget->mediaLayout->addWidget(new_entry->media_widget);
}

void init_queue_media_data(QueueMediaData *new_entry, const std::string path, const std::string filename,
			   const std::string name, const std::string ext, size_t index, MediaWidget *media_widget,
			   PlaylistData *playlist_data)
{
	// Create and insert new MediaData
	new_entry->media_data = {path, filename, name, ext, index};
	if (media_widget == nullptr) {
		new_entry->media_widget = new MediaWidget(&new_entry->media_data, playlist_data->playlist_widget);
		playlist_data->playlist_widget->mediaLayout->addWidget(media_widget);
	} else {
		media_widget->media_data = nullptr;
		media_widget->media_data = &new_entry->media_data;
		new_entry->media_widget = media_widget;
	}
}

void init_queue_media_data_from_media_data(QueueMediaData *new_entry, MediaData media_data, MediaWidget *media_widget,
					   PlaylistData *playlist_data)

{
	// Create and insert new MediaData
	new_entry->media_data = media_data;
	if (media_widget == nullptr) {
		new_entry->media_widget = new MediaWidget(&new_entry->media_data, playlist_data->playlist_widget);
		playlist_data->playlist_widget->mediaLayout->addWidget(media_widget);
	} else {
		media_widget->media_data = nullptr;
		media_widget->media_data = &new_entry->media_data;
		new_entry->media_widget = media_widget;
	}
}

std::string obs_array_to_string(obs_data_array_t *array)
{
	size_t array_size = obs_data_array_count(array);

	if (array == NULL || array_size == 0) {
		return "[]";
	}

	// Estimate maximum size for the  std::string  (2  strings per element + commas + brackets)
	size_t estimated_size = array_size * 2 + array_size - 1 + 2; // Two  strings per element + commas + brackets
	std::string result = "";

	// Start the  std::string  with the opening bracket
	result += "[";

	// Loop through the array and append each element
	for (size_t i = 0; i < array_size; ++i) {
		// Convert element to  std::string  (single  stringacter)
		obs_data_t *data = obs_data_array_item(array, i);
		char *element[2] = {(char *)obs_data_get_string(data, "value"),
				    (char *)'\0'}; // Create a single  std::string   string

		result += *element;

		// Add a comma if not the last element
		if (i < array_size - 1) {
			result += ", ";
		}
	}

	// End the  std::string  with the closing bracket
	result += "]";

	return result;
}

bool valid_extension(const std::string *ext)
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

/* void obs_data_media_array_retain(MediaDataArray *media_file_data_array, obs_data_array_t *obs_playlist)
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
			continue;               // Skip if no valid  std::string  was found
		}
		fs::path file_path = element;

		if (fs::is_directory(file_path)) {
			for (const fs::directory_entry &entry : fs::recursive_directory_iterator(file_path)) {
				// Print the path of each entry (file or directory)
				if (entry.is_directory())
					continue;

				fs::path entry_path = entry.path();

				// Get the extension of the file or directory
				std::string  extension = entry_path.extension().string();
				if (!valid_extension(&extension))
					continue;
				const MediaData new_entry = load_media_data_from_path(
					entry.path().string(), media_file_data_array->size());
				media_file_data_array->push_back(new_entry);
				// push_media_file_data_back(media_file_data_array, new_entry);
			}
		} else {
			media_file_data_array->push_back(
				load_media_data_from_path(file_path.string(), media_file_data_array->size()));
		}

		obs_data_release(data);
	}
	obs_data_array_release(obs_playlist);
} */

const char *stringify_media_array(const MediaDataArray *media_array, size_t threshold, const std::string indent,
				  e_MediaStringifyTYPE media_stringify_type)
{
	if (media_array == NULL || media_array->size() <= 0) {
		return (char *)"[]"; // Return empty brackets if no elements
	}

	// Allocate memory for the final  std::string  (before prettification)
	std::string result = "";

	// Apply compact format first
	result += "[";
	for (size_t i = 0; i < media_array->size(); i++) {
		result += "\"";

		const MediaData *media_file_data = &((*media_array)[i]);

		switch (media_stringify_type) {
		case MEDIA_STRINGIFY_TYPE_PATH:
			result += media_file_data->path;
			break;
		case MEDIA_STRINGIFY_TYPE_FILENAME:
			// obs_log(LOG_INFO, "Media File Name: %s", media_file_data->filename.c_str());
			result += media_file_data->filename;
			break;
		case MEDIA_STRINGIFY_TYPE_NAME:
			// obs_log(LOG_INFO, "Media Name: %s", media_file_data->name.c_str());
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

	// If the compact  std::string  exceeds the threshold, prettify the result
	if (result.size() > threshold) {
		// Allocate memory for the prettified  string
		std::string prettified_result = "";

		// Construct the prettified  string
		prettified_result += "[\n";
		for (size_t i = 0; i < media_array->size(); i++) {
			prettified_result += indent; // Add indentation before each element
			prettified_result += "\"";

			const MediaData *media_file_data = &media_array->at(i);

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

		return prettified_result.c_str();
	}

	return result.c_str(); // Return the compact  std::string  if it's within the threshold
}
const char *stringify_queue_media_array(const QueueMediaDataArray *media_array, int queue_limit,
					const std::string indent, e_MediaStringifyTYPE media_stringify_type)
{
	size_t queue_size_list = (size_t)(queue_limit);

	if (media_array == NULL || media_array->size() <= 0) {
		return "[]"; // Return empty brackets if no elements
	}

	size_t min_size = std::min(media_array->size(), queue_size_list);

	if (min_size <= 0) {
		return "[]"; // Return empty brackets if no elements
	}

	// size_t prettified_length = 3; // For "[\n" and "]\n"
	// for (size_t i = 0; i < min_size; i++) {
	// 	prettified_length +=
	// 		strlen(indent) + strlen(get_media(media_array, i)->path) + 4; // Indent, quotes, and comma
	// }

	// Allocate memory for the prettified  string
	std::string prettified_result = "";

	// Construct the prettified  string
	prettified_result += "[\n";

	for (size_t i = 0; i < min_size; i++) {
		if (i >= (size_t)queue_limit) {
			break;
		}
		prettified_result += indent; // Add indentation before each element
		prettified_result += "\"";

		const MediaData *media_file_data = &(media_array->at(i).media_data);

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

	return prettified_result.c_str();
}

void obs_log_media_array(int log_level, std::string format, const MediaDataArray *media_array, size_t threshold,
			 const std::string indent, e_MediaStringifyTYPE media_stringify_type)
{
	std::string result = stringify_media_array(media_array, threshold, indent, media_stringify_type);

	obs_log(log_level, (format + result).c_str());
}

void obs_log_queue_media_array(int log_level, std::string format, const QueueMediaDataArray *media_array,
			       int queue_limit, const std::string indent, e_MediaStringifyTYPE media_stringify_type)
{
	std::string result = stringify_queue_media_array(media_array, queue_limit, indent, media_stringify_type);

	obs_log(log_level, (format + result).c_str());
}

bool compare_media_file_data(const MediaData *data_1, const MediaData *data_2)
{
	if (data_1->path == data_2->path && data_1->index == data_2->index) {
		return true;
	}
	return false;
}

bool compare_media_file_data_arrays(const MediaDataArray *array_1, const MediaDataArray *array_2)
{
	if (array_1->size() == array_2->size()) {
		for (size_t i = 0; i < array_1->size(); i++) {
			const MediaData *data_1 = &array_1->at(i);
			const MediaData *data_2 = &array_2->at(i);
			if (compare_media_file_data(data_1, data_2) == false) {
				return false;
			}
		}
		return true;
	}
	return false;
}

#pragma endregion