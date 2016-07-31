#include "grouppickerdialog.h"
#include "ui_grouppickerdialog.h"

#include <QIcon>

GroupPickerDialog::GroupPickerDialog(QKdbxDatabase* db, QKdbxDatabase::Group current, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GroupPickerDialog),
	fdb(db),
	fallowNull(false)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);
	ui->groupView->setModel(fdb);
	for (int i=1; i<ui->groupView->header()->count(); ++i){
		ui->groupView->header()->hideSection(i);
	}
	ui->groupView->expandAll();
	setCurrent(current);

	connect(ui->groupView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &GroupPickerDialog::onCurrentChanged);
	connect(fdb, &QObject::destroyed, this, &GroupPickerDialog::disable);
}

GroupPickerDialog::GroupPickerDialog(QKdbxDatabase* db, QWidget *parent)
	:GroupPickerDialog(db, QKdbxDatabase::Group(), parent)
{}


GroupPickerDialog::~GroupPickerDialog()
{
	delete ui;
}

QKdbxDatabase::Group GroupPickerDialog::current(){
	if (fdb)
		return fdb->group(ui->groupView->currentIndex());

	return QKdbxDatabase::Group();
}

void GroupPickerDialog::disable(){
	fdb = nullptr;
	ui->groupView->setModel(nullptr);
	ui->groupView->setEnabled(false);
	ui->okButton->setEnabled(false);
}

void GroupPickerDialog::setCurrent(QKdbxDatabase::Group current){
	if (fdb && current){
		ui->groupView->setCurrentIndex(fdb->index(current, 0));
	}else{
		ui->groupView->setCurrentIndex(QModelIndex());
	}
}

void GroupPickerDialog::setAllowNull(bool allow){
	fallowNull = allow;
	nullCheck();
}

void GroupPickerDialog::onCurrentChanged(){
	nullCheck();
}

void GroupPickerDialog::on_GroupPickerDialog_accepted(){
	emit groupAccepted(current());
}

void GroupPickerDialog::on_okButton_clicked(){
	if (!fdb || (!fallowNull && !ui->groupView->currentIndex().isValid()))
		return;
	accept();
}

void GroupPickerDialog::nullCheck(){
	bool disable = !fdb || (!fallowNull && !ui->groupView->currentIndex().isValid());
	ui->okButton->setDisabled(disable);
}

//------------------------------------------------------------------------------

GroupButton::GroupButton(QKdbxDatabase* db, QKdbxDatabase::Group current, QWidget* parent)
	:QToolButton(parent),
	   fdb(db),
	   fcurrent(current),
	   fallowNull(false)
{
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	setCurrent(current);

	if (fdb){
		connect(fdb, &QKdbxDatabase::rowsAboutToBeRemoved, this, &GroupButton::onGroupRemoved);
		connect(fdb, &QObject::destroyed, this, &GroupButton::disable);
	}
	connect(this, &GroupButton::clicked, this, &GroupButton::onClicked);
}

GroupButton::GroupButton(QKdbxDatabase* db, QWidget* parent)
	:GroupButton(db, QKdbxDatabase::Group(), parent)
{}

GroupButton::GroupButton(QWidget* parent)
	:GroupButton(nullptr, parent)
{}


void GroupButton::disable(){
	setDatabase(nullptr);
}

void GroupButton::setCurrent(QKdbxDatabase::Group current){
	if (current){
		if (fdb != current.model()){
			setDatabase(current.model());
		}

		QModelIndex idx = fdb->index(current, 0);
		setIcon(fdb->data(idx, Qt::DecorationRole).value<QIcon>());
		setText(fdb->data(idx, Qt::DisplayRole).toString());
	}else{
		setIcon(QIcon());
		setText(QString());
	}
	fcurrent = current;
}

void GroupButton::setDatabase(QKdbxDatabase* db){
	if (fdb == db)
		return;

	if (fdb){
		disconnect(fdb, &QKdbxDatabase::rowsAboutToBeRemoved, this, &GroupButton::onGroupRemoved);
		disconnect(fdb, &QObject::destroyed, this, &GroupButton::disable);
	}
	fdb = db;
	if (fdb){
		connect(fdb, &QKdbxDatabase::rowsAboutToBeRemoved, this, &GroupButton::onGroupRemoved);
		connect(fdb, &QObject::destroyed, this, &GroupButton::disable);
	}

	fcurrent = QKdbxDatabase::Group();
}


void GroupButton::setAllowNull(bool allow){
	fallowNull = allow;
}

void GroupButton::onClicked(){
	if (!fdb)
		return;

	GroupPickerDialog* dlg = new GroupPickerDialog(fdb, fcurrent, this);
	dlg->setAllowNull(fallowNull);
	connect(dlg, &GroupPickerDialog::groupAccepted, this, &GroupButton::setCurrent);
	dlg->show();
}


void GroupButton::onGroupRemoved(const QModelIndex& parentidx, int first, int last){
	if (!fdb)
		return;

	QKdbxDatabase::Group parent = fdb->group(parentidx);
	if (!parent)
		return;

	for (; first <= last; ++first){
		if (fcurrent->ancestor(parent.group(first))){
			setCurrent(QKdbxDatabase::Group());
			return;
		}
	}
}





