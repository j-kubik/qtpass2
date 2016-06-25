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
#include "passwordgenerator.h"
#include "ui_passwordgenerator.h"

#include <QMessageBox>

PasswordGenerator::PasswordGenerator(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::PasswordGenerator)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
}

PasswordGenerator::~PasswordGenerator(){
	delete ui;
}

void PasswordGenerator::generate(){
	try{
		QString pass = ui->passwordOptions->generate();
		ui->currentPassword->setText(pass);
	}catch(std::exception& e){
		QMessageBox::critical(this, tr("Error"), QString::fromUtf8(e.what()));
	}
}

void PasswordGenerator::passwordAccepted(){

	QString pass = ui->currentPassword->text();
	try{
		while (pass.size() == 0){
			pass = ui->passwordOptions->generate();
		}
	}catch(std::exception& e){
		QMessageBox::critical(this, tr("Error"), QString::fromUtf8(e.what()));
	}

	emit generated(pass);
	accept();
}



