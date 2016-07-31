#include "notificationframe.h"

#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>

class NotificationFrame::Ui{
public:
	QLabel* label;
	QToolButton* dismiss;
	QToolButton* update;
};

NotificationFrame::NotificationFrame(QWidget* parent)
	:QFrame(parent),
	  ui(new Ui())
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	ui->label = new QLabel(this);
	layout->addWidget(ui->label, 1, Qt::AlignVCenter | Qt::AlignLeft);

	ui->update = new QToolButton(this);
	ui->update->setToolButtonStyle(Qt::ToolButtonIconOnly);
	ui->update->setIcon(QIcon(":/kdbx/icons/16x16/r.png"));
	ui->update->setText(tr("Update"));
	layout->addWidget(ui->update, 0, Qt::AlignVCenter | Qt::AlignLeft);

	ui->dismiss = new QToolButton(this);
	ui->dismiss->setToolButtonStyle(Qt::ToolButtonIconOnly);
	ui->dismiss->setIcon(QIcon(":/kdbx/icons/16x16/x.png"));
	ui->dismiss->setText(tr("Dismiss"));
	layout->addWidget(ui->dismiss, 0, Qt::AlignVCenter | Qt::AlignLeft);

	layout->setContentsMargins(8,2,2,2);

	setButtons(Dismiss);

	connect(ui->update, &QToolButton::clicked, this, &NotificationFrame::hide);
	connect(ui->update, &QToolButton::clicked, this, &NotificationFrame::updated);
	connect(ui->dismiss, &QToolButton::clicked, this, &NotificationFrame::hide);
	connect(ui->dismiss, &QToolButton::clicked, this, &NotificationFrame::dismissed);

	hide();
}

NotificationFrame::~NotificationFrame(){
	delete ui;
}

QString NotificationFrame::text(){
	return ui->label->text();
}

void NotificationFrame::setButtons(Buttons buttons){
	fbuttons = buttons;

	ui->dismiss->setHidden(!fbuttons.testFlag(Dismiss));
	ui->update->setHidden(!fbuttons.testFlag(Update));
}

void NotificationFrame::setText(QString text){
	ui->label->setText(text);
}
