#include "databasesettings.h"
#include "ui_databasesettings.h"

#include "qkdbxdatabase.h"

#include <QTreeView>

class DatabaseSettings::MyUi: public Ui::DatabaseSettings{
public:

//	QTreeView* recycleBinComboView;
//	QTreeView* templatesComboView;

	QPersistentModelIndex templates;
	QPersistentModelIndex recycleBin;

//	void setupUi(QDialog *DatabaseSettings){
//		Ui::DatabaseSettings::setupUi(DatabaseSettings);
//	}



};

DatabaseSettings::DatabaseSettings(QKdbxDatabase* db, QWidget *parent) :
	QDialog(parent),
	fdb(nullptr),
	ui(new MyUi)
{
	ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	fromDatabase(db);
}

DatabaseSettings::~DatabaseSettings()
{
	delete ui;
}

void DatabaseSettings::fromDatabase(QKdbxDatabase* db){
	if (fdb){
		disconnect(fdb, &QKdbxDatabase::modelReset, this, &DatabaseSettings::drop);
		disconnect(fdb, &QObject::destroyed, this, &DatabaseSettings::drop);
	}
	fdb = db;
	connect(fdb, &QKdbxDatabase::modelReset, this, &DatabaseSettings::drop);
	connect(fdb, &QObject::destroyed, this, &DatabaseSettings::drop);

	const Kdbx::Database::Settings& settings = fdb->get()->settings();
	const Kdbx::Database::File::Settings& fileSettings = fdb->fileSettings();

	ui->nameEdit->setText(QString::fromUtf8(settings.name().c_str(), settings.name().size()));
	ui->usernameEdit->setText(QString::fromUtf8(settings.defaultUsername().c_str(), settings.defaultUsername().size()));
	ui->descriptionEdit->document()->setPlainText(QString::fromUtf8(settings.description().c_str(), settings.description().size()));

	if (fileSettings.compress && fileSettings.compression == Kdbx::Database::File::CompressionAlgorithm::GZip){
		ui->compressionCombo->setCurrentIndex(0);
	}else{
		ui->compressionCombo->setCurrentIndex(1);
	}

	if (fileSettings.encrypt){
		ui->encryptionCombo->setCurrentIndex(0);
	}else{
		ui->encryptionCombo->setCurrentIndex(1);
	}

	ui->roundsSpin->setValue(fileSettings.transformRounds);

	// Following: http://qt.shoutwiki.com/wiki/Implementing_QTreeView_in_QComboBox_using_Qt-_Part_2
	// Anyway, combobox behavior is terribly inconsistent and requires manual behavior
	// programming with QTreeView - I should come up with something better at some point.

	// ToDo: history maximum sizes - this needs implementing through the library...
	// ToDo: GUI shuld be adjusted so amaller size limit can also be added.

	ui->recycleBinBox->setChecked(settings.recycleBinEnabled);
	ui->recycleBinCombo->setEnabled(settings.recycleBinEnabled);
	ui->recycleBin = fdb->index(fdb->recycleBin(),0);
	ui->recycleBinCombo->setModel(fdb);
	ui->recycleBinCombo->setCurrentIndex(ui->recycleBin);
	ui->recycleBinCombo->view()->setExpanded(fdb->root(), true);

	ui->templates = fdb->index(fdb->templates(), 0);
	ui->templatesBox->setChecked(ui->templates.isValid());
	ui->templatesCombo->setEnabled(ui->templates.isValid());
	ui->templatesCombo->setModel(fdb);
	ui->templatesCombo->setCurrentIndex(ui->templates);
	ui->templatesCombo->view()->setExpanded(fdb->root(), true);

	ui->historyCountBox->setValue(settings.historyMaxItems);
	ui->historyDaysBox->setValue(settings.maintenanceHistoryDays);
	ui->historySizeBox->setValue(settings.historyMaxSize/(1024*1024));
	ui->titleBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Title));
	ui->usernameBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::UserName));
	ui->passwordBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Password));
	ui->urlBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Url));
	ui->notesBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Notes));
}

void DatabaseSettings::saveDatabase(){
	if (!fdb)
		return;


}


void DatabaseSettings::on_pushButton_clicked(){
	saveDatabase();
}

void DatabaseSettings::on_recycleBinCombo_currentIndexChanged(int){
	if (!fdb)
		return;

	//ui->recycleBin = ui->templatesCombo->view()->currentIndex();
}

void DatabaseSettings::on_templatesCombo_currentIndexChanged(int){
	if (!fdb)
		return;

	//ui->templates = ui->templatesCombo->view()->currentIndex();
}

void DatabaseSettings::drop(){
	fdb = nullptr;
	ui->tabWidget->setEnabled(false);
	ui->okButton->setEnabled(false);
}

