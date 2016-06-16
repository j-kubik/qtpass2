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
#include "entryversionsdialog.h"
#include "ui_entryversionsdialog.h"

#include "entryeditdialog.h"

#include <QDateTime>
#include <QMessageBox>

#include "qkdbxdatabase.h"

EntryVersionsModel::EntryVersionsModel(QKdbxDatabase::Entry entry, QObject* parent)
	:QAbstractTableModel(parent),
	  fentry(entry){
	connect(entry.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &EntryVersionsModel::groupRemoved);
	connect(entry.model(), &QKdbxDatabase::beginEntryRemove, this, &EntryVersionsModel::entryRemove);
	connect(entry.model(), &QKdbxDatabase::beginVersionAdd, this, &EntryVersionsModel::beginVersionAdd);
	connect(entry.model(), &QKdbxDatabase::endVersionAdd, this, &EntryVersionsModel::endVersionAdd);
	connect(entry.model(), &QKdbxDatabase::beginVersionRemove, this, &EntryVersionsModel::beginVersionRemove);
	connect(entry.model(), &QKdbxDatabase::endVersionRemove, this, &EntryVersionsModel::endVersionRemove);
}


Qt::ItemFlags EntryVersionsModel::flags(const QModelIndex & index) const{
	unused(index);
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant EntryVersionsModel::data(const QModelIndex & index, int role) const{
	if (role != Qt::DisplayRole || !fentry || !index.isValid() || index.row() >= int(fentry->versions()))
		return QVariant();

	QKdbxDatabase::Version version = fentry.version(index.row());

	switch (index.column()) {
	case 0:{
		QDateTime t = QDateTime::fromTime_t(version->times.lastModification);
		t.setTimeSpec(Qt::LocalTime);
		return t.toString(Qt::ISODate);
	}
	case 1:{
		QDateTime t = QDateTime::fromTime_t(version->times.creation);
		t.setTimeSpec(Qt::LocalTime);
		return t.toString(Qt::ISODate);
	}
	case 2:{
		QDateTime t = QDateTime::fromTime_t(version->times.expires);
		t.setTimeSpec(Qt::LocalTime);
		return t.toString(Qt::ISODate);
	}
	case 3:{
		auto fnd = version->strings.find(Kdbx::Database::Version::titleString);
		if (fnd != version->strings.end()){
			auto plain = fnd->second.plainString();
			return QString::fromUtf8(plain.c_str(), plain.size());
		}
	}
	default:
		break;
	}

	return QVariant();

}
QVariant EntryVersionsModel::headerData(int section, Qt::Orientation orientation, int role) const{
	unused(orientation);

	if (role != Qt::DisplayRole)
		return QVariant();

	switch (section) {
	case 0:
		return tr("Last modified");
	case 1:
		return tr("Created");
	case 2:
		return tr("Expires at");
	case 3:
		return tr("Title");
	default:
		break;
	}

	return QVariant();
}
int EntryVersionsModel::rowCount(const QModelIndex & parent) const{
	if (parent.isValid() || !fentry)
		return 0;

	return fentry->versions();
}

int EntryVersionsModel::columnCount(const QModelIndex & parent) const{
	unused(parent);
	return 4;
}

void EntryVersionsModel::groupRemoved(QModelIndex parent, int begin, int end){
	if (fentry){
		QKdbxDatabase::Group group = fentry.model()->group(parent);
		QKdbxDatabase::Group ancestor = fentry.parent();
		while (ancestor && ancestor.parent() != group){
			ancestor = ancestor.parent();
		}
		if (ancestor){
			for (; begin<= end; ++begin){
				if (group.group(begin) == ancestor){
					fentry = QKdbxDatabase::Entry();
					return;
				}
			}
		}
	}
}

void EntryVersionsModel::entryRemove(QKdbxDatabase::Group parent, size_t index){
	QKdbxDatabase::Entry entry = parent.entry(index);
	if (entry == fentry){
		beginResetModel();
		fentry = QKdbxDatabase::Entry();
		endResetModel();
	}
}

void EntryVersionsModel::beginVersionAdd(QKdbxDatabase::Entry entry, size_t index){
	if (entry == fentry){
		beginInsertRows(QModelIndex(), index, index);
	}
}

void EntryVersionsModel::endVersionAdd(QKdbxDatabase::Entry entry, size_t){
	if (entry == fentry){
		endInsertRows();
	}
}

void EntryVersionsModel::beginVersionRemove(QKdbxDatabase::Entry entry, size_t index){
	if (entry == fentry){
		beginRemoveRows(QModelIndex(), index, index);
	}
}

void EntryVersionsModel::endVersionRemove(QKdbxDatabase::Entry entry, size_t){
	if (entry == fentry){
		endRemoveRows();
	}
}



//------------------------------------------------------------------------------

EntryVersionsDialog::EntryVersionsDialog(QKdbxDatabase::Entry entry, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EntryVersionsDialog),
	entryVersions(new EntryVersionsModel(entry, this))
{
	ui->setupUi(this);
	ui->treeView->setModel(entryVersions);
	ui->treeView->header()->setSectionResizeMode(QHeaderView::Stretch);

	currentChanged(ui->treeView->selectionModel()->currentIndex(), QModelIndex());
	connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
			this, SLOT(currentChanged(QModelIndex,QModelIndex)));
	connect(entryVersions, &EntryVersionsModel::rowsInserted, this, &EntryVersionsDialog::itemContChanged);
	connect(entryVersions, &EntryVersionsModel::rowsRemoved, this, &EntryVersionsDialog::itemContChanged);
}

