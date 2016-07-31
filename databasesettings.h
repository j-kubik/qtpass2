#ifndef DATABASESETTINGS_H
#define DATABASESETTINGS_H

#include <QDialog>

namespace Ui {
class DatabaseSettings;
}

class QKdbxDatabase;

class DatabaseSettings : public QDialog
{
	Q_OBJECT

	QKdbxDatabase* fdb;

public:
	explicit DatabaseSettings(QKdbxDatabase* db, QWidget *parent = 0);
	~DatabaseSettings();

	void fromDatabase(QKdbxDatabase* db);
	void toDatabase();

private slots:
	void on_okButton_clicked();

	void reload();
	void drop();

private:
	Ui::DatabaseSettings *ui;
};

#endif // DATABASESETTINGS_H
