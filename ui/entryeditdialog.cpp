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

EntryEditDialog::EntryEditDialog(QKdbxDatabase::Version version , QWidget *parent) :
	QDialog(parent),
	fentry(version.parent()),
	fversion(new Kdbx::Database::Version(*version.get())),
	ui(new Ui::EntryEditDialog)
{
	assert(fentry);

	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose, true);
	setWindowTitle(tr("Entry: %1").arg(QKdbxDatabase::entryString(version, Kdbx::Database::Version::titleString)));

	ui->tabWidget->fromVersion(version);

	setDisabled(fentry.model()->frozen());
	connect(fentry.model(), &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
	connect(fentry.model(), &QObject::destroyed, this, &EntryEditDialog::disable);
	connect(fentry.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &EntryEditDialog::onGroupRemove);
	connect(fentry.model(), &QKdbxDatabase::beginEntryRemove, this, &EntryEditDialog::onEntryRemove);
}

EntryEditDialog::~EntryEditDialog()
{
	delete ui;
}

void EntryEditDialog::disable(){
	fentry = QKdbxDatabase::Entry();
	setWindowTitle(windowTitle() + tr(" (dropped)"));
	ui->updateButton->setEnabled(false);
	ui->tabWidget->setEnabled(false);
}

void EntryEditDialog::on_updateButton_clicked(){

	if (!fentry){
		reject();
		return;
	}

	ui->tabWidget->toVersion(fversion.get());

	fentry.addVersion(std::move(fversion), fentry.versions());
	accept();
	close();
}

void EntryEditDialog::onEntryRemove(QKdbxDatabase::Group parent, size_t index){
	if (fentry == parent.entry(index))
		disable();
}

void EntryEditDialog::onGroupRemove(const QModelIndex& parent, int begin, int end){
	QKdbxDatabase::Group g = fentry.model()->group(parent);

	for (;begin <= end; ++begin){
		if (fentry->ancestor(g.group(begin))){
			disable();
			return;
		}

	}
}

