#pragma region Main

#include "../../include/qt-classes/media-widget.hpp"
#include <obs-module.h>
#include <plugin-support.h>

MediaWidget::MediaWidget(const MediaData *media_data, QWidget *parent) : QWidget(parent)
{
	moveToThread(parent->thread());
	this->media_data = media_data;
	QVBoxLayout *layout = new QVBoxLayout(this);
	obs_log(LOG_INFO, "Queue Media Name: %s", media_data->name.c_str());
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