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
#include "qkdbxview.h"
#include "ui_qkdbxview.h"

#include "qkdbxdatabase.h"
#include "qkdbxgroup.h"

#include "ui/newentrydialog.h"
#include "ui/newgroupdialog.h"
#include "ui/groupeditordialog.h"
#include "ui/entryeditdialog.h"
#include "ui/entryversionsdialog.h"
#include "databasesettings.h"
#include "ui/partial/iconseditor.h"

#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QString>
#include <QToolBar>
#include <QFileDialog>

#include <libkeepass2pp/compositekey.h>

#include <chrono>
#include <thread>

//------------------------------------------------------------------------------

QKdbxView::QKdbxView(Kdbx::CompositeKey key, QWidget *parent)
	:QKdbxView(Kdbx::Database::Ptr(new Kdbx::Database(std::move(key))), QString(), parent)
{}

QKdbxView::QKdbxView(Kdbx::Database::Ptr db, QString filename, QWidget *parent) :
	DatabaseViewWidget(parent),
	ui(new Ui::QKdbxView),
	database(new QKdbxDatabase(std::move(db), this)),
	ffilename(filename),
	fgroup(new QKdbxGroup(database, this))
{
	ui->setupUi(this);
	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 2);

	ui->groupView->viewport()->setAcceptDrops(true);
	ui->groupView->setModel(database);
	ui->groupView->setExpanded(database->rootIndex(), true);

	QSortFilterProxyModel* sortModel = new QSortFilterProxyModel(this);
	sortModel->setSourceModel(fgroup);
	ui->entriesView->setModel(sortModel);
	QHeaderView* header = ui->entriesView->header();
	header->setContextMenuPolicy(Qt::ActionsContextMenu);

	header->setSectionsClickable(true);

	//groupHeaderPopup = new HeaderPopup(header);

	QAction* action;

	for (int i=0; i<header->count(); i++){
		QVariant data = fgroup->headerData(i, header->orientation(), Qt::DisplayRole);
		if (!data.canConvert<QString>())
			continue;

		action = new QAction(data.toString(), this);
		action->setData(i);
		action->setCheckable(true);
		action->setChecked(!header->isSectionHidden(i));
		connect(action, &QAction::triggered, this, &QKdbxView::headerContextMenuColumnVisibility);
		header->addAction(action);
		headerActions.push_back(action);
	}

	action = new QAction(this);
	action->setSeparator(true);
	header->addAction(action);
	action = new QAction(tr("Don't sort"), this);
	connect(action, &QAction::triggered, this, &QKdbxView::headerContextMenuDontSort);
	header->addAction(action);

	ui->groupView->addAction(ui->actionGroupProperties);
	action = new QAction(this);
	action->setSeparator(true);
	ui->groupView->addAction(action);
	ui->groupView->addAction(ui->actionDeleteGroup);

	ui->entriesView->addAction(ui->actionEntryEdit);
	ui->entriesView->addAction(ui->actionEntryVersions);
	ui->entriesView->addAction(action);
	ui->entriesView->addAction(ui->actionEntryDelete);

	currentGroupChanged(ui->groupView->selectionModel()->currentIndex());
	connect(ui->groupView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QKdbxView::currentGroupChanged);

	currentEntryChanged(ui->entriesView->selectionModel()->currentIndex());
	connect(ui->entriesView->selectionModel(), &QItemSelectionModel::currentChanged, this, &QKdbxView::currentEntryChanged);

	//connect(groupHeaderPopup, &HeaderPopup::triggered, this, &DatabaseView::headerContextMenuActionTriggered);
	//connect(header, &QHeaderView::customContextMenuRequested, this, &DatabaseView::headerContextMenuRequested);

	connect(header, &QHeaderView::sectionClicked, this, &QKdbxView::headerSectionClicked);

	connect(database, &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
	connect(database, &QKdbxDatabase::frozenChanged, this, &DatabaseViewWidget::actionsUpdated);
	connect(database->undoStack(), &QUndoStack::cleanChanged, this, &DatabaseViewWidget::actionsUpdated);
	connect(database, &QKdbxDatabase::saveError, this, &QKdbxView::onSaveError);

	database->undoStack()->setClean();
}

QKdbxView::~QKdbxView()
{
	delete ui;
}

QIcon QKdbxView::icon() const{
	return database->icons()->icon(Kdbx::Icon(Kdbx::StandardIcon::Key));
}

QString QKdbxView::name() const{
	const std::string& name = database->get()->settings().name();
	if (name.size()){
		return QString::fromUtf8(name.c_str(), name.size());
	}
	return tr("Unnamed");
}

QString QKdbxView::style() const{
	return "Keepass2";
}

QUndoStack* QKdbxView::undoStack(){
	return database->undoStack();
}

DatabaseViewWidget::StandardBarActions QKdbxView::standardBarActions(){
	if (database->frozen())
		return DatabaseViewWidget::StandardBarActions();

	StandardBarActions result = Settings | Icons | SaveAs;

	if (!undoStack()->isClean() && !ffilename.isEmpty())
		result |= Save;

	QKdbxDatabase::Group group = database->group(ui->groupView->currentIndex());
	if (group){
		result |= DatabaseViewWidget::NewEntry | DatabaseViewWidget::NewGroup;
	}
	return result;
}

