#ifndef LOCALCLIPBOARD_H
#define LOCALCLIPBOARD_H

#include <QObject>

class LocalClipboard : public QObject{
	Q_OBJECT
public:
	explicit LocalClipboard(QObject *parent = 0);

	void setValue();

signals:

public slots:
};

#endif // LOCALCLIPBOARD_H
