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
#ifndef QKDBXGROUP_H
#define QKDBXGROUP_H

#include <libkeepass2pp/databasemodel.h>
#include <QAbstractTableModel>

#include "qkdbxdatabase.h"

class QKdbxGroup : public QAbstractTableModel{
	Q_OBJECT
private:
	QKdbxDatabase::Group fgroup;

public:
	explicit QKdbxGroup(QKdbxDatabase* database, QObject* parent);

	inline QKdbxDatabase::Group group() noexcept{
		return fgroup;
	}

	void setGroup(QKdbxDatabase::Group group) noexcept;

	QIcon icon() noexcept;

	Qt::ItemFlags flags(const QModelIndex & index) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;

	QKdbxDatabase::Entry entry(const QModelIndex& index) const noexcept;
	QKdbxDatabase::Version latest(const QModelIndex& index) const noexcept;

signals:

private:

private slots:
	void beginEntryAdd(QKdbxDatabase::Group parent, size_t index);
	void endEntryAdd(QKdbxDatabase::Group parent, size_t index);
	void beginEntryRemove(QKdbxDatabase::Group parent, size_t index);
	void endEntryRemove(QKdbxDatabase::Group parent, size_t index);

	void versionCountChanged(QKdbxDatabase::Entry entry);
	void groupAboutRemoved(const QModelIndex& index, int first, int last);

	void databaseDestroyed();
};

#endif // QKDBXGROUP_H
