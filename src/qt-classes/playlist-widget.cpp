#pragma region Main

#include "../../include/qt-classes/playlist-widget.hpp"
#include "../../include/qt-classes/media-widget.hpp"

PlaylistWidget::PlaylistWidget(const PlaylistData *playlist_data, QWidget *parent) : QWidget(parent), expanded(false)
{
	this->playlist_data = playlist_data;

	layout = new QVBoxLayout(this);
	toggleButton = new QPushButton(QString::fromStdString(playlist_data->name), this);
	mediaContainer = new QWidget(this);
	mediaLayout = new QVBoxLayout(mediaContainer);

	// Initially hide media items
	mediaContainer->setVisible(expanded);
	layout->addWidget(toggleButton);
	layout->addWidget(mediaContainer);
	setLayout(layout);

	// Populate media items
	/* for (size_t i = 0; i < playlist->queue.size(); i++) {
		const MediaData *media_file_data = &playlist->queue[i];
		mediaLayout->addWidget(new MediaWidget(media_file_data, this));
	} */
	mediaContainer->setLayout(mediaLayout);

	// Toggle playlist expansion
	connect(toggleButton, &QPushButton::clicked, this, &PlaylistWidget::toggleMediaVisibility);
}

void PlaylistWidget::toggleMediaVisibility()
{
	expanded = !expanded;
	mediaContainer->setVisible(expanded);
}

void PlaylistWidget::update_playlist_name()
{
	toggleButton->setText(QString::fromStdString(playlist_data->name));
}

void PlaylistWidget::update_playlist_data()
{
	PlaylistWidget::update_playlist_name();
	for (size_t i = 0; i < playlist_data->queue.size(); i++) {
		const QueueMediaData *media_file_data = &playlist_data->queue[i];
		media_file_data->media_widget->update_media_file_data();
	}
}

void PlaylistWidget::remove_widget()
{
	QWidget *parent_widget = parentWidget();
	if (parent_widget) {
		QLayout *layout = parent_widget->layout();
		if (layout) {
			layout->removeWidget(this);
		}
	}
	deleteLater();
}

#pragma endregion