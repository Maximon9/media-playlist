#include "../../include/qt-classes/playlist-media-widget.hpp"

MediaWidget::MediaWidget(const QString &mediaName, QWidget *parent) : QWidget(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	QLabel *label = new QLabel(mediaName, this);
	layout->addWidget(label);
	setLayout(layout);
}