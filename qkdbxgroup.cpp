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
#include "qkdbxgroup.h"
#include "qkdbxdatabase.h"

#include <QDateTime>

QKdbxGroup::QKdbxGroup(QKdbxDatabase* database, QObject* parent) :
	QAbstractTableModel(parent),
	fgroup(database->root())
{
	connect(database, &QKdbxDatabase::beginEntryAdd, this, &QKdbxGroup::beginEntryAdd);
	connect(database, &QKdbxDatabase::endEntryAdd, this, &QKdbxGroup::endEntryAdd);
	connect(database, &QKdbxDatabase::beginEntryRemove, this, &QKdbxGroup::beginEntryRemove);
	connect(database, &QKdbxDatabase::endEntryRemove, this, &QKdbxGroup::endEntryRemove);
	connect(database, &QKdbxDatabase::endVersionAdd, this, &QKdbxGroup::versionCountChanged);
	connect(database, &QKdbxDatabase::endVersionRemove, this, &QKdbxGroup::versionCountChanged);
	connect(database, &QKdbxDatabase::rowsAboutToBeRemoved, this, &QKdbxGroup::groupAboutRemoved);
	//connect(database, &QKdbxDatabase::databaseDestroyed, this, &QKdbxGroup::databaseDestroyed);
}

void QKdbxGroup::setGroup(QKdbxDatabase::Group group) noexcept{
	if (fgroup != group){
		beginResetModel();
		fgroup = group;
		endResetModel();
	}
}

QIcon QKdbxGroup::icon() noexcept{
	if (!fgroup)
		return QIcon();

	return fgroup.model()->icon(fgroup);
}

