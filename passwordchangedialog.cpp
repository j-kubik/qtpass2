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
#include "passwordchangedialog.h"
#include "ui_passwordchangedialog.h"

#include "passwordgenerator.h"

#include <QMessageBox>

PasswordChangeDialog::PasswordChangeDialog(QString current, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::PasswordChangeDialog)
{
	ui->setupUi(this);
	ui->password1->setText(current);
}

PasswordChangeDialog::~PasswordChangeDialog(){
	delete ui;
}

void PasswordChangeDialog::passwordAccepted(){
	if (!checkPassword2()){
		QMessageBox::critical(this, tr("Error"), tr("Given passwords don't match."));
		return;
	}

	emit passwordChanged(ui->password1->text());
	accept();
}

bool PasswordChangeDialog::checkPassword2(){
	if (!ui->show->isChecked()){
		ui->password2->setPalette(QPalette());
		return true;
	}

	if (ui->password1->text() != ui->password2->text()){
		QPalette p;
		p.setColor(QPalette::Base, QColor(Qt::red));
		ui->password2->setPalette(p);
		return false;
	}

	ui->password2->setPalette(QPalette());
	return true;
}

void PasswordChangeDialog::onPasswordGenrated(QString pass){
	ui->password1->setText(pass);
	if (ui->show->isChecked()){
		ui->password2->setText(pass);
		ui->password2->setPalette(QPalette());
	}
}


void PasswordChangeDialog::on_show_toggled(bool checked){
	if (checked){
		ui->password1->setEchoMode(QLineEdit::Normal);
		ui->password2->setEnabled(true);
		ui->password2->setText(ui->password1->text());
	}else{
		ui->password1->setEchoMode(QLineEdit::Password);
		ui->password2->setEnabled(false);
		ui->password2->clear();
	}
	checkPassword2();

}

void PasswordChangeDialog::on_generate_clicked(){
	PasswordGenerator* gen = new PasswordGenerator(this);
	gen->setAttribute( Qt::WA_DeleteOnClose, true );
	gen->setModal(true);
	connect(gen, &PasswordGenerator::generated, this, &PasswordChangeDialog::onPasswordGenrated);
	gen->show();
}
