#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

#include <QFuture>
#include <QPromise>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/types/playlist-source-types.hpp"

typedef struct PlaylistData PlaylistData;

class PlaylistWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	bool expanded;

public:
	const PlaylistData *playlist_data;
	explicit PlaylistWidget(const PlaylistData *playlist, QWidget *parent = nullptr);
	void toggleMediaVisibility();
	void update_playlist_name();
	void update_playlist_data();
	void remove_widget();
	void create_media_widget(MediaData *media_data, std::function<void(MediaWidget *)> callback);
	void add_media_widget(MediaWidget *mediaWidget);
};
#endif // PLAYLIST_WIDGET_HPP