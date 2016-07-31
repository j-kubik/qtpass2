#include "newdialog.h"
#include "ui_newdialog.h"

#include "passwordgenerator.h"

#include "qkdbxview.h"

#include <QFileDialog>

NewDialog::NewDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::NewDialog)
{
	ui->setupUi(this);
}

NewDialog::~NewDialog(){
	delete ui;
}

void NewDialog::on_passwordEdit_textChanged(const QString &arg1){
	ui->passwordBox->setChecked(!arg1.isEmpty());
}

void NewDialog::on_generateButton_clicked(){
	PasswordGenerator* generator = new PasswordGenerator(this);
	connect(generator, &PasswordGenerator::generated, ui->passwordEdit, &QLineEdit::setText);
	generator->show();
}

void NewDialog::on_passwordBox_toggled(bool checked){
	if (!checked)
		ui->passwordEdit->setText(QString());
}

void NewDialog::on_keyBrowseButton_clicked(){
	QFileDialog* dialog = new QFileDialog(this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setAcceptMode(QFileDialog::AcceptOpen);
	dialog->setFileMode(QFileDialog::ExistingFile);
	dialog->setModal(true);
	connect(dialog, &QFileDialog::fileSelected, ui->keyEdit, &QLineEdit::setText);
	dialog->show();
}

void NewDialog::on_createButton_clicked(){
	QFileDialog* dialog = new QFileDialog(this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setAcceptMode(QFileDialog::AcceptSave);
	dialog->setFileMode(QFileDialog::AnyFile);
	dialog->setModal(true);
	connect(dialog, &QFileDialog::fileSelected, this, &NewDialog::on_createAccepted);
	dialog->show();
}

void NewDialog::on_createAccepted(QString filename){
	QByteArray tmp = filename.toUtf8();
	Kdbx::CompositeKey::Key::createFile(Kdbx::SafeString<char>(tmp.data(), tmp.size()));
	ui->keyEdit->setText(filename);
}

