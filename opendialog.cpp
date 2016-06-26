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
#include "opendialog.h"
#include "ui_opendialog.h"

#include <QFileDialog>

OpenDialog::OpenDialog(std::promise<Kdbx::CompositeKey> compositePromise,
					   QWidget *parent) :
	QDialog(parent),
	ui(new Ui::OpenDialog),
	dialog(0),
	compositePromise(std::move(compositePromise))
{
	ui->setupUi(this);
	setModal(true);
}

OpenDialog::~OpenDialog()
{
	delete ui;
}

void OpenDialog::setOpenDbName(const QString& name){
	ui->databaseLabel->setText(name);
}

QString OpenDialog::password(){
	return ui->passwordEdit->text();
}

QString OpenDialog::keyFile(){
	return ui->keyEdit->text();
}

void OpenDialog::on_passwordBox_toggled(bool checked)
{
	if (!checked)
		ui->passwordEdit->setText(QString());
}

void OpenDialog::on_passwordEdit_textChanged(const QString &arg1)
{
	ui->passwordBox->setChecked(arg1.length());
}

void OpenDialog::on_keyEdit_textChanged(const QString &arg1)
{
	ui->keyBox->setChecked(arg1.length());
}

void OpenDialog::on_keyBox_toggled(bool checked)
{
	if (!checked)
		ui->keyEdit->setText(QString());
}

void OpenDialog::on_keyBrowseButton_clicked(){

	if (!dialog){
		dialog = new QFileDialog(this);
		dialog->setAcceptMode(QFileDialog::AcceptOpen);
		dialog->setFileMode(QFileDialog::ExistingFile);
		dialog->setModal(true);
		connect(dialog, &QFileDialog::accepted, this, &OpenDialog::onKeyBrowseDialogAccepted);
	}
	dialog->show();
}

void OpenDialog::onKeyBrowseDialogAccepted(){
	if (!dialog) return;
	QStringList files = dialog->selectedFiles();
	if (!files.size()) return;
	ui->keyEdit->setText(files.at(0));
}

void OpenDialog::on_OpenDialog_accepted(){
	Kdbx::CompositeKey compositeKey;
	QString passwd = password();
	if (passwd.size()){
		QByteArray tmpBuffer = passwd.toUtf8();
		compositeKey.addKey(Kdbx::CompositeKey::Key::fromPassword(SafeString<char>(tmpBuffer.data(),tmpBuffer.size())));
	}
	QString kFile = keyFile();
	if (kFile.size()){
		QByteArray tmpBuffer = kFile.toUtf8();
		compositeKey.addKey(Kdbx::CompositeKey::Key::fromFile(SafeString<char>(tmpBuffer.data(),tmpBuffer.size())));
	}

	compositePromise.set_value(std::move(compositeKey));
}
