#pragma region Main

#include "../../include/qt-classes/media-widget.hpp"
#include <obs-module.h>
#include <plugin-support.h>

MediaWidget::MediaWidget(const MediaContext *media_context, e_MediaStringifyTYPE media_stringify_type, QWidget *parent)
	: QWidget(parent),
	  media_stringify_type(media_stringify_type)
{
	this->media_context = media_context;
	QVBoxLayout *layout = new QVBoxLayout(this);

	this->setStyleSheet("QWidget {"
			    "border: 2px solid rgb(89, 94, 109);" // Gray border
			    "border-radius: 5px;"                 // Rounded corners
			    "padding: 5px;"                       // Padding around content
			    "background-color: rgb(52, 51, 61);"  // Light background
			    "}");

	switch (this->media_stringify_type) {
	case MEDIA_STRINGIFY_TYPE_PATH:
		label = new QLabel(QString::fromStdString(media_context->path), this);
		break;
	case MEDIA_STRINGIFY_TYPE_FILENAME:
		/* code */
		label = new QLabel(QString::fromStdString(media_context->filename), this);
		break;
	case MEDIA_STRINGIFY_TYPE_NAME:
		label = new QLabel(QString::fromStdString(media_context->name), this);
		break;
	case MEDIA_STRINGIFY_TYPE_EXTENSION:
		label = new QLabel(QString::fromStdString(media_context->ext), this);
		break;
	default:
		break;
	}

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

void MediaWidget::update_media_data(e_MediaStringifyTYPE *media_stringify_type)
{
	if (media_stringify_type != nullptr) {
		if (this->media_stringify_type != *media_stringify_type) {
			this->media_stringify_type = *media_stringify_type;
		}
	}

	switch (this->media_stringify_type) {
	case MEDIA_STRINGIFY_TYPE_PATH:
		label->setText(QString::fromStdString(media_context->path));
		break;
	case MEDIA_STRINGIFY_TYPE_FILENAME:
		/* code */
		label->setText(QString::fromStdString(media_context->filename));
		break;
	case MEDIA_STRINGIFY_TYPE_NAME:
		label->setText(QString::fromStdString(media_context->name));
		break;
	case MEDIA_STRINGIFY_TYPE_EXTENSION:
		label->setText(QString::fromStdString(media_context->ext));
		break;
	default:
		break;
	}
}

void MediaWidget::remove_widget(bool delete_later)
{
	QWidget *parent_widget = parentWidget();
	if (parent_widget) {
		QLayout *layout = parent_widget->layout();
		if (layout) {
			layout->removeWidget(this);
			if (delete_later == true) {
				deleteLater();
			}
		}
	}
}

#pragma endregion