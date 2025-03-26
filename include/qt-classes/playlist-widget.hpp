#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "../include/source-types/playlist-source-types.hpp"

class PlaylistWidget : public QWidget {
private:
	QVBoxLayout *layout;
	QPushButton *toggleButton;
	QWidget *mediaContainer;
	QVBoxLayout *mediaLayout;
	bool expanded;

public:
	explicit PlaylistWidget(const PlaylistData &playlist, QWidget *parent = nullptr);
	void toggleMediaVisibility();
};