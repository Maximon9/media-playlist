#pragma region Main

#include "../../include/qt-classes/multi-playlist-queue-viewer.hpp"
#include "../../include/qt-classes/playlist-queue-widget.hpp"

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

	resize(600, 600);

	setLayout(layout);
}

void MultiPlaylistQueueViewer::addPlaylistWidget(PlaylisQueueWidget *playlist_widget)
{
	multi_playlist_queue_viewer->contentLayout->addWidget(playlist_widget);
}

void MultiPlaylistQueueViewer::updatePlaylists()
{
	// Add new playlists
	for (size_t i = 0; i < this->playlist_datas.size(); i++) {
		const PlaylistData *playlist_data = this->playlist_datas[i];
		if (playlist_data == nullptr) {
			continue;
		}
		playlist_data->playlist_widget->update_playlist_data();
	}
}

MultiPlaylistQueueViewer *multi_playlist_queue_viewer = nullptr;

#pragma endregion