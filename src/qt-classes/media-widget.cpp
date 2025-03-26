#pragma region Main

#include "../../include/qt-classes/media-widget.hpp"

MediaWidget::MediaWidget(const MediaFileData *media_file_data, QWidget *parent) : QWidget(parent)
{
	this->media_file_data = media_file_data;
	QVBoxLayout *layout = new QVBoxLayout(this);
	label = new QLabel(QString::fromStdString(media_file_data->name), this);
	layout->addWidget(label);
	setLayout(layout);
}

void MediaWidget::update_media_file_data()
{
	label->setText(QString::fromStdString(media_file_data->name));
}

void MediaWidget::remove_widget()
{
	QWidget *parent = parent;
	QBoxLayout *layout = qobject_cast<QBoxLayout *>(parent->layout());
	layout->removeWidget(this);
}

#pragma endregion