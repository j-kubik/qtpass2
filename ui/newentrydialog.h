#ifndef NEWENTRYDIALOG_H
#define NEWENTRYDIALOG_H

#include <QDialog>
#include "qkdbxdatabase.h"

namespace Ui {
class NewEntryDialog;
}

class QKdbxGroup;
class QKdbxDatabase;

class NewEntryDialog : public QDialog{
	Q_OBJECT

	Kdbx::Database::Version::Ptr fversion;
	QKdbxDatabase::Group fgroup;
	Kdbx::Uuid fnewUuid;
public:
	explicit NewEntryDialog(QKdbxDatabase::Group group, QWidget *parent = 0);
	~NewEntryDialog();

private slots:
	void disable();
	void on_createButton_clicked();

	void onGroupRemove(const QModelIndex& parent, int begin, int end);

private:
	Ui::NewEntryDialog *ui;
};

#endif // NEWENTRYDIALOG_H
