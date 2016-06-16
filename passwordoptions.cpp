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
#include "passwordoptions.h"
#include "ui_passwordoptions.h"
#include "utils.h"

#include <openssl/err.h>
#include <openssl/rand.h>

PasswordOptions::PasswordOptions(QWidget *parent) :
	QGroupBox(parent),
	ui(new Ui::PasswordOptions)
{
	ui->setupUi(this);
}

PasswordOptions::~PasswordOptions()
{
	delete ui;
}

QString PasswordOptions::generate(){
	makePossibleChars();
	return generateImpl();
}

QStringList PasswordOptions::generate(unsigned int count){

	makePossibleChars();
	QStringList result;
	for (unsigned int i=0; i<count; i++){
		result.push_back(generateImpl());
	}
	return result;
}

void PasswordOptions::on_include_activated(const QString &arg1){
	int index = arg1.indexOf(": ");
	ui->include->setCurrentText(arg1.right(arg1.size() - index - 2));
}

void PasswordOptions::setDirty(){
	possibleChars.clear();
}

void PasswordOptions::makePossibleChars(){
	if (!possibleChars.empty()) return;

	QSet<QChar> chars;

	if (ui->uppercase->isChecked()){
		for (char c='A'; c<= 'Z'; ++c){
			chars.insert(c);
		}
	}
	if (ui->lowercase->isChecked()){
		for (char c='a'; c<= 'z'; ++c){
			chars.insert(c);
		}
	}

	if (ui->digits->isChecked()){
		for (char c='0'; c<= '9'; ++c){
			chars.insert(c);
		}
	}

	if (ui->minus->isChecked()){
		chars.insert('-');
	}

	if (ui->underline->isChecked()){
		chars.insert('_');
	}

	if (ui->space->isChecked()){
		chars.insert(' ');
	}

	if (ui->specials->isChecked()){
		chars.insert('!');
		chars.insert('"');
		chars.insert('#');
		chars.insert('$');
		chars.insert('%');
		chars.insert('&');
		chars.insert('\'');
		chars.insert('*');
		chars.insert('+');
		chars.insert(',');
		chars.insert('.');
		chars.insert('/');
		chars.insert(':');
		chars.insert(';');
		chars.insert('=');
		chars.insert('?');
		chars.insert('@');
		chars.insert('|');
		chars.insert('~');
		chars.insert('!');
	}

	if (ui->brackets->isChecked()){
		chars.insert('[');
		chars.insert(']');
		chars.insert('{');
		chars.insert('}');
		chars.insert('(');
		chars.insert(')');
		chars.insert('<');
		chars.insert('>');
	}

	if (ui->uniCharacters->isChecked()){
		for (int i=0x00A0; i<0x02B0; ++i){
			chars.insert(i);
		}
	}

	QString tmp = ui->include->currentText();

	for (QChar c: tmp){
		chars.insert(c);
	}

	tmp = ui->exclude->text();
	for (QChar c: tmp){
		chars.remove(c);
	}

	possibleChars = chars.toList();

	if (possibleChars.empty()){
		QByteArray arr = tr("0 possible characters to use to genrate a password.").toUtf8();
		throw std::runtime_error(std::string(arr.data(), arr.size()));
	}
}

QString PasswordOptions::generateImpl(){

	QString result;
	unsigned int passLength = ui->passLength->value();
	unsigned int possibleCharsLength = possibleChars.length();
	unsigned int maxValue = 0xffff - 0xffff%possibleCharsLength;

	std::vector<unsigned char> randomData = getRandomBytes(passLength*2, this);
	unsigned int randomAt = 0;

	for (unsigned int i=0; i<passLength; i++){
		unsigned int randomIndex;
		do{
			if (randomAt >= randomData.size()){
				randomData = getRandomBytes((passLength-i)*2, this);
				randomAt = 0;
			}

			randomIndex = randomData.at(randomAt);
			randomAt++;
			if (randomAt >= randomData.size()){
				randomData = getRandomBytes((passLength-i)*2, this);
				randomAt = 0;
			}
			randomIndex = randomIndex << 8 | randomData.at(randomAt);
		}while(randomIndex > maxValue);
		randomAt++;

		randomIndex %= possibleCharsLength;
		result += possibleChars.at(randomIndex);
	}

	return result;
}

