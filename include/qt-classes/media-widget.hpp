#ifndef MEDIA_WIDGET_HPP
#define MEDIA_WIDGET_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "../types/media-file-data-types.hpp"

typedef struct QueueMediaData QueueMediaData;

class MediaWidget : public QWidget {
public:
	QLabel *label;
	const QueueMediaData *media_data;
	explicit MediaWidget(const QueueMediaData *media_data, QWidget *parent = nullptr);
	void update_media_data();
	void remove_widget();
};

#endif // MEDIA_WIDGET_HPP