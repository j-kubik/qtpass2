#include "groupeditor.h"
#include "ui_groupeditor.h"
#include "utils.h"

#include <QDateTime>

GroupEditor::GroupEditor(QWidget *parent) :
	QTabWidget(parent),
	ui(new Ui::GroupEditor)
{
	ui->setupUi(this);
}

GroupEditor::~GroupEditor()
{
	delete ui;
}

void GroupEditor::setDefaults(QKdbxDatabase* db, const Kdbx::Uuid& uuid){
	fdb = db;

	ui->iconButton->setDatabase(fdb);
	ui->iconButton->setDefaultIcon(Kdbx::StandardIcon::Folder);
	ui->iconButton->setCurrentIcon(Kdbx::Icon(Kdbx::StandardIcon::Folder));

	ui->expireEdit->setEnabled(false);

	std::string suuid = std::string(uuid);
	ui->uuidLabel->setText(QString::fromUtf8(suuid.c_str(), suuid.size()));
}


void GroupEditor::fromGroup(QKdbxDatabase* db, const Kdbx::Database::Group::Properties& properties, const Kdbx::Uuid& uuid){

	fdb = db;
	fproperties = Kdbx::Database::Group::Properties::Ptr(new Kdbx::Database::Group::Properties(properties));

	ui->iconButton->setDatabase(fdb);
	ui->iconButton->setDefaultIcon(Kdbx::StandardIcon::Folder);
	ui->iconButton->setCurrentIcon(fproperties->icon);
	ui->titleEdit->setText(QString::fromUtf8(fproperties->name.c_str(), fproperties->name.size()));
	ui->searchBox->setChecked(fproperties->enableSearching);
	ui->expireBox->setChecked(fproperties->times.expires);
	if (fproperties->times.expires){
		ui->expireEdit->setDateTime(QDateTime::fromTime_t(fproperties->times.expiry, Qt::UTC));
	}else{
		ui->expireEdit->setEnabled(false);
	}
	ui->notesEdit->document()->setPlainText(QString::fromUtf8(fproperties->notes.c_str(), fproperties->notes.size()));


	std::string suuid = std::string(uuid);
	ui->uuidLabel->setText(QString::fromUtf8(suuid.c_str(), suuid.size()));

	if (fproperties->times.creation)
		ui->createdLabel->setText(QDateTime::fromTime_t(fproperties->times.creation).toString(Qt::SystemLocaleLongDate));
	if (fproperties->times.lastModification)
		ui->lastModifiedLabel->setText(QDateTime::fromTime_t(fproperties->times.lastModification).toString(Qt::SystemLocaleLongDate));
	if (fproperties->times.lastAccess)
		ui->lastAccessedLabel->setText(QDateTime::fromTime_t(fproperties->times.lastAccess).toString(Qt::SystemLocaleLongDate));
	if (fproperties->times.locationChanged)
		ui->locationChangedLabel->setText(QDateTime::fromTime_t(fproperties->times.locationChanged).toString(Qt::SystemLocaleLongDate));
	ui->usageCountLabel->setText(QString("%1").arg(fproperties->times.usageCount));
}

void GroupEditor::fromGroup(QKdbxDatabase::Group group){
	fromGroup(group.model(), group.properties(), group->uuid());
}

Kdbx::Database::Group::Properties::Ptr GroupEditor::toGroup(){

	if (!fproperties){
		fproperties = Kdbx::Database::Group::Properties::Ptr(new Kdbx::Database::Group::Properties());
	}

	fproperties->icon = ui->iconButton->currentIcon();

	fproperties->name = utf8QString(ui->titleEdit->text());
	fproperties->enableSearching = ui->searchBox->isChecked();
	if ((fproperties->times.expires = ui->expireBox->isChecked())){
		fproperties->times.expiry = ui->expireEdit->dateTime().toTime_t();
	}else{
		fproperties->times.expiry = 0;
	}
	fproperties->notes = utf8QString(ui->notesEdit->document()->toPlainText());
	fproperties->times.lastModification = std::time(nullptr);
	return std::move(fproperties);
}




