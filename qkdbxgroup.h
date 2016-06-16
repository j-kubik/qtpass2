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

class QKdbxDatabase;

class QKdbxGroup : public QAbstractTableModel{
	Q_OBJECT
private:
	Kdbx::DatabaseModel<QKdbxDatabase>::Group fgroup;

public:
	explicit QKdbxGroup(QKdbxDatabase* database, QObject* parent);

	inline Kdbx::DatabaseModel<QKdbxDatabase>::Group group() noexcept{
		return fgroup;
	}

	void setGroup(Kdbx::DatabaseModel<QKdbxDatabase>::Group group) noexcept;

	QIcon icon() noexcept;

	Qt::ItemFlags flags(const QModelIndex & index) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;

	Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry(const QModelIndex& index) const noexcept;
	Kdbx::DatabaseModel<QKdbxDatabase>::Version latest(const QModelIndex& index) const noexcept;

signals:

private:

private slots:
	void beginEntryAdd(Kdbx::DatabaseModel<QKdbxDatabase>::Group parent, size_t index);
	void endEntryAdd(Kdbx::DatabaseModel<QKdbxDatabase>::Group parent, size_t index);
	void beginEntryRemove(Kdbx::DatabaseModel<QKdbxDatabase>::Group parent, size_t index);
	void endEntryRemove(Kdbx::DatabaseModel<QKdbxDatabase>::Group parent, size_t index);

	void versionCountChanged(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry);
	void groupAboutRemoved(const QModelIndex& index, int first, int last);

	void databaseDestroyed();
};

#endif // QKDBXGROUP_H
