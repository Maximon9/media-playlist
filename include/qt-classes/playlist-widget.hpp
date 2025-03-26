#ifndef PLAYLIST_WIDGET_HPP
#define PLAYLIST_WIDGET_HPP

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
	PlaylistData *playlist;
	explicit PlaylistWidget(PlaylistData *playlist, QWidget *parent = nullptr);
	void toggleMediaVisibility();
};
#endif // PLAYLIST_WIDGET_HPP