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
#ifndef ICONDIALOG_H
#define ICONDIALOG_H

#include <QDialog>

#include "qkdbxdatabase.h"

namespace Ui {
class IconDialog;
}

class IconDialog : public QDialog{
	Q_OBJECT
public:
	explicit IconDialog(QKdbxDatabase* database, const Kdbx::Icon& selectedIcon, QWidget *parent = 0);
	~IconDialog();

signals:
	void iconAccepted(Kdbx::Icon icon);

private slots:
	void on_iconEditor_currentIconChanged(Kdbx::Icon current);
	void on_okButton_clicked();

	void disable();

private:
	Ui::IconDialog *ui;

};

#endif // ICONDIALOG_H
