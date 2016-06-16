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
#ifndef QKDBXDATABASE_H
#define QKDBXDATABASE_H

#include <libkeepass2pp/databasemodel.h>

#include <QAbstractListModel>
#include <QMap>
#include <QIcon>
#include <QUndoStack>

class QKdbxGroup;

class DatabaseCommand;

class QKdbxDatabase: public QAbstractItemModel, public Kdbx::DatabaseModel<QKdbxDatabase>{
public:
	class Icons;
private:
	Q_OBJECT

	static const QString dataMimeType;
	Icons* ficons;
	QUndoStack* fundoStack;
	friend class Icons;
public:

	typedef Kdbx::DatabaseModel<QKdbxDatabase>::Group Group;
	typedef Kdbx::DatabaseModel<QKdbxDatabase>::Entry Entry;
	typedef Kdbx::DatabaseModel<QKdbxDatabase>::Version Version;

	typedef Kdbx::DatabaseModel<const QKdbxDatabase>::Group CGroup;
	typedef Kdbx::DatabaseModel<const QKdbxDatabase>::Entry CEntry;
	typedef Kdbx::DatabaseModel<const QKdbxDatabase>::Version CVersion;

	class Icons: public QAbstractListModel{
	private:
		Icons(QKdbxDatabase* database) noexcept;

		void reset(Kdbx::Database* newDatabase);
		Kdbx::Icon insertQIcon(QIcon icon, Kdbx::CustomIcon::Ptr customIcon);

		QKdbxDatabase* fdatabase;
		std::map<const Uuid, QIcon> entries;

	public:

		int rowCount(const QModelIndex &) const override;
		QVariant data(const QModelIndex &index, int role = Qt::DecorationRole) const override;

		inline QModelIndex modelIndex(const Uuid& uuid, int column=0) const noexcept{
			int idx = index(uuid);
			if (idx < 0)
				return QModelIndex();
			return createIndex(idx, column);
		}

		inline int index(const Uuid& uuid) const noexcept{
			return fdatabase->get()->customIconIndex(uuid);
		}

		inline const Uuid& uuid(int index) const noexcept{
			return fdatabase->get()->customIcon(index)->uuid();
		}

		inline const QIcon& icon(int index) const noexcept{
			return entries.at(fdatabase->get()->customIcon(index)->uuid());
		}

		inline QIcon icon(const Uuid& uuid) const noexcept{
			int idx = index(uuid);
			if (idx >= 0){
			   return entries.at(fdatabase->get()->customIcon(idx)->uuid());
			}
			return QIcon();
		}

		inline int size() const noexcept{
			return fdatabase->get()->customIcons();
		}

		Uuid insert(QIcon icon);
		void insert(QIcon icon, Uuid uuid);

		QIcon icon(Kdbx::Icon i) const noexcept;

		static QIcon customQIcon(const Kdbx::CustomIcon::Ptr& customIcon);
		static QIcon standardIcon(Kdbx::StandardIcon icon) noexcept;

		friend class QKdbxDatabase;
	};

	QKdbxDatabase(Kdbx::Database::Ptr database, QObject* parent=0);
	//QKdbxDatabase(IFile file, const Kdbx::CompositeKey& key, QObject* parent=0);
	~QKdbxDatabase() noexcept;

	QModelIndex root() const noexcept;

	QModelIndex index(const Kdbx::Database::Group* group, int column) const noexcept;
	QModelIndex index(const Kdbx::Database::Group* group, int row, int column) const noexcept;
	QModelIndex index(CGroup group, int column) const noexcept;
	QModelIndex index(CGroup group, int row, int column) const noexcept;

	Group rootGroup() noexcept;
	CGroup rootGroup() const noexcept;
	Group group(const QModelIndex& index) noexcept;
	CGroup group(const QModelIndex& index) const noexcept;

	using Kdbx::DatabaseModel<QKdbxDatabase>::group;

	inline QIcon icon(CVersion entry) const noexcept{
		return icons()->icon(entry->icon);
	}
	inline QIcon icon(CGroup group) const noexcept{
		return icons()->icon(group.properties().icon);
	}

	Icons* icons() noexcept;
	const Icons* icons() const noexcept;

	void moveEntry(Entry entry, Group newParent, size_t newIndex);
	void moveGroup(Group group, Group newParent, size_t newIndex);

