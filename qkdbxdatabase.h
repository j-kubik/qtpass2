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

class QKdbxDatabase: public QAbstractItemModel, public Kdbx::DatabaseModelCRTP<QKdbxDatabase>{
public:
	class Icons;
private:
	Q_OBJECT

	typedef Kdbx::DatabaseModelCRTP<QKdbxDatabase> Base;

	static const QString dataMimeType;

	Kdbx::Database::Ptr fdatabase;
	Icons* ficons;
	QUndoStack* fundoStack;
	bool ffrozen;
	friend class Icons;
public:


	typedef Kdbx::DatabaseModelCRTP<QKdbxDatabase>::Group Group;
	typedef Kdbx::DatabaseModelCRTP<QKdbxDatabase>::Entry Entry;
	typedef Kdbx::DatabaseModelCRTP<QKdbxDatabase>::Version Version;

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
	~QKdbxDatabase() noexcept;

	Kdbx::Database* getDatabase() const noexcept override;

	inline bool frozen() const noexcept{
		return ffrozen;
	}

	QModelIndex rootIndex() const noexcept;

	QModelIndex index(const Kdbx::Database::Group* group, int column) const noexcept;
	QModelIndex index(const Kdbx::Database::Group* group, int row, int column) const noexcept;

	using Kdbx::DatabaseModelCRTP<QKdbxDatabase>::root;

	Group group(const QModelIndex& index) noexcept;
	const Kdbx::Database::Group* group(const QModelIndex& index) const noexcept;

	using Kdbx::DatabaseModelCRTP<QKdbxDatabase>::group;
	using Kdbx::DatabaseModelCRTP<QKdbxDatabase>::entry;

	inline QIcon icon(Version entry) const noexcept{
		return icons()->icon(entry->icon);
	}

	inline QIcon icon(Group group) const noexcept{
		return icons()->icon(group.properties().icon);
	}

	Icons* icons() const noexcept;

	//ToDo: This needs re-thinking...
	void moveEntry(Entry entry, Group newParent, size_t newIndex);
	void moveGroup(Group group, Group newParent, size_t newIndex);

	inline QUndoStack* undoStack() const{
		if (ffrozen)
			return nullptr;
		return fundoStack;
	}

	void setProperties(const Kdbx::Database::Group* group, Kdbx::Database::Group::Properties::Ptr properties) override;
	void setSettings(Kdbx::Database::Settings::Ptr settings) override;
	void setTemplates(const Kdbx::Database::Group* templ, std::time_t changed = time(nullptr)) override;
	void setRecycleBin(const Kdbx::Database::Group* bin, std::time_t changed = time(nullptr)) override;

	void saveToFile(std::unique_ptr<std::ostream> stream);

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
	void setPropertiesCommand(const Kdbx::Database::Group* group, Kdbx::Database::Group::Properties::Ptr& properties);
	void swapSettingsCommand(Kdbx::Database::Settings::Ptr& settings);
	void setTemplatesCommand(const Kdbx::Database::Group* templ, std::time_t changed);
	void setRecycleBinCommand(const Kdbx::Database::Group* bin, std::time_t changed);

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

	static QString entryString(const Kdbx::Database::Version* version, const char* name) noexcept;
	static XorredBuffer entryStringBuffer(const Kdbx::Database::Version* version, const char* name) noexcept;

	static inline QString entryString(Version version, const char* name) noexcept{
		return entryString(version.get(), name);
	}

	static inline XorredBuffer entryStringBuffer(Version version, const char* name) noexcept{
		return entryStringBuffer(version.get(), name);
	}

public slots:
	void saveAs(QString filename);

private slots:
	void freeze();
	void unfreeze();

signals:
	//Kdbx::Database
	void settingsChanged();
	void databaseChanged();

	void frozenChanged(bool frozen);

	void beginGroupAdd(Group parent, size_t index);
	void endGroupAdd(Group parent, size_t index);
	void beginGroupRemove(Group parent, size_t index);
	void endGroupRemove(Group parent, size_t index);
	void beginEntryAdd(Group parent, size_t index);
	void endEntryAdd(Group parent, size_t index);
	void beginEntryRemove(Group parent, size_t index);
	void endEntryRemove(Group parent, size_t index);
	void beginVersionAdd(Entry parent, size_t index);
	void endVersionAdd(Entry parent, size_t index);
	void beginVersionRemove(Entry parent, size_t index);
	void endVersionRemove(Entry parent, size_t index);

	friend class QKdbxGroup;
	friend class DatabaseCommand;
};

#endif // QKDBXDATABASE_H
