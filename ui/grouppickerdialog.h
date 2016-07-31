#ifndef GROUPPICKERDIALOG_H
#define GROUPPICKERDIALOG_H

#include <QDialog>
#include <QToolButton>

#include "qkdbxdatabase.h"

namespace Ui {
class GroupPickerDialog;
}

class GroupPickerDialog : public QDialog{
	Q_OBJECT

public:
	GroupPickerDialog(QKdbxDatabase* db, QKdbxDatabase::Group current, QWidget *parent = 0);
	GroupPickerDialog(QKdbxDatabase* db, QWidget *parent = 0);
	~GroupPickerDialog();

	QKdbxDatabase::Group current();

	inline bool allowNull() noexcept{
		return fallowNull;
	}

public slots:
	void disable();
	void setCurrent(QKdbxDatabase::Group current);
	void setAllowNull(bool allow);

signals:
	void groupAccepted(QKdbxDatabase::Group group);

private slots:
	void onCurrentChanged();
	void on_GroupPickerDialog_accepted();
	void on_okButton_clicked();

private:
	void nullCheck();

	Ui::GroupPickerDialog *ui;

	QKdbxDatabase* fdb;
	bool fallowNull;
};

//------------------------------------------------------------------------------

class GroupButton: public QToolButton{
	Q_OBJECT
public:
	GroupButton(QKdbxDatabase* db, QKdbxDatabase::Group current, QWidget* parent = 0);
	GroupButton(QKdbxDatabase* db, QWidget* parent = 0);
	GroupButton(QWidget* parent = 0);

	inline QKdbxDatabase::Group current() noexcept{
		return fcurrent;
	}

	inline bool allowNull() noexcept{
		return fallowNull;
	}

public slots:
	void disable();
	void setCurrent(QKdbxDatabase::Group current);
	void setDatabase(QKdbxDatabase* db);
	void setAllowNull(bool allow);

private slots:
	void onClicked();
	void onGroupRemoved(const QModelIndex& parent, int first, int last);

private:
	QKdbxDatabase* fdb;
	QKdbxDatabase::Group fcurrent;
	bool fallowNull;


};



#endif // GROUPPICKERDIALOG_H
