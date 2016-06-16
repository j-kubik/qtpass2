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
#ifndef PASSWORDOPTIONS_H
#define PASSWORDOPTIONS_H

#include <QGroupBox>

namespace Ui {
class PasswordOptions;
}

class PasswordOptions : public QGroupBox{
	Q_OBJECT

public:
	explicit PasswordOptions(QWidget *parent = 0);
	~PasswordOptions();

	QString generate();
	QStringList generate(unsigned int count);

private slots:

	void on_include_activated(const QString &arg1);
	void setDirty();

private:
	void makePossibleChars();
	QString generateImpl();


	Ui::PasswordOptions *ui;
	QList<QChar> possibleChars;
};

#endif // PASSWORDOPTIONS_H
