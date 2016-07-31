#include "qtpasswindow.h"
#include "ui_qtpasswindow.h"

#include "ui/opendialog.h"
#include "ui/newdialog.h"
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
	executor(Executor::create(this)),
	ui(new Ui::QtPassWindow),
	openDbDialog(nullptr),
	openDialog(nullptr),
	newDialog(nullptr),
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

		std::thread(&openArgsFile, executor, this, password, key, args.at(i)).detach();
	}
}

QtPassWindow::~QtPassWindow()
{
	delete ui;
}

void QtPassWindow::updateActions(DatabaseViewWidget* w){
	if (!w){
		undoGroup->setActiveStack(nullptr);
		ui->actionAddEntry->setEnabled(false);
		ui->actionAddGroup->setEnabled(false);
		ui->actionDatabaseSettings->setEnabled(false);
		ui->actionSave->setEnabled(false);
		ui->actionSaveAs->setEnabled(false);
		ui->actionClose->setEnabled(false);
		ui->actionIcons->setEnabled(false);
	}else{
		undoGroup->setActiveStack(w->undoStack());
		DatabaseViewWidget::StandardBarActions actions = w->standardBarActions();
		ui->actionAddEntry->setEnabled(actions.testFlag(DatabaseViewWidget::NewEntry));
		ui->actionAddGroup->setEnabled(actions.testFlag(DatabaseViewWidget::NewGroup));
		ui->actionDatabaseSettings->setEnabled(actions.testFlag(DatabaseViewWidget::Settings));
		ui->actionSave->setEnabled(actions.testFlag(DatabaseViewWidget::Save));
		ui->actionSaveAs->setEnabled(actions.testFlag(DatabaseViewWidget::SaveAs));
		ui->actionClose->setEnabled(true);
		ui->actionIcons->setEnabled(actions.testFlag(DatabaseViewWidget::Icons));
	}
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
	assert(openDbDialog);

	QStringList files = openDbDialog->selectedFiles();
	if (!files.size()) return;

	std::thread(&doOpenAsync, executor, this, files.at(0)).detach();
}

void QtPassWindow::onNewDb_accepted(){
	assert(newDialog);

}


void QtPassWindow::tabActionsUpdated(){
	DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->currentWidget());
	if (w == sender()){
		updateActions(w);
	}
}

void QtPassWindow::openArgsFile(Executor::Ptr executor, QtPassWindow* ths, QString password, QString keyFilePath, QString filename){
	try{
		QByteArray tmpBuffer = filename.toUtf8();
		Kdbx::Database::File dbFile = Kdbx::Database::loadFromFile(std::string(tmpBuffer.data(), tmpBuffer.size()));

		std::future<Kdbx::Database::Ptr> database;
		if (dbFile.needsKey()){
			Kdbx::CompositeKey key;
			if (!password.isEmpty()){
				QByteArray tmpBuffer = password.toUtf8();
				key.addKey(Kdbx::CompositeKey::Key::fromPassword(Kdbx::SafeString<char>(tmpBuffer.data(), tmpBuffer.size())));
			}
			if (!keyFilePath.isEmpty()){
				QByteArray tmpBuffer = keyFilePath.toUtf8();
				key.addKey(Kdbx::CompositeKey::Key::fromFile(Kdbx::SafeString<char>(tmpBuffer.data(), tmpBuffer.size())));
			}

			database = dbFile.getDatabase(std::move(key));
		}else{
			database = dbFile.getDatabase();
		}

		auto newDb = [ths, &dbFile, filename, db = database.get()](std::promise<void> promise) mutable ->void{
			QKdbxView* view = new QKdbxView(std::move(db), filename, ths);
			ths->addWindow(view);
			promise.set_value();
		};

		executor->callback<void>(std::move(newDb)).get();


	}catch(std::exception& e){
		QString msg =QString::fromUtf8(e.what());
		auto errMsg = [ths, filename, msg](std::promise<void> promise)->void{
							QMessageBox::critical(ths, QObject::tr("Error opening database."),
												  QObject::tr("Error opening file %1: %2").arg(filename).arg(msg));
							promise.set_value();
						};

		executor->callback<void>(errMsg).get();
	}
}


