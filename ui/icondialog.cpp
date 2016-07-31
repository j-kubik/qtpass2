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
	ui(new Ui::IconDialog)
{
	setAttribute(Qt::WA_DeleteOnClose);
	ui->setupUi(this);

	ui->iconEditor->setDatabase(database);
	ui->iconEditor->setCurrent(selectedIcon);
	connect(database, &QObject::destroyed, this, &IconDialog::disable);
}

IconDialog::~IconDialog(){
	delete ui;
}

void IconDialog::on_iconEditor_currentIconChanged(Kdbx::Icon current){
	ui->okButton->setDisabled(current.type() == Kdbx::Icon::Type::Null);
}

void IconDialog::on_okButton_clicked(){
	Kdbx::Icon icon = ui->iconEditor->current();
	if (icon.type() == Kdbx::Icon::Type::Null)
		return;

	emit iconAccepted(icon);
	accept();
}

void IconDialog::disable(){
	setEnabled(false);
}

