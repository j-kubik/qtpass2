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
#ifndef PASSWORDCHANGEDIALOG_H
#define PASSWORDCHANGEDIALOG_H

#include <QDialog>

namespace Ui {
class PasswordChangeDialog;
}

class PasswordChangeDialog : public QDialog
{
	Q_OBJECT

public:
	explicit PasswordChangeDialog(QString current, QWidget *parent = 0);
	~PasswordChangeDialog();

signals:
	void passwordChanged(QString password);

private slots:
	void passwordAccepted();
	bool checkPassword2();

	void onPasswordGenrated(QString pass);

	void on_show_toggled(bool checked);
	void on_generate_clicked();

private:
	Ui::PasswordChangeDialog *ui;
};

#endif // PASSWORDCHANGEDIALOG_H
