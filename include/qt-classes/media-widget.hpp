#ifndef MEDIA_WIDGET_HPP
#define MEDIA_WIDGET_HPP

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class MediaWidget : public QWidget {
public:
	explicit MediaWidget(const QString &mediaName, QWidget *parent = nullptr);
};

#endif // MEDIA_WIDGET_HPP