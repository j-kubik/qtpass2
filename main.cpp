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
#include "qtpasswindow.h"
#include <QApplication>

void attachSSLEntropyEventFilter(QCoreApplication* app) noexcept;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setApplicationName("QtPass");
	a.setOrganizationName("Coobic.com");
	a.setOrganizationDomain("ritkub.from-de.com"); // Just for identification purposes.
	attachSSLEntropyEventFilter(&a);
	QtPassWindow w;
	w.show();

	return a.exec();
}
