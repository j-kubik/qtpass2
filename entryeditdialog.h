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
#ifndef ENTRYEDITDIALOG_H
#define ENTRYEDITDIALOG_H

#include <QDialog>

#include <libkeepass2pp/databasemodel.h>

#include <QAbstractTableModel>
//#include <QTreeView>

namespace Ui {
class EntryEditDialog;
}

class QKdbxGroup;
class QKdbxDatabase;

class EntryEditDialog : public QDialog{
private:
	Q_OBJECT
	QKdbxDatabase* fdb;
	Kdbx::Database::Version::Ptr fversion;
	Uuid fuuid;

public:
	explicit EntryEditDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Version version, QWidget *parent = 0);

	~EntryEditDialog();

private slots:
	void disable();

	void on_updateButton_clicked();

private:
	Ui::EntryEditDialog *ui;
};

#endif // ENTRYEDITDIALOG_H
