#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class MediaWidget : public QWidget {
public:
	explicit MediaWidget(const QString &mediaName, QWidget *parent = nullptr);
};