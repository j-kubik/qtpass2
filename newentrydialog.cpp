#include "newentrydialog.h"
#include "ui_newentrydialog.h"

#include "qkdbxdatabase.h"

NewEntryDialog::NewEntryDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Group group, QWidget *parent) :
	QDialog(parent),
	fdb(group.model()),
	fversion(new Kdbx::Database::Version()),
	fuuid(group->uuid()),
	fnewUuid(Uuid::generate()),
	ui(new Ui::NewEntryDialog)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	ui->setupUi(this);

	ui->tabWidget->fromVersion(fdb, fnewUuid, fversion.get());
	connect(fdb, &QKdbxDatabase::databaseChanged, this, &NewEntryDialog::disable);
	connect(fdb, &QObject::destroyed, this, &NewEntryDialog::disable);
}

NewEntryDialog::~NewEntryDialog()
{
	delete ui;
}

void NewEntryDialog::disable(){
	fdb = nullptr;
	setWindowTitle(windowTitle() + tr(" (dropped)"));
	ui->createButton->setEnabled(false);
	ui->tabWidget->setEnabled(false);
}

void NewEntryDialog::on_createButton_clicked(){

	if (!fdb){
		reject();
		return;
	}

	QKdbxDatabase::Group g = fdb->group(fuuid);
	if (!g) // ToDo: Message box in case group was removed?
		return;

	ui->tabWidget->toVersion(fversion.get());
	Kdbx::Database::Entry::Ptr e(new Kdbx::Database::Entry(fnewUuid, std::move(fversion)));
	g.addEntry(std::move(e), g.entries());
	accept();
	disable();
}
