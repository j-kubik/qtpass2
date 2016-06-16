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
#include <QMenu>

namespace Ui {
class DatabaseView;
}

class QKdbxDatabase;
class QKdbxGroup;

class DatabaseView : public DatabaseViewWidget{
	Q_OBJECT

public:
	explicit DatabaseView(std::unique_ptr<QKdbxDatabase> database, QWidget *parent = 0);
	~DatabaseView();

	QIcon icon() const override;
	QString name() const override;
	QString style() const override;
	void saveSettings(QSettings& s) const override;
	void applySettings(QSettings& s) override;
	QUndoStack* undoStack() override;
	void addEntryAction() override;


private slots:
	void headerContextMenuColumnVisibility();
	void headerContextMenuDontSort();
	void headerSectionClicked(int index);

	void currentGroupChanged(const QModelIndex & current);
	void currentEntryChanged(const QModelIndex & current);

	void on_entriesView_doubleClicked(const QModelIndex &index);

	void on_actionDeleteGroup_triggered();

	void on_actionGroupProperties_triggered();

	void on_actionEntryEdit_triggered();

	void on_actionEntryVersions_triggered();

	void on_actionEntryDelete_triggered();

	void on_actionEntryNew_triggered();

private:

	Ui::DatabaseView *ui;
	std::unique_ptr<QKdbxDatabase> database;
	QKdbxGroup* fgroup;
	QList<QAction*> headerActions;
};

#endif // DATABASEVIEW_H
