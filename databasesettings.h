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

private slots:
	void on_pushButton_clicked();

private:
	Ui::DatabaseSettings *ui;
};

#endif // DATABASESETTINGS_H