void QtPassWindow::doOpenAsync(Executor::Ptr executor, QtPassWindow* ths, QString filename){
	try{
		QByteArray tmpBuffer = filename.toUtf8();
		Kdbx::Database::File dbFile = Kdbx::Database::loadFromFile(std::string(tmpBuffer.data(), tmpBuffer.size()));

		std::future<Kdbx::Database::Ptr> database;


		if (dbFile.needsKey()){
			auto callOpenDialog	= [ths, filename](std::promise<Kdbx::CompositeKey> promise)->void{
				OpenDialog* dlg = new OpenDialog(std::move(promise), ths);
				dlg->setOpenDbName(filename);
				dlg->show();
			};

			Kdbx::CompositeKey key = executor->callback<Kdbx::CompositeKey>(callOpenDialog).get();

			database = dbFile.getDatabase(std::move(key));
		}else{
			database = dbFile.getDatabase();
		}

		auto newDb = [ths, &dbFile, filename, db{database.get()}](std::promise<void> promise) mutable ->void{
			QKdbxView* view = new QKdbxView(std::move(db), filename, ths);
			ths->addWindow(view);
			promise.set_value();
		};

		executor->callback<void>(newDb).get();

	}catch(std::exception& e){

		QString msg =QString::fromUtf8(e.what());
		auto errMsg = [ths, filename, msg](std::promise<void> promise)->void{
			QMessageBox::critical(ths, QObject::tr("Error opening database."),
								  QObject::tr("Error opening file %1: %2").arg(filename).arg(msg));
			promise.set_value();
		};

		executor->callback<void>(errMsg).get();
	}
}

void QtPassWindow::closeEvent(QCloseEvent * event){

	while (ui->tabWidget->count())
		removeWindow(0);

	QSettings settings(this);
	settings.setValue(qtPassGeometry, saveGeometry());
	QMainWindow::closeEvent(event);
}

void QtPassWindow::on_actionGenerate_password_triggered(){
   PasswordGenerator* pgen = new PasswordGenerator(this);
   pgen->show();
}

void QtPassWindow::addWindow(DatabaseViewWidget* widget){
	connect(widget, &DatabaseViewWidget::actionsUpdated, this, &QtPassWindow::tabActionsUpdated);
	ui->tabWidget->addTab(widget, widget->icon(), widget->name());
}

void QtPassWindow::removeWindow(int index){
	DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
	disconnect(w, &DatabaseViewWidget::actionsUpdated, this, &QtPassWindow::tabActionsUpdated);
	ui->tabWidget->removeTab(index);
	w->deleteLater();
}

void QtPassWindow::on_tabWidget_currentChanged(int index){
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		updateActions(w);
	}else{
		updateActions(nullptr);
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

void QtPassWindow::on_actionSave_triggered(){

	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		w->actionActivated(DatabaseViewWidget::Save);
	}
}

void QtPassWindow::on_actionSaveAs_triggered()
{
	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		w->actionActivated(DatabaseViewWidget::SaveAs);
	}
}

void QtPassWindow::on_actionClose_triggered(){
	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		removeWindow(index);
	}
}

void QtPassWindow::on_tabWidget_tabCloseRequested(int index){
	if (index >= 0 && index < ui->tabWidget->count()){
		removeWindow(index);
	}
}

void QtPassWindow::on_actionIcons_triggered(){

	int index = ui->tabWidget->currentIndex();
	if (index >= 0 && index < ui->tabWidget->count()){
		DatabaseViewWidget* w = static_cast<DatabaseViewWidget*>(ui->tabWidget->widget(index));
		w->actionActivated(DatabaseViewWidget::Icons);
	}
}

void QtPassWindow::on_actionNew_triggered(){

	if (!newDialog){
		newDialog = new NewDialog(this);
		connect(newDialog, &NewDialog::accepted, this, &QtPassWindow::onNewDb_accepted);
	}

	newDialog->show();
}
