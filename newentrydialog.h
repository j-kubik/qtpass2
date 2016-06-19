#ifndef NEWENTRYDIALOG_H
#define NEWENTRYDIALOG_H

#include <QDialog>
#include <libkeepass2pp/databasemodel.h>

namespace Ui {
class NewEntryDialog;
}

class QKdbxGroup;
class QKdbxDatabase;

class NewEntryDialog : public QDialog{
	Q_OBJECT

	QKdbxDatabase* fdb;
	Kdbx::Database::Version::Ptr fversion;
	Uuid fuuid;
	Uuid fnewUuid;
public:
	explicit NewEntryDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Group group, QWidget *parent = 0);
	~NewEntryDialog();

private slots:
	void disable();
	void on_createButton_clicked();

private:
	Ui::NewEntryDialog *ui;
};

#endif // NEWENTRYDIALOG_H
