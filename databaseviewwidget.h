#ifndef DATABASEVIEWWIDGET_H
#define DATABASEVIEWWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QUndoStack>
#include <QFlags>

class DatabaseViewWidget: public QWidget{
	Q_OBJECT
public:

	enum StandardBarAction{
		NewEntry,
		NewGroup,
		Settings
	};
	Q_DECLARE_FLAGS( StandardBarActions, StandardBarAction)

	using QWidget::QWidget;

	virtual QIcon icon() const =0;
	virtual QString name() const =0;
	virtual QString style() const =0;
	virtual QUndoStack* undoStack() =0;
	virtual StandardBarActions standardBarActions() =0;
	virtual void actionActivated(StandardBarAction action)=0;

signals:
	void standardBarActionsUpdated(StandardBarActions actions);

};

Q_DECLARE_OPERATORS_FOR_FLAGS( DatabaseViewWidget::StandardBarActions)

#endif // DATABASEVIEWWIDGET_H
