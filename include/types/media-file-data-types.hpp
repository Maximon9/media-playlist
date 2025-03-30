#ifndef MEDIA_FILE_DATA_TYPES_HPP
#define MEDIA_FILE_DATA_TYPES_HPP

#include <string>
#include <deque>
#include "../qt-classes/media-widget.hpp"

class MediaWidget;

typedef struct MediaData {
	std::string path;
	std::string filename;
	std::string name;
	std::string ext;
	size_t index;
} MediaData;

typedef struct QueueMediaData {
	MediaData media_data;
	MediaWidget *media_widget;
	MediaWidget *param_media_widget;
	// std::string path;
	// std::string filename;
	// std::string name;
	// std::string ext;
	// size_t index;
} QueueMediaData;

typedef std::deque<MediaData> MediaDataArray;
typedef std::deque<std::shared_ptr<QueueMediaData>> QueueMediaDataArray;

#endif MEDIA_FILE_DATA_TYPES_HPP