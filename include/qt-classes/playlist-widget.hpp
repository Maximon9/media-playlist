#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

class PlaylistWidget : public QWidget {
	Q_OBJECT
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	bool expanded;

public:
	explicit PlaylistWidget(QWidget *parent = nullptr);
	void toggleMediaVisibility();
};