Qt::ItemFlags QKdbxGroup::flags(const QModelIndex & index) const{
	unused(index);
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant QKdbxGroup::data(const QModelIndex & index, int role) const{
	if (!fgroup || !index.isValid() || index.row() >= int(fgroup->entries()))
		return QVariant();

	const QKdbxDatabase::Version version = latest(index);

	switch (role){
	case Qt::DisplayRole:
		switch(index.column()){
		case 0:
			return QKdbxDatabase::entryString(version, Kdbx::Database::Version::titleString);
		case 1:
			return QKdbxDatabase::entryString(version, Kdbx::Database::Version::userNameString);
		case 2:{
			int size = QKdbxDatabase::entryStringBuffer(version, Kdbx::Database::Version::passwordString).size();
			QString result;
			while (size--) result += '*';
			return result;
		}
		case 3:
			return QKdbxDatabase::entryString(version, Kdbx::Database::Version::urlString);
		case 4:
			return QKdbxDatabase::entryString(version, Kdbx::Database::Version::notesString);
		case 5:{
			QDateTime t = QDateTime::fromTime_t(version->times.creation);
			t.setTimeSpec(Qt::LocalTime);
			return t.toString(Qt::SystemLocaleShortDate);
		}
		case 6:{
			QDateTime t = QDateTime::fromTime_t(version->times.lastModification);
			t.setTimeSpec(Qt::LocalTime);
			return t.toString(Qt::SystemLocaleShortDate);
		}
		case 7:
			if (version->times.expires){
				QDateTime t = QDateTime::fromTime_t(version->times.expiry);
				t.setTimeSpec(Qt::LocalTime);
				return t.toString(Qt::SystemLocaleShortDate);
			}
			return QString();
		case 8:
			return QString::fromStdString(std::string(version->parent()->uuid()));
		case 9:
			return QString("%1").arg(version->binaries.size());
		case 10:
			return QString("%1").arg(version->parent()->versions());
		case 12:{
			QString result;
			for (const std::string& s: version->tags){
				if (result.length()) result += ",";
				result += QString::fromUtf8(s.c_str(), s.size());
			}
			return result;
		}
		default:
			return QVariant();
		}
	case Qt::DecorationRole:
		if (index.column() != 0) break;
		return version.model()->icon(version);

	default:
		break;
	}

	return QVariant();

}

QVariant QKdbxGroup::headerData(int section, Qt::Orientation orientation, int role) const{
	if (role != Qt::DisplayRole || section > 11)
		return QAbstractItemModel::headerData(section, orientation, role);

	switch (section){
	default:
	case 0:
		return tr("Title");
	case 1:
		return tr("Username");
	case 2:
		return tr("Password");
	case 3:
		return tr("URL");
	case 4:
		return tr("Notes");
	case 5:
		return tr("Creation time");
	case 6:
		return tr("Last modification time");
	case 7:
		return tr("Expiry time");
	case 8:
		return tr("UUID");
	case 9:
		return tr("Attachments");
	case 10:
		return tr("History count");
	case 11:
		return tr("Tags");
	}

}

int QKdbxGroup::rowCount(const QModelIndex & parent) const{
	if (!fgroup || parent.isValid())
		return 0;
	//std::cout << "Fgroup: " << fgroup << std::endl;
	return fgroup->entries();
}

int QKdbxGroup::columnCount(const QModelIndex & parent) const{
	unused(parent);
	return 12;
}

/** Returns entry that correponds to a model index.
 * Produces UB if model index is invalid or belongs to a different model.
 */
QKdbxDatabase::Entry QKdbxGroup::entry(const QModelIndex& index) const noexcept{
	assert(fgroup);
	return fgroup.entry(index.row());
}

//const Kdbx::DatabaseModel::Entry QKdbxGroup::entry(const QModelIndex& index) const noexcept{
//	assert(bool(fgroup));
//	return fgroup.entry(index.row());
//}

QKdbxDatabase::Version QKdbxGroup::latest(const QModelIndex& index) const noexcept{
	assert(bool(fgroup) != 0);
	QKdbxDatabase::Entry e = entry(index);
	assert(e->versions() > 0);
	return e.latest();
}

void QKdbxGroup::beginEntryAdd(QKdbxDatabase::Group parent, size_t index){
	assert(parent);
	assert(parent->entries() >= index);
	assert(fgroup);
	if (parent == fgroup)
		beginInsertRows(QModelIndex(), index, index);
}

void QKdbxGroup::endEntryAdd(QKdbxDatabase::Group parent, size_t index){
	assert(parent);
	assert(parent->entries() >= index);
	assert(fgroup);
	unused(index);
	if (parent == fgroup)
		endInsertRows();
}

void QKdbxGroup::beginEntryRemove(QKdbxDatabase::Group parent, size_t index){
	assert(parent);
	assert(parent->entries() > index);
	assert(fgroup);
	if (parent == fgroup)
		beginRemoveRows(QModelIndex(), index, index);
}

void QKdbxGroup::endEntryRemove(QKdbxDatabase::Group parent, size_t index){
	assert(parent);
	assert(parent->entries() >= index);
	assert(fgroup);
	if (parent == fgroup)
		endRemoveRows();
}

void QKdbxGroup::versionCountChanged(QKdbxDatabase::Entry entry){
	assert(entry);
	assert(fgroup);
	if (entry.parent() == fgroup){
		for (size_t i=0; i<fgroup.entries(); i++){
			if (fgroup.entry(i) == entry){
				emit dataChanged(index(i, 0), index(i, 11));
				return;
			}
		}
	}
}

void QKdbxGroup::groupAboutRemoved(const QModelIndex& index, int first, int last){
	QKdbxDatabase::Group removed = fgroup.model()->group(index);
	QKdbxDatabase::Group group = fgroup;
	do{
		if (group == removed){
			setGroup(QKdbxDatabase::Group());
			return;
		}
	}while ((group = group.parent()));

	for (;first <= last; ++first){
		if (removed.group(first) == fgroup){
			setGroup(QKdbxDatabase::Group());
			return;
		}
	}

}

void QKdbxGroup::databaseDestroyed(){
	setGroup(QKdbxDatabase::Group());
}



