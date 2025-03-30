#ifndef PLAYLIST_QUEUE_VIEWER_HPP
#define PLAYLIST_QUEUE_VIEWER_HPP

#include <QEventLoop>
#include <QScrollArea>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

typedef struct PlaylistData PlaylistData;

class PlaylisQueuetWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QHBoxLayout *buttonLayout;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	bool expanded;

#pragma region Main Widget Params
	bool is_main_widget;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;
#pragma endregion

public:
	const PlaylistData *playlist_data;
	explicit PlaylisQueuetWidget(const PlaylistData *playlist, QWidget *parent = nullptr,
				     bool is_main_widget = false);
	void toggleMediaVisibility();
	void update_playlist_name();
	void update_playlist_data();
	void remove_widget();
	// void create_media_widget(MediaData *media_data, std::function<void(MediaWidget *)> callback);
	MediaWidget *create_media_widget(MediaData *media_data);
	void add_media_widget(MediaWidget *mediaWidget);
};
#endif // PLAYLIST_QUEUE_VIEWER_HPP