#pragma region Main

#include "../../include/qt-classes/playlist-widget.hpp"
#include "../../include/qt-classes/media-widget.hpp"

PlaylistWidget::PlaylistWidget(const PlaylistData *playlist_data, QWidget *parent) : QWidget(parent), expanded(false)
{
	this->playlist_data = playlist_data;

	// Main layout for the PlaylistWidget
	layout = new QVBoxLayout(this);

	// Create a layout for the toggleButton to make it expand horizontally
	buttonLayout = new QHBoxLayout();

	toggleButton = new QPushButton(QString::fromStdString(playlist_data->name), this);
	toggleButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed); // Make button expand horizontally

	buttonLayout->addWidget(toggleButton);
	buttonLayout->setAlignment(Qt::AlignTop);

	mediaContainer = new QWidget(this);
	mediaLayout = new QVBoxLayout(mediaContainer);

	// Shrink the mediaContainer horizontally
	mediaContainer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum); // Fixed size for vertical

	// Set the mediaLayout properties to shrink to the content
	mediaLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	mediaLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter); // Align to top and center

	// Initially hide media items
	mediaContainer->setVisible(expanded);

	// Add the button layout and media container to the main layout
	layout->addLayout(buttonLayout); // This ensures the button takes up full width
	layout->addWidget(mediaContainer);

	setLayout(layout);

	mediaContainer->setLayout(mediaLayout);
	mediaContainer->adjustSize(); // Shrink the container to fit its content

	// Ensure that the container is aligned at the top
	layout->setAlignment(mediaContainer, Qt::AlignTop); // Align mediaContainer to the top when collapsed
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