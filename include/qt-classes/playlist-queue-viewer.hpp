#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <deque>
#include "../include/source-types/playlist-source-types.hpp"

typedef std::deque<PlaylistData *> PlaylistDatas;

class PlaylistQueueViewer : public QWidget {
	Q_OBJECT
private:
	QVBoxLayout *layout;
	QScrollArea *scrollArea;
	QWidget *contentWidget;
	QVBoxLayout *contentLayout;

public:
	PlaylistDatas *playlist_datas;
	explicit PlaylistQueueViewer(QWidget *parent = nullptr);
	void updatePlaylists();
};

static PlaylistQueueViewer *playlist_queue_viewer;