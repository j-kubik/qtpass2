#include "databasesettings.h"
#include "ui_databasesettings.h"

#include "qkdbxdatabase.h"
#include "utils.h"

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



private slots:


};

DatabaseSettings::DatabaseSettings(QKdbxDatabase* db, QWidget *parent) :
	QDialog(parent),
	fdb(nullptr),
	ui(new MyUi)
{
	ui->setupUi(this);
	ui->updateFrame->hide();
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
		disconnect(fdb, &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
		disconnect(fdb, &QKdbxDatabase::settingsChanged, ui->updateFrame, &QWidget::show);
	}
	fdb = db;
	connect(fdb, &QKdbxDatabase::modelReset, this, &DatabaseSettings::drop);
	connect(fdb, &QObject::destroyed, this, &DatabaseSettings::drop);
	connect(fdb, &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
	connect(fdb, &QKdbxDatabase::settingsChanged, ui->updateFrame, &QWidget::show);

	setDisabled(fdb->frozen());

	const Kdbx::Database::Settings& settings = fdb->get()->settings();

	ui->nameEdit->setText(QString::fromUtf8(settings.name().c_str(), settings.name().size()));
	ui->usernameEdit->setText(QString::fromUtf8(settings.defaultUsername().c_str(), settings.defaultUsername().size()));
	ui->descriptionEdit->document()->setPlainText(QString::fromUtf8(settings.description().c_str(), settings.description().size()));

	if (settings.fileSettings.compress && settings.fileSettings.compression == Kdbx::Database::File::CompressionAlgorithm::GZip){
		ui->compressionCombo->setCurrentIndex(0);
	}else{
		ui->compressionCombo->setCurrentIndex(1);
	}

	if (settings.fileSettings.encrypt){
		ui->encryptionCombo->setCurrentIndex(0);
	}else{
		ui->encryptionCombo->setCurrentIndex(1);
	}

	ui->roundsSpin->setValue(settings.fileSettings.transformRounds);

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
	ui->recycleBinCombo->view()->setExpanded(fdb->rootIndex(), true);

	ui->templates = fdb->index(fdb->templates(), 0);
	ui->templatesBox->setChecked(ui->templates.isValid());
	ui->templatesCombo->setEnabled(ui->templates.isValid());
	ui->templatesCombo->setModel(fdb);
	ui->templatesCombo->setCurrentIndex(ui->templates);
	ui->templatesCombo->view()->setExpanded(fdb->rootIndex(), true);

	ui->historyCountBox->setValue(settings.historyMaxItems);
	ui->historyDaysBox->setValue(settings.maintenanceHistoryDays);
	ui->historySizeBox->setValue(settings.historyMaxSize/(1024*1024));
	ui->titleBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Title));
	ui->usernameBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::UserName));
	ui->passwordBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Password));
	ui->urlBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Url));
	ui->notesBox->setChecked(settings.memoryProtection.test(Kdbx::MemoryProtection::Notes));
}

void DatabaseSettings::toDatabase(){
	if (!fdb || fdb->frozen())
		return;

	Kdbx::Database::Settings::Ptr settings(new Kdbx::Database::Settings(fdb->get()->settings()));

	settings->setName(utf8QString(ui->nameEdit->text()));
	settings->setDefaultUsername(utf8QString(ui->usernameEdit->text()));
	settings->setDescription(utf8QString(ui->descriptionEdit->document()->toPlainText()));

	int compression = ui->compressionCombo->currentIndex();
	if (compression == 0){
		settings->fileSettings.compress = false;
		settings->fileSettings.compression = Kdbx::Database::File::CompressionAlgorithm::GZip;
	}else{
		settings->fileSettings.compress = true;
		settings->fileSettings.compression = Kdbx::Database::File::CompressionAlgorithm::None;
	}
	//----

	int encryption = ui->encryptionCombo->currentIndex();
	if (encryption == 0){
		settings->fileSettings.encrypt = true;
		settings->fileSettings.cipherId = Kdbx::Database::File::AES_CBC_256_UUID;
	}else{
		settings->fileSettings.encrypt = false;
		settings->fileSettings.cipherId = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	}

	settings->fileSettings.transformRounds = ui->roundsSpin->value();

	// ToDo: recycle bin and templates are not saved for now...

	settings->historyMaxItems = ui->historyCountBox->value();
	settings->maintenanceHistoryDays = ui->historyDaysBox->value();
	settings->historyMaxSize = ui->historySizeBox->value()*(1024*1024);

	settings->memoryProtection.set(Kdbx::MemoryProtection::Title, ui->titleBox->isChecked());
	settings->memoryProtection.set(Kdbx::MemoryProtection::UserName, ui->usernameBox->isChecked());
	settings->memoryProtection.set(Kdbx::MemoryProtection::Password, ui->passwordBox->isChecked());
	settings->memoryProtection.set(Kdbx::MemoryProtection::Url, ui->urlBox->isChecked());
	settings->memoryProtection.set(Kdbx::MemoryProtection::Notes, ui->notesBox->isChecked());

	fdb->setSettings(std::move(settings));
}

void DatabaseSettings::on_recycleBinCombo_currentIndexChanged(int){
	if (!fdb)
		return;

	ui->recycleBin = ui->templatesCombo->view()->currentIndex();
}

void DatabaseSettings::on_templatesCombo_currentIndexChanged(int){
	if (!fdb)
		return;

	ui->templates = ui->templatesCombo->view()->currentIndex();
}

void DatabaseSettings::on_okButton_clicked(){
	toDatabase();
	accept();
}

void DatabaseSettings::on_reloadButton_clicked(){
	if (!fdb || fdb->frozen())
		return;

	fromDatabase(fdb);
}

void DatabaseSettings::drop(){
	fdb = nullptr;
	ui->tabWidget->setEnabled(false);
	ui->okButton->setEnabled(false);
}

