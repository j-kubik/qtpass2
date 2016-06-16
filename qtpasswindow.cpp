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
#include "qtpasswindow.h"
#include "ui_qtpasswindow.h"

#include "opendialog.h"
#include "databaseview.h"

#include "passwordgenerator.h"

#include <libkeepass2pp/database.h>
#include <libkeepass2pp/compositekey.h>

#include <QSettings>

#include <QMessageBox>

#include <fstream>

#include "qkdbxdatabase.h"

static const char* qtPassGeometry = "QtPassGeometry";
static const char* databaseStyles = "DatabaseStyles";

QtPassWindow::QtPassWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::QtPassWindow),
	openDbDialog(0),
	openDialog(0),
	undoGroup(new QUndoGroup(this))
{

	ui->setupUi(this);
	ui->menuEdit->addAction(undoGroup->createUndoAction(this));
	ui->menuEdit->addAction(undoGroup->createRedoAction(this));
	Kdbx::Database::init();

	QSettings settings(this);

	QVariant v = settings.value(qtPassGeometry);
	if (v.canConvert<QByteArray>()){
		restoreGeometry(v.toByteArray());
	}

	//setWindowIcon(QKdbxDatabase::standardIcon(KdbxDatabase::Icon::Key));
}

QtPassWindow::~QtPassWindow()
{
	delete ui;
}

void QtPassWindow::on_actionOpen_triggered()
{
	if (!openDbDialog){
		openDbDialog = new QFileDialog(this);
		openDbDialog->setAcceptMode(QFileDialog::AcceptOpen);
		openDbDialog->setFileMode(QFileDialog::ExistingFile);
		openDbDialog->setModal(true);
		connect(openDbDialog, &QFileDialog::accepted, this, &QtPassWindow::onOpenDb_accepted);
	}

	openDbDialog->show();
}

void QtPassWindow::onOpenDb_accepted(){
	if (!openDbDialog) return;
	QStringList files = openDbDialog->selectedFiles();
	if (!files.size()) return;
	openDbFilename = files.at(0);

	if (!openDialog){
		openDialog = new OpenDialog(this);
		connect(openDialog, &OpenDialog::accepted, this, &QtPassWindow::onOpen_accepted);
	}
	openDialog->setOpenDbName(openDbFilename);
	openDialog->show();
}

void QtPassWindow::onOpen_accepted(){
	if (!openDialog) return;

	QByteArray tmpBuffer;

	try{

		Kdbx::CompositeKey compositeKey;
		QString passwd = openDialog->password();
		if (passwd.size()){
			tmpBuffer = passwd.toUtf8();
			compositeKey.addKey(Kdbx::CompositeKey::Key::fromPassword(std::string(tmpBuffer.data(),tmpBuffer.size())));
		}
		QString keyFile = openDialog->keyFile();
		if (keyFile.size()){
			tmpBuffer = keyFile.toUtf8();
			compositeKey.addKey(Kdbx::CompositeKey::Key::fromFile(std::string(tmpBuffer.data(),tmpBuffer.size())));
		}

		tmpBuffer = openDbFilename.toUtf8();
		std::unique_ptr<std::ifstream> file(new std::ifstream());
		file->exceptions ( std::istream::failbit | std::istream::badbit | std::istream::eofbit );
		file->open(std::string(tmpBuffer.data(), tmpBuffer.size()));

		Kdbx::Database::File f = Kdbx::Database::loadFromFile(std::move(file));
		std::unique_ptr<QKdbxDatabase> db(new QKdbxDatabase(f.getDatabase(compositeKey).get(), this));

		addWindow(new DatabaseView(std::move(db), this));

	}catch (std::exception& e){
		QMessageBox::critical(this, "Error opening database file.", QString::fromUtf8(e.what()));
	}

}

void QtPassWindow::on_actionGenerate_password_triggered(){
   PasswordGenerator* pgen = new PasswordGenerator(this);
   pgen->setModal(true);
   pgen->show();
}

void QtPassWindow::closeEvent(QCloseEvent * event){
	QSettings settings(this);
	settings.setValue(qtPassGeometry, saveGeometry());
	ui->tabWidget->widget(0);
	QMainWindow::closeEvent(event);
}

void QtPassWindow::addWindow(DatabaseViewWidget* widget){
	ui->tabWidget->addTab(widget, widget->icon(), widget->name());
	QSettings s;
	s.beginGroup(databaseStyles);
	s.beginGroup(widget->style());
	widget->applySettings(s);
	//s.endGroup();
	//s.endGroup();
}

//void QtPassWindow::on_actionDsAsDefault_triggered(){
//  DatabaseViewWidget* widget = static_cast<DatabaseViewWidget*>(ui->tabWidget->currentWidget());
//  if (!widget)
//	  return;
//  QSettings s;
//  s.beginGroup(databaseStyles);
//  s.beginGroup(widget->style());
//  widget->saveSettings(s);
//}

void QtPassWindow::on_tabWidget_currentChanged(int index){
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		undoGroup->setActiveStack(w->undoStack());
	}else{
		undoGroup->setActiveStack(nullptr);
	}
}

void QtPassWindow::on_actionNewKdbx_triggered(){
	std::unique_ptr<QKdbxDatabase> db(new QKdbxDatabase(Kdbx::Database::Ptr(new Kdbx::Database()), this));
	addWindow(new DatabaseView(std::move(db), this));
}

void QtPassWindow::on_actionNewEntry_triggered(){
	DatabaseViewWidget* widget = static_cast<DatabaseViewWidget*>(ui->tabWidget->currentWidget());
	if (!widget)
		return;
	widget->addEntryAction();
}
