#include <QWidget>

class MediaWidget : public QWidget {
	Q_OBJECT
public:
	explicit MediaWidget(const QString &mediaName, QWidget *parent = nullptr);
};