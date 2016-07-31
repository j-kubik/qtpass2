#ifndef NEWGROUPDIALOG_H
#define NEWGROUPDIALOG_H

#include <QDialog>
#include "qkdbxdatabase.h"

namespace Ui {
class NewGroupDialog;
}

class NewGroupDialog : public QDialog{
	Q_OBJECT

public:
	explicit NewGroupDialog(QKdbxDatabase::Group group, QWidget *parent = 0);
	~NewGroupDialog();

	QKdbxDatabase::Group fparent;
	Kdbx::Uuid fnewUuid;

private slots:
	void on_NewGroupDialog_accepted();

	void disable();
	void onGroupRemove(const QModelIndex& parent, int begin, int end);


private:
	Ui::NewGroupDialog *ui;
};

#endif // NEWGROUPDIALOG_H
