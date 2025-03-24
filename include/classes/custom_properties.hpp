#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class CustomProperties : public QDialog {
	Q_OBJECT
private:
public:
	explicit CustomProperties();

	// Destructor: This is called when an object is destroyed.
	~CustomProperties();
};