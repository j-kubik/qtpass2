#ifndef NOTIFICATIONFRAME_H
#define NOTIFICATIONFRAME_H

#include <QFrame>

class NotificationFrame : public QFrame{
	Q_OBJECT
public:

	enum Button{
		Dismiss = 0x1,
		Update = 0x2,
	};

	Q_DECLARE_FLAGS(Buttons, Button)

	NotificationFrame(QWidget* parent = 0);
	~NotificationFrame();

	inline Buttons buttons() noexcept{
		return fbuttons;
	}

	QString text();

public slots:
	void setButtons(Buttons buttons);
	void setText(QString text);

signals:
	void dismissed();
	void updated();

private:
	Q_PROPERTY(Buttons buttons READ buttons WRITE setButtons)
	Q_PROPERTY(QString text READ text WRITE setText)
	Buttons fbuttons;

	class Ui;
	Ui* ui;
};


Q_DECLARE_OPERATORS_FOR_FLAGS(NotificationFrame::Buttons)

#endif // NOTIFICATIONFRAME_H
