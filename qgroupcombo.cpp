#include "qgroupcombo.h"
#include <QTreeView>
#include <QMouseEvent>

#include <iostream>

QGroupCombo::QGroupCombo(QWidget* parent)
	:QComboBox(parent),
	  fview(new QTreeView(this)),
	  skipHide(false)
{
	fview->viewport()->installEventFilter(this);
	setView(fview);
}

void QGroupCombo::setCurrentIndex(const QModelIndex& index){
	setRootModelIndex(index.parent());
	QComboBox::setCurrentIndex(index.row());
}

bool QGroupCombo::eventFilter(QObject* object, QEvent* event){
	//std::cout << "Event: " << int(event->type()) <<  ", object: " << object << ", viewport: " << fview->viewport() << std::endl;
	if (event->type() == QEvent::MouseButtonPress && object == fview->viewport())
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		QModelIndex index = fview->indexAt(mouseEvent->pos());
		if (!fview->visualRect(index).contains(mouseEvent->pos()))
			skipHide = true;
	}
	return false;
}

void QGroupCombo::showPopup(){
	setRootModelIndex(QModelIndex());
	QComboBox::showPopup();
}

void QGroupCombo::hidePopup(){
	//std::cout << "Skip hide: " << skipHide << std::endl;
	if (skipHide){
		skipHide = false;
		return;
	}
	setCurrentIndex(fview->currentIndex());
	QComboBox::hidePopup();
}


