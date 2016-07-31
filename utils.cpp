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
#include "utils.h"

#include <openssl/rand.h>
#include <openssl/err.h>
#include <sstream>

#include <QMessageBox>

std::vector<uint8_t> getRandomBytes(std::size_t size, QWidget* parent){
	while (ERR_get_error());

	std::vector<unsigned char> buffer(size);

	int result = RAND_bytes(reinterpret_cast<unsigned char*>(buffer.data()), size);
	if (result != 1){
		QMessageBox::warning(parent, "This should show a window that gathers user input to appear.\nIt will get implemented at some point.\nI promise!!!", "Sorry, not implemented yet.");
		std::stringstream s;
		QByteArray arr = QObject::tr("Error generating a password.").toUtf8();
		s.write(arr.data(), arr.size());
		s << "\n";
		unsigned long lastError;
		while ((lastError = ERR_get_error())){
			char strBuf[256];
			ERR_error_string_n(lastError, &strBuf[0], 256);
			s << strBuf << "\n";
		}
		throw std::runtime_error(s.str());
	}

	return buffer;
}

Kdbx::SafeVector<uint8_t> safeGetRandomBytes(std::size_t size, QWidget* parent){
	while (ERR_get_error());

	Kdbx::SafeVector<uint8_t> buffer(size);

	int result = RAND_bytes(reinterpret_cast<unsigned char*>(buffer.data()), size);
	if (result != 1){
		QMessageBox::warning(parent, "This should show a window that gathers user input to appear.\nIt will get implemented at some point.\nI promise!!!", "Sorry, not implemented yet.");
		std::stringstream s;
		QByteArray arr = QObject::tr("Error generating a password.").toUtf8();
		s.write(arr.data(), arr.size());
		s << "\n";
		unsigned long lastError;
		while ((lastError = ERR_get_error())){
			char strBuf[256];
			ERR_error_string_n(lastError, &strBuf[0], 256);
			s << strBuf << "\n";
		}
		throw std::runtime_error(s.str());
	}

	return buffer;
}


