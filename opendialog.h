/*Copyright (C) 2016 Jaroslaw Kubik
 *
   This file is part of QtPass2.

QtPass2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QtPass2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QtPass2.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef OPENDIALOG_H
#define OPENDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <future>

#include <libkeepass2pp/compositekey.h>

namespace Ui {
class OpenDialog;
}

class OpenDialog : public QDialog{
	Q_OBJECT
public:
	explicit OpenDialog(std::promise<Kdbx::CompositeKey> compositePromise,
						QWidget *parent = 0);
	~OpenDialog();

	void setOpenDbName(const QString& name);

	QString password(); // Empty means no password;
	QString keyFile(); // Empty means no key file;

private slots:
	void on_passwordBox_toggled(bool checked);

	void on_passwordEdit_textChanged(const QString &arg1);

	void on_keyEdit_textChanged(const QString &arg1);

	void on_keyBox_toggled(bool checked);

	void on_keyBrowseButton_clicked();

	void onKeyBrowseDialogAccepted();


	void on_OpenDialog_accepted();

private:
	Ui::OpenDialog *ui;
	QFileDialog* dialog;
	std::promise<Kdbx::CompositeKey> compositePromise;
};

#endif // OPENDIALOG_H
