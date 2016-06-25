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
		Save =0x1,
		SaveAs,
		NewEntry,
		NewGroup,
		Settings
	};
	Q_DECLARE_FLAGS( StandardBarActions, StandardBarAction)

	inline DatabaseViewWidget(QWidget* parent=0) noexcept
		:QWidget(parent)
	{}

	virtual QIcon icon() const =0;
	virtual QString name() const =0;
	virtual QString style() const =0;
	virtual QUndoStack* undoStack() =0;
	virtual StandardBarActions standardBarActions() =0;
	virtual void actionActivated(StandardBarAction action)=0;

protected slots:

signals:
	void nameChanged(QString name);
	void actionsUpdated();

};

Q_DECLARE_OPERATORS_FOR_FLAGS( DatabaseViewWidget::StandardBarActions)

#endif // DATABASEVIEWWIDGET_H
