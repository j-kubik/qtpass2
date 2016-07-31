#include "newgroupdialog.h"
#include "ui_newgroupdialog.h"

NewGroupDialog::NewGroupDialog(QKdbxDatabase::Group group, QWidget *parent) :
	QDialog(parent),
	fparent(group),
	fnewUuid(Kdbx::Uuid::generate()),
	ui(new Ui::NewGroupDialog)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

	ui->setupUi(this);
	ui->groupEditor->setDefaults(group.model(), fnewUuid);
	ui->notificationFrame->setText(tr("Group '%1' was removed, new group will be placed in the root group.").arg(QString::fromUtf8(fparent.properties().name.c_str())));

	setDisabled(fparent.model()->frozen());
	connect(fparent.model(), &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
	connect(fparent.model(), &QObject::destroyed, this, &NewGroupDialog::disable);
	connect(fparent.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &NewGroupDialog::onGroupRemove);

}

NewGroupDialog::~NewGroupDialog()
{
	delete ui;
}

void NewGroupDialog::on_NewGroupDialog_accepted()
{
	if (!fparent){
		return;
	}

	Kdbx::Database::Group::Ptr group(new Kdbx::Database::Group(fnewUuid));
	group->setProperties(ui->groupEditor->toGroup());
	fparent.addGroup(std::move(group), fparent.groups());
}

void NewGroupDialog::disable(){
	fparent = QKdbxDatabase::Group();
	setWindowTitle(windowTitle() + tr(" (dropped)"));
	ui->createButton->setEnabled(false);
	ui->groupEditor->setEnabled(false);
}

void NewGroupDialog::onGroupRemove(const QModelIndex& parent, int begin, int end){

	QKdbxDatabase::Group g = fparent.model()->group(parent);

	for (;begin <= end; ++begin){
		if (fparent->ancestor(g.group(begin))){
			fparent = fparent.model()->root();
			ui->notificationFrame->show();
			return;
		}

	}
}
