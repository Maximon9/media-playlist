#include "Qt"
#include <QDockWidget>
#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <obs-module.h>
#include <plugin-support.h>
#include <deque>
#include "../include/source-types/playlist-source-types.hpp"

typedef std::deque<PlaylistData> PlaylistSources;

class PlaylistQueueViewer : public QWidget {
	// Q_OBJECT
private:
	PlaylistSources *playlist_sources;
	QWidget *parent;
	QVBoxLayout *layout;
	QLabel *label;
	// obs_data_t *settings;
	// QSpinBox queue_list_size;
	// MediaFileDataArray all_media;
	// obs_source_t *source;
	// obs_source_t *media_source;
	// bool shuffle_queue;
	// e_StartBehavior playlist_start_behavior;
	// e_EndBehavior playlist_end_behavior;
	// int loop_index;
	// bool infinite;
	// int loop_count;
	// e_LoopEndBehavior loop_end_behavior;
	// int song_history_limit;
	// bool debug;

public:
	explicit PlaylistQueueViewer(/* const QString &title,  */ QWidget *parent);

	// Destructor: This is called when an object is destroyed.
	~PlaylistQueueViewer();
};

static PlaylistQueueViewer *playlist_queue_viewer;