#ifndef ICONBUTTON_H
#define ICONBUTTON_H

#include <QToolButton>
#include <QModelIndex>
#include <libkeepass2pp/icon.h>

class QKdbxDatabase;

class IconButton : public QToolButton{
	Q_OBJECT
public:
	IconButton(QWidget* parent);

	void setDatabase(QKdbxDatabase* database);

	void setDefaultIcon(Kdbx::StandardIcon icon);
	Kdbx::StandardIcon defaultIcon();

	Kdbx::Icon currentIcon();

public slots:
	void setCurrentIcon(Kdbx::Icon icon);

private slots:
	void onIconRemoved(QModelIndex idx, int first, int last);
	void onModelDestroyed();
	void onActivated();

private:
	QKdbxDatabase* fdatabase;
	Kdbx::StandardIcon fdefaultIcon;
	Kdbx::Icon fcurrentIcon;
};

#endif // ICONBUTTON_H
