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
#include "databaseview.h"
#include "ui_databaseview.h"

#include "qkdbxdatabase.h"
#include "qkdbxgroup.h"

#include "entryeditdialog.h"
#include "entryversionsdialog.h"

#include <QSortFilterProxyModel>
#include <QMessageBox>

//------------------------------------------------------------------------------

DatabaseView::DatabaseView(std::unique_ptr<QKdbxDatabase> db, QWidget *parent) :
	DatabaseViewWidget(parent),
	ui(new Ui::DatabaseView),
	database(std::move(db)),
	fgroup(new QKdbxGroup(database.get(), this))
{
	ui->setupUi(this);
	ui->splitter->setStretchFactor(0, 1);
	ui->splitter->setStretchFactor(1, 2);

	ui->groupView->viewport()->setAcceptDrops(true);
	ui->groupView->setModel(database.get());
	ui->groupView->setExpanded(database->root(), true);
	//ui->groupView->setRootIndex(database->root());

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
		connect(action, &QAction::triggered, this, &DatabaseView::headerContextMenuColumnVisibility);
		header->addAction(action);
		headerActions.push_back(action);

//		QAction* action = new QAction(groupHeaderPopup);
//		action->setText(data.toString());
//		action->setData(i);
//		action->setCheckable(true);
//		groupHeaderPopup->popupActions.append(action);
//		groupHeaderPopup->addAction(action);
	}

	action = new QAction(this);
	action->setSeparator(true);
	header->addAction(action);
	action = new QAction(tr("Don't sort"), this);
	connect(action, &QAction::triggered, this, &DatabaseView::headerContextMenuDontSort);
	header->addAction(action);

	//groupHeaderPopup->addSeparator();
	//groupHeaderPopup->removeSorting = groupHeaderPopup->addAction("Don't sort");

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
	connect(ui->groupView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DatabaseView::currentGroupChanged);

	currentEntryChanged(ui->entriesView->selectionModel()->currentIndex());
	connect(ui->entriesView->selectionModel(), &QItemSelectionModel::currentChanged, this, &DatabaseView::currentEntryChanged);

	//connect(groupHeaderPopup, &HeaderPopup::triggered, this, &DatabaseView::headerContextMenuActionTriggered);
	//connect(header, &QHeaderView::customContextMenuRequested, this, &DatabaseView::headerContextMenuRequested);

	connect(header, &QHeaderView::sectionClicked, this, &DatabaseView::headerSectionClicked);
}

DatabaseView::~DatabaseView()
{
	delete ui;
}

QIcon DatabaseView::icon() const{
	return database->icons()->icon(Kdbx::Icon(Kdbx::StandardIcon::Key));
}

QString DatabaseView::name() const{
	const std::string& name = database->get()->settings().databaseName;
	if (name.size()){
		return QString::fromUtf8(name.c_str(), name.size());
	}
	return tr("Unnamed");
}

QString DatabaseView::style() const{
	return "Keepass2";
}

static const char* splitterGeometry = "SplitterGeometry";
static const char* groupViewGeometry = "groupViewGeometry";
static const char* entryViewGeometry = "EntryViewGeometry";

void DatabaseView::saveSettings(QSettings& s) const{
	s.setValue(splitterGeometry, ui->splitter->saveState());
	s.setValue(groupViewGeometry, ui->groupView->header()->saveState());
	s.setValue(entryViewGeometry, ui->entriesView->header()->saveState());
}

void DatabaseView::applySettings(QSettings& s){
	QVariant tmp = s.value(splitterGeometry);
	if (tmp.canConvert<QByteArray>()){
		ui->splitter->restoreState(tmp.toByteArray());
	}
	tmp = s.value(groupViewGeometry);
	if (tmp.canConvert<QByteArray>()){
		ui->groupView->header()->restoreState(tmp.toByteArray());
	}
	tmp = s.value(entryViewGeometry);

	QHeaderView* header = ui->entriesView->header();
	if (tmp.canConvert<QByteArray>()){
		ui->entriesView->header()->restoreState(tmp.toByteArray());
		for (int i=0; i<headerActions.size(); i++){
			headerActions[i]->setChecked(!header->isSectionHidden(i));
		}
	}

	if (header->isSortIndicatorShown()){
		ui->entriesView->setSortingEnabled(true);
		ui->entriesView->sortByColumn(header->sortIndicatorSection(), header->sortIndicatorOrder());
	}
}

QUndoStack* DatabaseView::undoStack(){
	return database->undoStack();
}

void DatabaseView::addEntryAction() {
	QModelIndex index = ui->groupView->currentIndex();
	if (!index.isValid()) return;

	QKdbxDatabase::Group group = database->group(index);

	EntryEditDialog* edit = new EntryEditDialog(group, this);
	edit->setAttribute( Qt::WA_DeleteOnClose, true );
	edit->show();
}


void DatabaseView::headerContextMenuColumnVisibility(){
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

void DatabaseView::headerContextMenuDontSort(){
	QHeaderView* header = ui->entriesView->header();
	header->setSortIndicatorShown(false);
	ui->entriesView->setSortingEnabled(false);
}

void DatabaseView::headerSectionClicked(int index){
	unused(index);
	QHeaderView* header = ui->entriesView->header();
	header->setSortIndicatorShown(true);
	ui->entriesView->setSortingEnabled(true);
	ui->entriesView->sortByColumn(header->sortIndicatorSection(), header->sortIndicatorOrder());
}

void DatabaseView::currentGroupChanged(const QModelIndex & current){
	bool enabled = current.isValid();
	QKdbxDatabase::Group group = database->group(current);
	fgroup->setGroup(group);
	if (enabled){
		enabled = bool(group.parent());
	}
	ui->actionDeleteGroup->setEnabled(enabled);
	ui->actionGroupProperties->setEnabled(enabled);
}

void DatabaseView::currentEntryChanged(const QModelIndex & current){
	bool enabled = current.isValid();
	ui->actionEntryDelete->setEnabled(enabled);
	ui->actionEntryVersions->setEnabled(enabled);
	ui->actionEntryEdit->setEnabled(enabled);
}


void DatabaseView::on_entriesView_doubleClicked(const QModelIndex &index)
{
	Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry = fgroup->entry(index);
	if (!entry || !entry->versions()) return;

	EntryEditDialog* edit = new EntryEditDialog(entry.latest(), this);
	edit->setAttribute( Qt::WA_DeleteOnClose, true );
	edit->show();
}

void DatabaseView::on_actionDeleteGroup_triggered()
{
	QModelIndex index = ui->groupView->currentIndex();
	if (!index.isValid()) return;

	QKdbxDatabase::Group group = database->group(index);
	if (!group.parent())
		return;
	group.remove();
}

void DatabaseView::on_actionGroupProperties_triggered(){
	QMessageBox::warning(this, "Not implemented yet.", "Sorry, not implemented yet.");
}

void DatabaseView::on_actionEntryEdit_triggered()
{
	QModelIndex idx = ui->entriesView->currentIndex();
	if (!idx.isValid())
		return;

	Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry = fgroup->entry(idx);
	if (!entry)
		return;

	assert(entry->versions() > 0);

	EntryEditDialog* edit = new EntryEditDialog(entry.latest(), this);
	edit->setAttribute( Qt::WA_DeleteOnClose, true );
	edit->show();
}

void DatabaseView::on_actionEntryVersions_triggered()
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

void DatabaseView::on_actionEntryDelete_triggered()
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

void DatabaseView::on_actionEntryNew_triggered(){

	if (!fgroup->group())
		return;



}
