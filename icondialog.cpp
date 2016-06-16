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
#include "icondialog.h"
#include "ui_icondialog.h"

#include "qkdbxdatabase.h"

#include <QAbstractItemDelegate>
#include <QAbstractListModel>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

class StandardIconModel: public QAbstractListModel{
public:
	using QAbstractListModel::QAbstractListModel;

	int rowCount(const QModelIndex &) const override{
		return 69;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DecorationRole) const{
		if (role == Qt::DecorationRole && index.row() < 69){
			return QKdbxDatabase::Icons::standardIcon(Kdbx::StandardIcon(index.row()));
		}
		return QVariant();
	}
};

IconDialog::IconDialog(QKdbxDatabase* database, const Kdbx::Icon& selectedIcon, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::IconDialog),
	fdatabase(database),
	fileDialog(0)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	StandardIconModel* standardIconModel = new StandardIconModel(this);
	ui->standardIconView->setModel(standardIconModel);
	ui->databaseIconView->setModel(database->icons());
	switch (selectedIcon.type()){
	case Kdbx::Icon::Type::Standard:
		ui->standardIconView->setCurrentIndex(standardIconModel->index(int(selectedIcon.standard())));
	case Kdbx::Icon::Type::Null:
		ui->tabWidget->setCurrentIndex(0);
		break;
	case Kdbx::Icon::Type::Custom:
		ui->tabWidget->setCurrentIndex(1);
		ui->databaseIconView->setCurrentIndex(database->icons()->modelIndex(selectedIcon.custom()->uuid()));
		break;
	}
}

IconDialog::~IconDialog(){
	delete ui;
}

void IconDialog::on_pushButton_2_clicked(){
	if (!fileDialog){
		fileDialog = new QFileDialog(this);
		connect(fileDialog, &QFileDialog::accepted, this, &IconDialog::saveIconActivated);
	}

	fileDialog->setWindowTitle(tr("Save icon.."));
	fileDialog->setModal(true);
	fileDialog->setNameFilter("PNG image files (*.png)");
	fileDialog->setAcceptMode(QFileDialog::AcceptSave);
	fileDialog->show();
}

void IconDialog::on_pushButton_clicked(){
	if (!fileDialog){
		fileDialog = new QFileDialog(this);
		connect(fileDialog, &QFileDialog::accepted, this, &IconDialog::saveIconActivated);
	}

	fileDialog->setWindowTitle(tr("Load icon.."));
	fileDialog->setModal(true);
	fileDialog->setNameFilter("PNG image files (*.png)");
	fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog->show();
}

void IconDialog::saveIconActivated(){
	if (fileDialog->selectedFiles().size() < 1)
		return;

	if (fileDialog->acceptMode() == QFileDialog::AcceptSave){

		QModelIndex index = ui->databaseIconView->currentIndex();
		if (!index.isValid() || index.row() >= fdatabase->icons()->size() )
			return;

		QIcon icon = fdatabase->icons()->icon(index.row());
		QPixmap p = icon.pixmap(icon.actualSize(QSize(64,64)));
		if (p.save(fileDialog->selectedFiles().first(), "PNG") == false){
			QMessageBox::critical(this, tr("Error saving icon file."), tr("Failed saving icon file."));
		}
	}else{
		QPixmap p;
		if (!p.load(fileDialog->selectedFiles().first())){
			QMessageBox::critical(this, tr("Error opening icon file."), tr("Failed opening icon file."));
			return;
		}

		fdatabase->icons()->insert(QIcon(p));
	}
}



void IconDialog::on_IconDialog_accepted(){
	if (ui->tabWidget->currentIndex() == 0){
		QModelIndex index = ui->standardIconView->currentIndex();
		if (index.isValid())
			emit iconAccepted(Kdbx::Icon(Kdbx::StandardIcon(index.row())));
	}else{
		QModelIndex index = ui->databaseIconView->currentIndex();
		if (index.isValid()){
			emit iconAccepted(Kdbx::Icon(fdatabase->get()->customIcon(index.row())));
		}
	}
}
