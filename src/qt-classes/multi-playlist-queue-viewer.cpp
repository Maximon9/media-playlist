#pragma region Main

#include "../../include/qt-classes/multi-playlist-queue-viewer.hpp"
#include "../../include/qt-classes/playlist-widget.hpp"

/* MultiPlaylistQueueViewer::MultiPlaylistQueueViewer(QWidget *parent) : QWidget(parent)
{
	this->playlist_datas = {};

	this->layout = new QVBoxLayout(this);

	QLabel *label = new QLabel("Hi, this is my custom dock", this);

	layout->addWidget(label);

	setLayout(this->layout);
} */

MultiPlaylistQueueViewer::MultiPlaylistQueueViewer(QWidget *parent) : QWidget(parent)
{
	this->playlist_datas = {};
	layout = new QVBoxLayout(this);

	// Scroll area setup
	scrollArea = new QScrollArea(this);
	// scrollArea->setSizePolicy();
	scrollArea->setWidgetResizable(true);

	// Container inside scroll area
	contentWidget = new QWidget();
	contentLayout = new QVBoxLayout(contentWidget);

	contentWidget->setLayout(contentLayout);

	contentLayout->setAlignment(Qt::AlignTop);

	scrollArea->setWidget(contentWidget);

	layout->addWidget(scrollArea);
	setLayout(layout);
}

void MultiPlaylistQueueViewer::addPlaylistWidget(PlaylistWidget *param_playlist_widget)
{
	multi_playlist_queue_viewer->contentLayout->addWidget(param_playlist_widget);
}

void MultiPlaylistQueueViewer::updatePlaylists()
{
	// Add new playlists
	for (size_t i = 0; i < this->playlist_datas.size(); i++) {
		const PlaylistWidgetData *playlist_widget_data = this->playlist_datas[i];
		if (playlist_widget_data == nullptr) {
			continue;
		}
		playlist_widget_data->param_playlist_widget->update_playlist_data();
	}
}

MultiPlaylistQueueViewer *multi_playlist_queue_viewer = nullptr;

#pragma endregion