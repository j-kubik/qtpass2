#include "qtpasswindow.h"
#include "ui_qtpasswindow.h"

#include "opendialog.h"
#include "qkdbxview.h"

#include "passwordgenerator.h"

#include <libkeepass2pp/database.h>
#include <libkeepass2pp/compositekey.h>

#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>

#include <fstream>

#include "qkdbxdatabase.h"

static const char* qtPassGeometry = "QtPassGeometry";
//static const char* databaseStyles = "DatabaseStyles";

QtPassWindow::QtPassWindow(QWidget *parent) :
	QMainWindow(parent),
	callbacks(CallbackSite::create()),
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

	QStringList args = QCoreApplication::arguments();

	QString key;
	QString password;

	for (int i=1; i<args.size(); i++){
		if (args.at(i) == ("--key")){
			i++;
			if (i >= args.size())
				break;
			key = args.at(i);
			continue;
		}
		if (args.at(i) == ("--password")){
			i++;
			if (i >= args.size())
				break;
			password = args.at(i);
			continue;
		}

		std::thread(&openArgsFile, callbacks.toWeakRef(), this, password, key, args.at(i)).detach();
	}
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
		connect(openDbDialog, SIGNAL(accepted()), this, SLOT(onOpenDb_accepted()));
	}

	openDbDialog->show();
}

void QtPassWindow::onOpenDb_accepted(){
	if (!openDbDialog) return;
	QStringList files = openDbDialog->selectedFiles();
	if (!files.size()) return;

	std::thread(&doOpenAsync, callbacks.toWeakRef(), this, files.at(0)).detach();
}

void QtPassWindow::openArgsFile(CallbackSite::WeakPtr callbacks, QtPassWindow* ths, QString password, QString keyFilePath, QString filename){
	try{
		QByteArray tmpBuffer = filename.toUtf8();
		std::unique_ptr<std::ifstream> file(new std::ifstream());
		file->exceptions ( std::istream::failbit | std::istream::badbit | std::istream::eofbit );
		file->open(std::string(tmpBuffer.data(), tmpBuffer.size()));

		Kdbx::Database::File dbFile = Kdbx::Database::loadFromFile(std::move(file));

		std::future<Kdbx::Database::Ptr> database;
		if (dbFile.needsKey()){
			Kdbx::CompositeKey key;
			if (!password.isEmpty()){
				QByteArray tmpBuffer = password.toUtf8();
				key.addKey(Kdbx::CompositeKey::Key::fromPassword(std::string(tmpBuffer.data(), tmpBuffer.size())));
			}
			if (!keyFilePath.isEmpty()){
				QByteArray tmpBuffer = keyFilePath.toUtf8();
				key.addKey(Kdbx::CompositeKey::Key::fromFile(std::string(tmpBuffer.data(), tmpBuffer.size())));
			}

			database = dbFile.getDatabase(key);
		}else{
			database = dbFile.getDatabase();
		}

		auto newDb = [ths, &dbFile, db = database.get()](std::promise<void> promise) mutable ->void{
			QKdbxView* view = new QKdbxView(std::make_unique<QKdbxDatabase>(std::move(db), dbFile.settings), ths);
			ths->addWindow(view);
			promise.set_value();
		};

		CallbackSite::Ptr cs = callbacks.toStrongRef();
		if (!cs)
			return;
		cs->callback<void>(std::move(newDb)).get();


	}catch(std::exception& e){
		CallbackSite::Ptr cs = callbacks.toStrongRef();
		if (!cs)
			return;

		QString msg =QString::fromUtf8(e.what());
		auto errMsg = [ths, filename, msg](std::promise<void> promise)->void{
							QMessageBox::critical(ths, QObject::tr("Error opening database."),
												  QObject::tr("Error opening file %1: %2").arg(filename).arg(msg));
							promise.set_value();
						};

		cs->callback<void>(errMsg).get();
	}
}


void QtPassWindow::doOpenAsync(CallbackSite::WeakPtr callbacks, QtPassWindow* ths, QString filename){
	try{
	QByteArray tmpBuffer = filename.toUtf8();
	std::unique_ptr<std::ifstream> file(new std::ifstream());
	file->exceptions ( std::istream::failbit | std::istream::badbit | std::istream::eofbit );
	file->open(std::string(tmpBuffer.data(), tmpBuffer.size()));

	Kdbx::Database::File dbFile = Kdbx::Database::loadFromFile(std::move(file));

	std::future<Kdbx::Database::Ptr> database;

	if (dbFile.needsKey()){
		CallbackSite::Ptr cs = callbacks.toStrongRef();
		if (!cs)
			return;

		auto callOpenDialog	= [ths, filename](std::promise<Kdbx::CompositeKey> promise)->void{
					OpenDialog* dlg = new OpenDialog(std::move(promise), ths);
					dlg->show();
				};

		std::future<Kdbx::CompositeKey> key = cs->callback<Kdbx::CompositeKey>(callOpenDialog);
		cs.clear();

		database = dbFile.getDatabase(key.get());
	}else{
		database = dbFile.getDatabase();
	}

	auto newDb = [ths, &dbFile, db=database.get()](std::promise<void> promise) mutable ->void{
		QKdbxView* view = new QKdbxView(std::make_unique<QKdbxDatabase>(std::move(db), dbFile.settings), ths);
		ths->addWindow(view);
		promise.set_value();
	};

	CallbackSite::Ptr cs = callbacks.toStrongRef();
	if (!cs)
		return;

	cs->callback<void>(newDb).get();

	}catch(std::exception& e){
		CallbackSite::Ptr cs = callbacks.toStrongRef();
		if (!cs)
			return;

		QString msg =QString::fromUtf8(e.what());
		auto errMsg = [ths, filename, msg](std::promise<void> promise)->void{
							QMessageBox::critical(ths, QObject::tr("Error opening database."),
												  QObject::tr("Error opening file %1: %2").arg(filename).arg(msg));
							promise.set_value();
						};

		cs->callback<void>(errMsg).get();
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
}

void QtPassWindow::on_tabWidget_currentChanged(int index){
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		undoGroup->setActiveStack(w->undoStack());
		ui->actionAddEntry->setEnabled(true);
		ui->actionAddGroup->setEnabled(true);
		ui->actionDatabaseSettings->setEnabled(true);
	}else{
		undoGroup->setActiveStack(nullptr);
		ui->actionAddEntry->setEnabled(false);
		ui->actionAddGroup->setEnabled(false);
		ui->actionDatabaseSettings->setEnabled(false);
	}
}

void QtPassWindow::on_actionAddEntry_triggered(){
	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		w->actionActivated(DatabaseViewWidget::NewEntry);
	}
}

void QtPassWindow::on_actionAddGroup_triggered(){
	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		w->actionActivated(DatabaseViewWidget::NewGroup);
	}
}

void QtPassWindow::on_actionDatabaseSettings_triggered(){
	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		w->actionActivated(DatabaseViewWidget::Settings);
	}
}

void QtPassWindow::on_actionExit_triggered(){
	close();
}
