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
	void saveDatabase();

private slots:
	void on_pushButton_clicked();

	void on_recycleBinCombo_currentIndexChanged(int index);
	void on_templatesCombo_currentIndexChanged(int index);

	void drop();

private:
	class MyUi;

	MyUi* ui;

	//Ui::DatabaseSettings *ui;
};

#endif // DATABASESETTINGS_H
