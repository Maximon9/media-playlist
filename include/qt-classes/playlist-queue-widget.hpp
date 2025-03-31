#ifndef PLAYLIST_QUEUE_VIEWER_HPP
#define PLAYLIST_QUEUE_VIEWER_HPP

#include <QEventLoop>
#include <QScrollArea>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

typedef struct PlaylistData PlaylistData;

class PlaylisQueueWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QHBoxLayout *buttonLayout;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	e_MediaStringifyTYPE media_stringify_type;
	bool expanded;

#pragma region Main Widget Params
	bool is_main_widget;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;
#pragma endregion

public:
	const PlaylistData *playlist_data;
	explicit PlaylisQueueWidget(const PlaylistData *playlist, QWidget *parent = nullptr,
				    bool is_main_widget = false);
	void toggleMediaVisibility();
	void update_playlist_name();
	void update_playlist_data(e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME);
	void remove_widget();
	// void create_media_widget(MediaData *media_data, std::function<void(MediaWidget *)> callback);
	MediaWidget *create_media_widget(MediaData *media_data,
					 e_MediaStringifyTYPE media_stringify_type = MEDIA_STRINGIFY_TYPE_FILENAME);
	void insert_media_widget(MediaWidget *mediaWidget, size_t index);
};
#endif // PLAYLIST_QUEUE_VIEWER_HPP