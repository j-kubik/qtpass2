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
#ifndef UTILS_H
#define UTILS_H

#include <QWidget>
#include <vector>

#include <libkeepass2pp/util.h>

std::vector<uint8_t> getRandomBytes(std::size_t size, QWidget*);
SafeVector<uint8_t> safeGetRandomBytes(std::size_t size, QWidget*);

inline std::string utf8QString(QString s){
	QByteArray tmp = s.toUtf8();
	return std::string(tmp.data(), tmp.size());
}

#endif // UTILS_H
