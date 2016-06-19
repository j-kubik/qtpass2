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
#include "entryeditdialog.h"
#include "ui_entryeditdialog.h"




#include "qkdbxgroup.h"
#include "qkdbxdatabase.h"

#include "icondialog.h"

#include <QToolTip>


#include <QStringListModel>

EntryEditDialog::EntryEditDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Version version , QWidget *parent) :
	QDialog(parent),
	fdb(version.model()),
	fversion(new Kdbx::Database::Version(*version.get())),
	fuuid(version.parent()->uuid()),
	ui(new Ui::EntryEditDialog)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	ui->setupUi(this);
	setWindowTitle(tr("Entry: %1").arg(QKdbxDatabase::entryString(version, Kdbx::Database::Version::titleString)));

	ui->tabWidget->fromVersion(version);

	connect(fdb, &QKdbxDatabase::databaseChanged, this, &EntryEditDialog::disable);
	connect(fdb, &QObject::destroyed, this, &EntryEditDialog::disable);
}

EntryEditDialog::~EntryEditDialog()
{
	delete ui;
}

void EntryEditDialog::disable(){
	//fparentGroup = QKdbxDatabase::Group();
	fdb = nullptr;
	setWindowTitle(windowTitle() + tr(" (dropped)"));
	ui->updateButton->setEnabled(false);
	ui->tabWidget->setEnabled(false);
}

void EntryEditDialog::on_updateButton_clicked(){

	if (!fdb){
		reject();
		return;
	}

	ui->tabWidget->toVersion(fversion.get());
	QKdbxDatabase::Entry e = fdb->entry(fuuid);
	if (!e){
		reject();
		return;
	}

	e.addVersion(std::move(fversion), e.versions());
	accept();
	disable();
}