	inline QUndoStack* undoStack(){
		return fundoStack;
	}

protected:
	//Kdbx::Database
	// those nethods operate on ondo stack and not directly on the database.
	Kdbx::Database::Version* addVersion(Kdbx::Database::Entry* entry, Kdbx::Database::Version::Ptr version, size_t index) override;
	void removeVersion(Kdbx::Database::Entry* entry, size_t index) override;
	Kdbx::Database::Version::Ptr takeVersion(Kdbx::Database::Entry* entry, size_t index) override;
	Kdbx::Database::Entry* addEntry(Kdbx::Database::Group* group, Kdbx::Database::Entry::Ptr entry, size_t index) override;
	void removeEntry(Kdbx::Database::Group* group, size_t index) override;
	Kdbx::Database::Entry::Ptr takeEntry(Kdbx::Database::Group* group, size_t index) override;
	Kdbx::Database::Group* addGroup(Kdbx::Database::Group* parent, Kdbx::Database::Group::Ptr group, size_t index) override;
	void removeGroup(Kdbx::Database::Group* parent, size_t index) override;
	Kdbx::Database::Group::Ptr takeGroup(Kdbx::Database::Group* parent, size_t index) override;

	void setProperties(Kdbx::Database::Group* group, Kdbx::Database::Group::Properties properties) override;
	void setSettings(Kdbx::Database::Settings settings) override;


	Kdbx::Database::Ptr reset(Kdbx::Database::Ptr newDatabase)override;

	Kdbx::Icon addCustomIcon(Kdbx::CustomIcon::Ptr ptr) override;

	// those methods are used by DatabaseCommand instances to operate on database from withing
	// undo stack.
	Kdbx::Database::Version* addVersionCommand(Kdbx::Database::Entry* entry, Kdbx::Database::Version::Ptr version, size_t index);
	Kdbx::Database::Version::Ptr takeVersionCommand(Kdbx::Database::Entry* entry, size_t index);
	Kdbx::Database::Entry* addEntryCommand(Kdbx::Database::Group* group, Kdbx::Database::Entry::Ptr entry, size_t index);
	Kdbx::Database::Entry::Ptr takeEntryCommand(Kdbx::Database::Group* group, size_t index);
	Kdbx::Database::Group* addGroupCommand(Kdbx::Database::Group* parent, Kdbx::Database::Group::Ptr group, size_t index);
	bool moveGroupCommand(Kdbx::Database::Group* oldParent, size_t oldIndex, size_t count, Kdbx::Database::Group* newParent, size_t newIndex);
	Kdbx::Database::Group::Ptr takeGroupCommand(Kdbx::Database::Group* parent, size_t index);
	void setPropertiesCommand(Kdbx::Database::Group* group, Kdbx::Database::Group::Properties properties);


public:

	//QAbstractItemModel
	Qt::ItemFlags flags(const QModelIndex & index) const noexcept override;
	QVariant data(const QModelIndex & index, int role) const noexcept override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const noexcept override;
	int rowCount(const QModelIndex & parent) const noexcept override;
	int columnCount(const QModelIndex & parent) const noexcept override;
	QModelIndex index(int row, int column, const QModelIndex & parent) const noexcept override;
	QModelIndex parent(const QModelIndex & index) const noexcept override;

	Qt::DropActions supportedDropActions() const override;
	QStringList mimeTypes() const;
	QMimeData* mimeData(const QModelIndexList &indexes) const override;
	bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;

	//bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

	static QString entryString(Version entry, const char* name) noexcept;
	static XorredBuffer entryStringBuffer(Version entry, const char* name) noexcept;

signals:
	//Kdbx::Database
	void settingsChanged();

	void beginEntryAdd(Group parent, size_t index);
	void endEntryAdd(Group parent, size_t index);
	void beginEntryRemove(Group parent, size_t index);
	void endEntryRemove(Group parent, size_t index);
	void beginVersionAdd(Entry parent, size_t index);
	void endVersionAdd(Entry parent, size_t index);
	void beginVersionRemove(Entry parent, size_t index);
	void endVersionRemove(Entry parent, size_t index);

	void databaseDestroyed();

	friend class QKdbxGroup;
	friend class DatabaseCommand;
};

#endif // QKDBXDATABASE_H
