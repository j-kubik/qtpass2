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
#ifndef DATABASEVIEW_H
#define DATABASEVIEW_H

#include "databaseviewwidget.h"

#include <libkeepass2pp/database.h>
#include <QThread>
#include <QMenu>

namespace Ui {
class QKdbxView;
}

class QKdbxDatabase;
class QKdbxGroup;

class QKdbxView : public DatabaseViewWidget{
	Q_OBJECT

public:
	explicit QKdbxView(std::unique_ptr<QKdbxDatabase> database, QWidget *parent = 0);
	~QKdbxView();

	QIcon icon() const override;
	QString name() const override;
	QString style() const override;
	QUndoStack* undoStack() override;
	StandardBarActions standardBarActions() override;
	void actionActivated(StandardBarAction action) override;

private slots:
	void headerContextMenuColumnVisibility();
	void headerContextMenuDontSort();
	void headerSectionClicked(int index);

	void currentGroupChanged(const QModelIndex & current);
	void currentEntryChanged(const QModelIndex & current);

	void onFrozenChanged(bool frozen);

	void on_entriesView_doubleClicked(const QModelIndex &index);

	void on_actionDeleteGroup_triggered();

	void on_actionGroupProperties_triggered();

	void on_actionEntryEdit_triggered();

	void on_actionEntryVersions_triggered();

	void on_actionEntryDelete_triggered();

	void on_actionEntryNew_triggered();

private:

	Ui::QKdbxView *ui;
	std::unique_ptr<QKdbxDatabase> database;
	QString filename;

	QKdbxGroup* fgroup;
	QList<QAction*> headerActions;
};

#endif // DATABASEVIEW_H
