#ifndef MEDIA_WIDGET_HPP
#define MEDIA_WIDGET_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../types/media-file-data-types.hpp"
#include "../include/utils/enum-utils.hpp"

typedef struct MediaData MediaData;

class MediaWidget : public QWidget {
public:
	QLabel *label;
	e_MediaStringifyTYPE media_stringify_type;
	const MediaData *media_data;
	explicit MediaWidget(const MediaData *media_data,
			     e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME,
			     QWidget *parent = nullptr);
	void update_media_data(e_MediaStringifyTYPE *media_stringify_type = nullptr);
	void remove_widget();
};

#endif // MEDIA_WIDGET_HPP