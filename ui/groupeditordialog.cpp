#include "groupeditordialog.h"
#include "ui_groupeditordialog.h"

GroupEditorDialog::GroupEditorDialog(QKdbxDatabase::Group group, QWidget *parent) :
	QDialog(parent),
	fgroup(group),
	ui(new Ui::GroupEditorDialog)
{
	assert(fgroup);

	setAttribute(Qt::WA_DeleteOnClose, true);

	ui->setupUi(this);

	QString name= QString::fromUtf8(fgroup.properties().name.c_str(), fgroup.properties().name.size());
	setWindowTitle(tr("Group: %1").arg(name));
	ui->notifyFrame->setButtons(NotificationFrame::Update | NotificationFrame::Dismiss);
	ui->notifyFrame->setText(tr("Group %1 properties changed.").arg(name));

	ui->groupEditor->fromGroup(fgroup);

	setDisabled(fgroup.model()->frozen());
	connect(ui->notifyFrame, &NotificationFrame::updated, this, &GroupEditorDialog::reload);
	connect(fgroup.model(), &QKdbxDatabase::frozenChanged, this, &QWidget::setDisabled);
	connect(fgroup.model(), &QObject::destroyed, this, &GroupEditorDialog::disable);
	connect(fgroup.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &GroupEditorDialog::onGroupRemove);
	connect(fgroup.model(), &QKdbxDatabase::groupPropertiesChanged, this, &GroupEditorDialog::onPropertiesChanged);
}

GroupEditorDialog::~GroupEditorDialog()
{
	delete ui;
}

void GroupEditorDialog::disable(){
	fgroup = QKdbxDatabase::Group();
	setWindowTitle(windowTitle() + tr(" (dropped)"));
	ui->updateButton->setEnabled(false);
	ui->groupEditor->setEnabled(false);
}

void GroupEditorDialog::onPropertiesChanged(QKdbxDatabase::Group group){
	if (group == fgroup)
		ui->notifyFrame->show();
}

void GroupEditorDialog::onGroupRemove(const QModelIndex& parent, int begin, int end){
	QKdbxDatabase::Group g = fgroup.model()->group(parent);

	for (;begin <= end; ++begin){
		if (fgroup->ancestor(g.group(begin))){
			disable();
			return;
		}
	}
}


void GroupEditorDialog::on_GroupEditorDialog_accepted(){
	if (!fgroup)
		return;
	fgroup.setProperties(ui->groupEditor->toGroup());
}

void GroupEditorDialog::reload(){
	if (fgroup){
		ui->groupEditor->fromGroup(fgroup);
	}
}
