#pragma region Main

#include "../../include/qt-classes/media-widget.hpp"

MediaWidget::MediaWidget(const QueueMediaData *media_data, QWidget *parent) : QWidget(parent)
{
	this->media_data = media_data;
	QVBoxLayout *layout = new QVBoxLayout(this);
	label = new QLabel(QString::fromStdString(media_data->name), this);
	layout->addWidget(label);
	setLayout(layout);
}

void MediaWidget::update_media_data()
{
	label->setText(QString::fromStdString(media_data->name));
}

void MediaWidget::remove_widget()
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