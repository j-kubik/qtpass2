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
#ifndef DATABASEVIEWWIDGET_H
#define DATABASEVIEWWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QUndoStack>

class DatabaseViewWidget: public QWidget{
	Q_OBJECT
public:

	using QWidget::QWidget;

	virtual QIcon icon() const =0;
	virtual QString name() const =0;
	virtual QString style() const =0;
	virtual void saveSettings(QSettings& s) const =0;
	virtual void applySettings(QSettings& s) =0;
	virtual QUndoStack* undoStack() =0;
	virtual void addEntryAction() =0;

signals:
	virtual void addEntryActionEnabled(bool enabled);
};


#endif // DATABASEVIEWWIDGET_H
