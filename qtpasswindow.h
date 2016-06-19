#ifndef QTPASSWINDOW_H
#define QTPASSWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QUndoGroup>

class OpenDialog;

namespace Ui {
class QtPassWindow;
}

class DatabaseViewWidget;

class QtPassWindow : public QMainWindow{
	Q_OBJECT
public:
	explicit QtPassWindow(QWidget *parent = 0);
	~QtPassWindow();

private slots:
	void on_actionOpen_triggered();

	void onOpenDb_accepted();
	void onOpen_accepted();

	void on_actionGenerate_password_triggered();

	void on_tabWidget_currentChanged(int index);

	void on_actionAddEntry_triggered();

	void on_actionAddGroup_triggered();

	void on_actionDatabaseSettings_triggered();

private:
	virtual void closeEvent(QCloseEvent * event) override;

	void addWindow(DatabaseViewWidget* widget);

	Ui::QtPassWindow *ui;
	QFileDialog* openDbDialog;
	OpenDialog* openDialog;

	QString openDbFilename;
	QUndoGroup* undoGroup;
};

#endif // QTPASSWINDOW_H
