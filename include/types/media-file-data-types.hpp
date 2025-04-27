#ifndef MEDIA_FILE_DATA_TYPES_HPP
#define MEDIA_FILE_DATA_TYPES_HPP

#include <string>
#include <deque>
#include "../qt-classes/media-widget.hpp"

class MediaWidget;

typedef struct MediaContext {
	std::string path;
	std::string filename;
	std::string name;
	std::string ext;
	size_t index;
} MediaContext;

typedef struct MediaData {
	MediaContext media_data;
	MediaWidget *media_widget;
	MediaWidget *param_media_widget;
	// std::string path;
	// std::string filename;
	// std::string name;
	// std::string ext;
	// size_t index;
} MediaData;

typedef std::shared_ptr<MediaData> QueueMedia;

typedef std::deque<MediaContext> MediaList;
typedef std::deque<QueueMedia> QueuedMedia;

#endif MEDIA_FILE_DATA_TYPES_HPP