#ifndef GROUPEDITORDIALOG_H
#define GROUPEDITORDIALOG_H

#include <QDialog>
#include "qkdbxdatabase.h"

namespace Ui {
class GroupEditorDialog;
}

class GroupEditorDialog : public QDialog{
	Q_OBJECT

public:
	explicit GroupEditorDialog(QKdbxDatabase::Group group, QWidget *parent = 0);
	~GroupEditorDialog();

private slots:
	void disable();
	void onPropertiesChanged(QKdbxDatabase::Group group);
	void onGroupRemove(const QModelIndex& parent, int begin, int end);

	void on_GroupEditorDialog_accepted();

	void reload();

private:
	QKdbxDatabase::Group fgroup;
	Ui::GroupEditorDialog *ui;
};

#endif // GROUPEDITORDIALOG_H
