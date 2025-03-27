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

	// Prevent mediaContainer from expanding unnecessarily
	mediaContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	mediaLayout->setSizeConstraint(QLayout::SetFixedSize);

	// Center all media items inside the mediaContainer
	mediaLayout->setAlignment(Qt::AlignCenter);

	// Initially hide media items
	mediaContainer->setVisible(expanded);

	layout->addWidget(toggleButton);
	layout->addWidget(mediaContainer);

	// Align mediaContainer to the top center
	layout->setAlignment(mediaContainer, Qt::AlignTop | Qt::AlignHCenter);

	setLayout(layout);

	mediaContainer->setLayout(mediaLayout);
	mediaContainer->adjustSize(); // Ensure it shrinks to fit content

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
		playlist_data->queue[i]->media_widget->update_media_data();
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

// QFuture<MediaWidget *> PlaylistWidget::create_media_widget(MediaData *media_data)
// {
// 	QPromise<MediaWidget *> promise;
// 	QFuture<MediaWidget *> future = promise.future();

// 	QMetaObject::invokeMethod(
// 		this,
// 		[=, p = std::move(promise)]() mutable {
// 			MediaWidget *widget = new MediaWidget(media_data, this);
// 			p.addResult(widget);
// 			p.finish();
// 		},
// 		Qt::QueuedConnection);

// 	return future;
// }

void PlaylistWidget::create_media_widget(MediaData *media_data, std::function<void(MediaWidget *)> callback)
{
	QMetaObject::invokeMethod(
		this,
		[=]() {
			MediaWidget *widget = new MediaWidget(media_data, this);
			if (callback) {
				callback(widget);
			}
		},
		Qt::QueuedConnection);
}

void PlaylistWidget::add_media_widget(MediaWidget *mediaWidget)
{
	// Add the MediaWidget to this PlaylistWidget's layout
	QMetaObject::invokeMethod(this, [=]() { mediaLayout->addWidget(mediaWidget); }, Qt::QueuedConnection);
}

#pragma endregion