void QKdbxView::actionActivated(StandardBarAction action){

	switch (action){
	case Save:
		database->saveAs(ffilename);
		break;
	case SaveAs:{
		QFileDialog* dlg = new QFileDialog(this);
		dlg->setAttribute(Qt::WA_DeleteOnClose);
		dlg->setAcceptMode(QFileDialog::AcceptSave);
		dlg->setFileMode(QFileDialog::AnyFile);
		dlg->selectFile(ffilename);
		connect(dlg, &QFileDialog::fileSelected, this, &QKdbxView::saveAs);
		dlg->show();
		}
		break;
	case NewGroup:{
		QKdbxDatabase::Group group = database->group(ui->groupView->currentIndex());
		if (!group)
			return;
		(new NewGroupDialog(group, this))->show();
		break;
	}
	case NewEntry:{
		QKdbxDatabase::Group group = database->group(ui->groupView->currentIndex());
		if (!group)
			return;
		(new NewEntryDialog(group, this))->show();
		break;
	}
	case Settings:
		(new DatabaseSettings(database, this))->show();
		break;
	case Icons:
		(new IconsEditorDialog(database, this))->show();
	}

}

void QKdbxView::saveAs(QString filename){
	ffilename = filename;
	database->saveAs(ffilename);
}

void QKdbxView::onSaveError(QString message){
	QMessageBox::critical(this, tr("Error saving file."), tr("Error saving file %1.\n%2").arg(ffilename).arg(message));
}

void QKdbxView::headerContextMenuColumnVisibility(){
	QHeaderView* header = ui->entriesView->header();

	QAction* action = qobject_cast<QAction*>(sender());
	if (!action) return;

	QVariant data = action->data();
	if (!data.canConvert<int>()) return;
	bool ok;
	int idx = data.toInt(&ok);
	if (!ok) return;

	header->setSectionHidden(idx, !action->isChecked());
}

void QKdbxView::headerContextMenuDontSort(){
	QHeaderView* header = ui->entriesView->header();
	header->setSortIndicatorShown(false);
	ui->entriesView->setSortingEnabled(false);
}

void QKdbxView::headerSectionClicked(int index){
	Kdbx::unused(index);
	QHeaderView* header = ui->entriesView->header();
	header->setSortIndicatorShown(true);
	ui->entriesView->setSortingEnabled(true);
	ui->entriesView->sortByColumn(header->sortIndicatorSection(), header->sortIndicatorOrder());
}

void QKdbxView::currentGroupChanged(const QModelIndex & current){
	bool enabled = current.isValid();
	QKdbxDatabase::Group group = database->group(current);
	fgroup->setGroup(group);
	if (enabled){
		enabled = bool(group.parent());
	}
	ui->actionDeleteGroup->setEnabled(enabled);
	ui->actionGroupProperties->setEnabled(enabled);
	emit actionsUpdated();
}

void QKdbxView::currentEntryChanged(const QModelIndex & current){
	bool enabled = current.isValid();
	ui->actionEntryDelete->setEnabled(enabled);
	ui->actionEntryVersions->setEnabled(enabled);
	ui->actionEntryEdit->setEnabled(enabled);
}

void QKdbxView::onFrozenChanged(bool frozen){
	setDisabled(frozen);
	emit actionsUpdated();
}


void QKdbxView::on_entriesView_doubleClicked(const QModelIndex &index)
{
	QKdbxDatabase::Entry entry = fgroup->entry(index);
	if (!entry || !entry->versions()) return;

	EntryEditDialog* edit = new EntryEditDialog(entry.latest(), this);
	edit->setAttribute( Qt::WA_DeleteOnClose, true );
	edit->show();
}

void QKdbxView::on_actionDeleteGroup_triggered()
{
	QModelIndex index = ui->groupView->currentIndex();
	if (!index.isValid()) return;

	QKdbxDatabase::Group group = database->group(index);
	if (!group.parent())
		return;
	group.remove();
}

void QKdbxView::on_actionGroupProperties_triggered(){
	QModelIndex index = ui->groupView->currentIndex();
	if (!index.isValid()) return;

	QKdbxDatabase::Group group = database->group(index);
	if (!group.parent())
		return;

	(new GroupEditorDialog(group, this))->show();

	//QMessageBox::warning(this, "Not implemented yet.", "Sorry, not implemented yet.");
}

void QKdbxView::on_actionEntryEdit_triggered()
{
	QModelIndex idx = ui->entriesView->currentIndex();
	if (!idx.isValid())
		return;

	QKdbxDatabase::Entry entry = fgroup->entry(idx);
	if (!entry)
		return;

	assert(entry->versions() > 0);

	EntryEditDialog* edit = new EntryEditDialog(entry.latest(), this);
	edit->setAttribute( Qt::WA_DeleteOnClose, true );
	edit->show();
}

void QKdbxView::on_actionEntryVersions_triggered()
{
	QModelIndex idx = ui->entriesView->currentIndex();
	if (!idx.isValid())
		return;

	QKdbxDatabase::Entry entry = fgroup->entry(idx);
	if (!entry)
		return;

	assert(entry->versions() > 0);

	EntryVersionsDialog* versions = new EntryVersionsDialog(entry, this);
	versions->setAttribute( Qt::WA_DeleteOnClose, true );
	versions->show();
}

void QKdbxView::on_actionEntryDelete_triggered()
{    //ToDo: entry title as message part?
	if (QMessageBox::question(this, tr("Erase an entry."), tr("Are you sure?"),
									QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
		return;

	QModelIndex idx = ui->entriesView->currentIndex();
	if (!idx.isValid())
		return;

	QKdbxDatabase::Entry entry = fgroup->entry(idx);
	if (!entry)
		return;

	entry.remove();
}

void QKdbxView::on_actionEntryNew_triggered(){

	if (!fgroup->group())
		return;



}
