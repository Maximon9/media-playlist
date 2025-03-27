#pragma region Main

#include "../../include/qt-classes/media-widget.hpp"
#include <obs-module.h>
#include <plugin-support.h>

MediaWidget::MediaWidget(const MediaData *media_data, QWidget *parent) : QWidget(parent)
{
	this->media_data = media_data;
	QVBoxLayout *layout = new QVBoxLayout(this);

	this->setStyleSheet("QWidget {"
			    "border: 2px solid rgb(89, 94, 109);" // Gray border
			    "border-radius: 5px;"                 // Rounded corners
			    "padding: 5px;"                       // Padding around content
			    "background-color: rgb(52, 51, 61);"  // Light background
			    "}");

	label = new QLabel(QString::fromStdString(media_data->name), this);

	label->setStyleSheet("QLabel {"
			     "border: 2px solid transparent;" // Gray border
			     "border-radius: 5px;"            // Rounded corners
			     "padding: 5px;"                  // Padding around content
			     "background-color: transparent;" // Light background
			     "}");

	label->setAlignment(Qt::AlignTop);
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