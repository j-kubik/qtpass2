#include "newentrydialog.h"
#include "ui_newentrydialog.h"

#include "qkdbxdatabase.h"

using namespace Kdbx;

NewEntryDialog::NewEntryDialog(QKdbxDatabase::Group group, QWidget *parent) :
	QDialog(parent),
	fversion(new Kdbx::Database::Version()),
	fgroup(group),
	fnewUuid(Uuid::generate()),
	ui(new Ui::NewEntryDialog)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	ui->setupUi(this);
	ui->tabWidget->fromVersion(fgroup.model(), fnewUuid, fversion.get());
	ui->updateLabel->setText(tr("Group '%1' was removed, new entry will be placed in the root group.").arg(QString::fromUtf8(fgroup.properties().name.c_str())));
	ui->updateFrame->hide();

	setDisabled(fgroup.model()->frozen());
	connect(fgroup.model(), &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
	connect(fgroup.model(), &QObject::destroyed, this, &NewEntryDialog::disable);
	connect(fgroup.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &NewEntryDialog::onGroupRemove);
}

NewEntryDialog::~NewEntryDialog()
{
	delete ui;
}

void NewEntryDialog::disable(){
	fgroup = QKdbxDatabase::Group();
	setWindowTitle(windowTitle() + tr(" (dropped)"));
	ui->createButton->setEnabled(false);
	ui->tabWidget->setEnabled(false);
}

void NewEntryDialog::on_createButton_clicked(){

	if (!fgroup){
		reject();
		return;
	}

	ui->tabWidget->toVersion(fversion.get());
	Kdbx::Database::Entry::Ptr e(new Kdbx::Database::Entry(fnewUuid, std::move(fversion)));
	fgroup.addEntry(std::move(e), fgroup.entries());
	accept();
	close();
}

void NewEntryDialog::onGroupRemove(const QModelIndex& parent, int begin, int end){

	QKdbxDatabase::Group g = fgroup.model()->group(parent);

	for (;begin <= end; ++begin){
		if (fgroup->ancestor(g.group(begin))){
			fgroup = fgroup.model()->root();
			ui->updateFrame->show();
			return;
		}

	}
}