EntryVersionsDialog::~EntryVersionsDialog()
{
	delete ui;
}

void EntryVersionsDialog::showCurrentItem(){
	showItem(ui->treeView->selectionModel()->currentIndex());
}

void EntryVersionsDialog::itemContChanged(){
	currentChanged(ui->treeView->currentIndex(), QModelIndex());
}

void EntryVersionsDialog::showItem(const QModelIndex& index){
	if (!index.isValid())
		return;

	if (index.row() >= int(entryVersions->entry()->versions()))
		return;

	QKdbxDatabase::Version version = entryVersions->entry().version(index.row());

	EntryEditDialog* edit = new EntryEditDialog(version, parentWidget());
	edit->setAttribute( Qt::WA_DeleteOnClose, true );
	edit->show();
}

void EntryVersionsDialog::currentChanged(const QModelIndex& current, const QModelIndex& previous){
	unused(previous);
	ui->show->setEnabled(current.isValid());

	if (current.isValid()){
		const Kdbx::Database::Version* v = entryVersions->entry()->version(current.row());
		ui->restore->setEnabled(v != entryVersions->entry()->latest());

		ui->erase->setEnabled(entryVersions->entry()->versions() > 1);
	}else{
		ui->restore->setEnabled(false);
		ui->erase->setEnabled(false);
	}

}


void EntryVersionsDialog::on_restore_clicked(){

	QModelIndex index = ui->treeView->currentIndex();
	if (!index.isValid())
		return;

	QKdbxDatabase::Entry entry = entryVersions->entry();
	const Kdbx::Database::Version* v = entry->version(index.row());
	if (v == entry->latest())
		return;

	Kdbx::Database::Version::Ptr version(new Kdbx::Database::Version(*v));
	size_t idx = entry.versions();
	entry.addVersion(std::move(version), idx);
	ui->treeView->setCurrentIndex(entryVersions->index(idx, 0));

}

void EntryVersionsDialog::on_erase_clicked(){
	QModelIndex index = ui->treeView->currentIndex();
	if (!index.isValid())
		return;

	if (entryVersions->entry().versions() < 2){
		QMessageBox::critical(this, tr("Erase a version."), tr("Cannot erase last version."));
		return;
	}

	if (QMessageBox::question(this, tr("Erase a version."), tr("Are you sure?"),
									QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
		return;

	entryVersions->entry().removeVersion(index.row());
}
