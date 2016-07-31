#ifndef QTPASSWINDOW_H
#define QTPASSWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QUndoGroup>

#include "executor.h"
#include "databaseviewwidget.h"


class OpenDialog;
class NewDialog;

namespace Ui {
class QtPassWindow;
}

class QtPassWindow : public QMainWindow{
	Q_OBJECT
public:
	explicit QtPassWindow(QWidget *parent = 0);
	~QtPassWindow();

	void updateActions(DatabaseViewWidget* w);

private slots:
	void on_actionOpen_triggered();
	void onOpenDb_accepted();
	void onNewDb_accepted();

	void on_actionGenerate_password_triggered();

	void on_tabWidget_currentChanged(int index);

	void on_actionAddEntry_triggered();

	void on_actionAddGroup_triggered();

	void on_actionDatabaseSettings_triggered();

	void on_actionExit_triggered();

	void tabActionsUpdated();

	void on_actionSave_triggered();

	void on_actionSaveAs_triggered();

	void on_actionClose_triggered();

	void on_tabWidget_tabCloseRequested(int index);

	void on_actionIcons_triggered();

	void on_actionNew_triggered();

private:
	static void openArgsFile(Executor::Ptr callbacks, QtPassWindow* ths, QString password, QString keyFilePath, QString database);
	static void doOpenAsync(Executor::Ptr callbacks, QtPassWindow* ths, QString filename);

	void closeEvent(QCloseEvent * event) override;

	void addWindow(DatabaseViewWidget* widget);
	void removeWindow(int index);

	Executor::Ptr executor;

	Ui::QtPassWindow *ui;
	QFileDialog* openDbDialog;
	OpenDialog* openDialog;
	NewDialog* newDialog;

	QUndoGroup* undoGroup;
};

#endif // QTPASSWINDOW_H
