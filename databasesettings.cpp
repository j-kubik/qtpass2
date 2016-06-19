#include "databasesettings.h"
#include "ui_databasesettings.h"

#include "qkdbxdatabase.h"

#include <QTreeView>

DatabaseSettings::DatabaseSettings(QKdbxDatabase* db, QWidget *parent) :
	QDialog(parent),
	fdb(db),
	ui(new Ui::DatabaseSettings)
{
	ui->setupUi(this);
	const Kdbx::Database::Settings& settings = db->get()->settings();
	const Kdbx::Database::File::Settings& fileSettings = db->fileSettings();

	ui->nameEdit->setText(QString::fromUtf8(settings.databaseName.c_str(), settings.databaseName.size()));
	ui->usernameEdit->setText(QString::fromUtf8(settings.defaultUsername.c_str(), settings.defaultUsername.size()));
	ui->descriptionEdit->document()->setPlainText(QString::fromUtf8(settings.databaseDescription.c_str(), settings.databaseDescription.size()));

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

	ui->recycleBinCombo->setModel(db);
	ui->recycleBinCombo->setModelColumn(0);
	QTreeView* view = new QTreeView();
	ui->recycleBinCombo->setView(view);

	QKdbxDatabase::Group g;
	if (settings.recycleBinEnabled && (g = db->group(settings.recycleBinUUID))){
		ui->recycleBinBox->setChecked(true);
		ui->recycleBinCombo->setEnabled(true);
		view->setCurrentIndex(db->index(g, 0));
	}else{
		ui->recycleBinCombo->setEnabled(false);
	}


}

DatabaseSettings::~DatabaseSettings()
{
	delete ui;
}

void DatabaseSettings::on_pushButton_clicked()
{
	//
}
