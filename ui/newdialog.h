#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
#include <libkeepass2pp/compositekey.h>

namespace Ui {
class NewDialog;
}

class NewDialog : public QDialog{
	Q_OBJECT

public:
	explicit NewDialog(QWidget *parent = 0);
	~NewDialog();

private slots:
	void on_passwordEdit_textChanged(const QString &arg1);

	void on_generateButton_clicked();

	void on_passwordBox_toggled(bool checked);

	void on_keyBrowseButton_clicked();

	void on_createButton_clicked();

	void on_createAccepted(QString filename);

private:
	Ui::NewDialog *ui;
};

#endif // NEWDIALOG_H
