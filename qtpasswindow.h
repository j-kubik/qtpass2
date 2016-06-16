/*Copyright (C) 2016 Jaroslaw Kubik
 *
   This file is part of QtPass2.

QtPass2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QtPass2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QtPass2.  If not, see <http://www.gnu.org/licenses/>.
*/
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

class QtPassWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit QtPassWindow(QWidget *parent = 0);
	~QtPassWindow();

private slots:
	void on_actionOpen_triggered();

	void onOpenDb_accepted();
	void onOpen_accepted();

	void on_actionGenerate_password_triggered();

	//void on_actionDsAsDefault_triggered();

	void on_tabWidget_currentChanged(int index);

	void on_actionNewKdbx_triggered();

	void on_actionNewEntry_triggered();

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
