#ifndef MEDIA_WIDGET_HPP
#define MEDIA_WIDGET_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../types/media-file-data-types.hpp"

typedef struct MediaFileData MediaFileData;

class MediaWidget : public QWidget {
public:
	QLabel *label;
	explicit MediaWidget(const MediaFileData *media_file_data, QWidget *parent = nullptr);
	void update_media_file_data();
};

#endif // MEDIA_WIDGET_HPP