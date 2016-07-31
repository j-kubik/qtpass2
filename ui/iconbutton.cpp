#include "ui/iconbutton.h"
#include "qkdbxdatabase.h"
#include "ui/icondialog.h"


IconButton::IconButton(QWidget* parent)
	:QToolButton(parent),
	  fdatabase(nullptr),
	  fdefaultIcon(Kdbx::StandardIcon::Key)
{
	connect(this, &IconButton::clicked, this, &IconButton::onActivated);
}

void IconButton::setDatabase(QKdbxDatabase* database){

	if (fdatabase){
		disconnect(fdatabase->icons(), &QKdbxDatabase::Icons::rowsAboutToBeRemoved, this, &IconButton::onIconRemoved);
		disconnect(fdatabase, &QObject::destroyed, this, &IconButton::onModelDestroyed);
	}

	fdatabase = database;
	connect(fdatabase->icons(), &QKdbxDatabase::Icons::rowsAboutToBeRemoved, this, &IconButton::onIconRemoved);
	connect(fdatabase, &QObject::destroyed, this, &IconButton::onModelDestroyed);
}

void IconButton::setDefaultIcon(Kdbx::StandardIcon icon){
	fdefaultIcon = icon;
}

Kdbx::StandardIcon IconButton::defaultIcon(){
	return fdefaultIcon;
}

Kdbx::Icon IconButton::currentIcon(){
	return fcurrentIcon;
}

void IconButton::setCurrentIcon(Kdbx::Icon icon){
	if (fdatabase){
		setIcon(fdatabase->icons()->icon(icon));
		fcurrentIcon = icon;
	}else if (icon.type() == Kdbx::Icon::Type::Standard){
		setIcon(QKdbxDatabase::Icons::standardIcon(icon.standard()));
		fcurrentIcon = icon;
	}else{
		setIcon(QKdbxDatabase::Icons::standardIcon(fdefaultIcon));
		fcurrentIcon = fdefaultIcon;
	}
}

void IconButton::onIconRemoved(QModelIndex idx, int first, int last){
	Kdbx::unused(idx);

	if (fcurrentIcon.type() == Kdbx::Icon::Type::Custom){
		int index = fdatabase->get()->iconIndex(fcurrentIcon.custom());
		if (index >= first && index <= last)
			setCurrentIcon(Kdbx::Icon(fdefaultIcon));
	}
}

void IconButton::onModelDestroyed(){
	fdatabase = nullptr;
	if (fcurrentIcon.type() == Kdbx::Icon::Type::Custom)
		setCurrentIcon(Kdbx::Icon(fdefaultIcon));
}


void IconButton::onActivated(){
	if (!fdatabase)
		return;

	IconDialog* dlg = new IconDialog(fdatabase, fcurrentIcon, this);
	connect(dlg, &IconDialog::iconAccepted, this, &IconButton::setCurrentIcon);
	dlg->show();
}


