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
#ifndef UNDOCOMMANDS_H
#define UNDOCOMMANDS_H

#include <QUndoCommand>
#include "qkdbxdatabase.h"

class DatabaseCommand: public QUndoCommand{
protected:

	inline Kdbx::Database::Version* addVersion(Kdbx::Database::Entry* entry,
													  Kdbx::Database::Version::Ptr version,
													  size_t index);
	inline Kdbx::Database::Version::Ptr takeVersion(Kdbx::Database::Entry* entry,
														   size_t index);
	inline Kdbx::Database::Entry* addEntry(Kdbx::Database::Group* group,
										   Kdbx::Database::Entry::Ptr entry,
										   size_t index);
	inline void moveEntry(Kdbx::Database::Group* oldParent,
						  size_t oldIndex,
						  Kdbx::Database::Group* newParent,
						  size_t newIndex);
	inline Kdbx::Database::Entry::Ptr takeEntry(Kdbx::Database::Group* group,
												size_t index);
	inline Kdbx::Database::Group* addGroup(Kdbx::Database::Group* parent,
										   Kdbx::Database::Group::Ptr group,
										   size_t index);
	inline void moveGroup(Kdbx::Database::Group* oldParent,
						  size_t oldIndex,
						  Kdbx::Database::Group* newParent,
						  size_t newIndex);
	inline Kdbx::Database::Group::Ptr takeGroup(Kdbx::Database::Group* parent,
												size_t index);
	inline void setProperties(const Kdbx::Database::Group* group,
							  Kdbx::Database::Group::Properties::Ptr& properties);
	inline void swapSettings(Kdbx::Database::Settings::Ptr& settings);
	inline void setTemplates(const Kdbx::Database::Group* templ, std::time_t changed);
	inline void setRecycleBin(const Kdbx::Database::Group* bin, std::time_t changed);

	QKdbxDatabase* fmodel;
public:

	inline DatabaseCommand(QKdbxDatabase* model,
						  const QString& text,
						  QUndoCommand * parent = 0) noexcept;
	inline DatabaseCommand(QKdbxDatabase* model,
						  QUndoCommand * parent = 0) noexcept;
};

class InsertIcons: public DatabaseCommand{
protected:
	std::vector<std::pair<QIcon, Kdbx::CustomIcon::Ptr>> icons;
public:
	InsertIcons(QKdbxDatabase* model,
			 QUndoCommand * parent = 0) noexcept;

	InsertIcons(std::vector<std::pair<QIcon, Kdbx::CustomIcon::Ptr>> icons,
			 QKdbxDatabase* model,
			 QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;

	void addIcon(Kdbx::CustomIcon::Ptr icon);
	Kdbx::Uuid addIcon(QIcon qicon);

};

class VersionUpdate: public InsertIcons{
private:
	Kdbx::Database::Version::Ptr fversion;
	size_t findex;
	Kdbx::Database::Entry* fentry;

public:

	VersionUpdate(Kdbx::Database::Entry* entry,
				  Kdbx::Database::Version::Ptr version,
				  size_t index,
				  QKdbxDatabase* model,
				  QUndoCommand * parent = 0) noexcept;

	inline Kdbx::Database::Version* addedVersion() const noexcept{
		return fentry->version(findex);
	}

	void redo() override;
	void undo() override;
};

class VersionTake: public DatabaseCommand{
private:
	Kdbx::Database::Version::Ptr fversion;
	size_t findex;
	Kdbx::Database::Entry* fentry;

public:

	VersionTake(Kdbx::Database::Entry* entry,
				  size_t index,
				  QKdbxDatabase* model,
				  QUndoCommand * parent = 0) noexcept;

	inline Kdbx::Database::Version* addedVersion() const noexcept{
		return fentry->version(findex);
	}

	inline const Kdbx::Database::Version::Ptr& version() const noexcept{
		return fversion;
	}

	inline Kdbx::Database::Version::Ptr versionCopy() const noexcept{
		return Kdbx::Database::Version::Ptr(new Kdbx::Database::Version(*fversion.get()));
	}

	void redo() override;
	void undo() override;
};

class EntryAdd: public InsertIcons{
private:
	Kdbx::Database::Entry::Ptr fentry;
	size_t findex;
	Kdbx::Database::Group* fgroup;

public:

	EntryAdd(Kdbx::Database::Group* group,
			 Kdbx::Database::Entry::Ptr entry,
			 size_t index,
			 QKdbxDatabase* model,
			 QUndoCommand * parent = 0) noexcept;

	inline Kdbx::Database::Entry* addedEntry() const noexcept{
		return fgroup->entry(findex);
	}

	void redo() override;
	void undo() override;
};

class EntryMove: public DatabaseCommand{
private:
	Kdbx::Database::Group* foldParent;
	Kdbx::Database::Group* fnewParent;
	size_t foldIndex;
	size_t fnewIndex;
public:

	EntryMove(Kdbx::Database::Group* oldParent,
			  size_t oldIndex,
			  Kdbx::Database::Group* newParent,
			  size_t newIndex,
			  QKdbxDatabase* model,
			  QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;
};

class EntryTake: public DatabaseCommand{
private:
	Kdbx::Database::Entry::Ptr fentry;
	size_t findex;
	Kdbx::Database::Group* fgroup;

public:

	EntryTake(Kdbx::Database::Group* group,
			  size_t index,
			  QKdbxDatabase* model,
			  QUndoCommand * parent = 0) noexcept;

	inline Kdbx::Database::Entry* addedEntry() const noexcept{
		return fgroup->entry(findex);
	}

	inline const Kdbx::Database::Entry::Ptr& entry() const noexcept{
		return fentry;
	}

	inline Kdbx::Database::Entry::Ptr entryCopy() const noexcept{
		return Kdbx::Database::Entry::Ptr(new Kdbx::Database::Entry(*fentry.get()));
	}

	void redo() override;
	void undo() override;
};

class GroupAdd: public InsertIcons{
private:
	Kdbx::Database::Group::Ptr fgroup;
	size_t findex;
	Kdbx::Database::Group* fparent;

public:
	GroupAdd(Kdbx::Database::Group* parentGroup,
			 Kdbx::Database::Group::Ptr group,
			 size_t index,
			 QKdbxDatabase* model,
			 QUndoCommand * parent = 0) noexcept;

	inline Kdbx::Database::Group* addedGroup() const noexcept{
		return fparent->group(findex);
	}

	void redo() override;
	void undo() override;
};

class GroupMove: public DatabaseCommand{
private:
	size_t foldIndex;
	size_t fnewIndex;
	Kdbx::Database::Group* foldParent;
	Kdbx::Database::Group* fnewParent;

public:

	GroupMove(Kdbx::Database::Group* oldParent,
			 size_t oldIndex,
			 Kdbx::Database::Group* newParent,
			 size_t newIndex,
			 QKdbxDatabase* model,
			 QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;
};

class GroupTake: public DatabaseCommand{
private:
	Kdbx::Database::Group::Ptr fgroup;
	size_t findex;
	size_t fcount;
	Kdbx::Database::Group* fparent;

public:

	GroupTake(Kdbx::Database::Group* parentGroup,
			  size_t index,
			  QKdbxDatabase* model,
			  QUndoCommand * parent = 0) noexcept;

	inline Kdbx::Database::Group* addedGroup() const noexcept{
		return fgroup->group(findex);
	}

	inline const Kdbx::Database::Group::Ptr& group() const noexcept{
		return fgroup;
	}

	inline Kdbx::Database::Group::Ptr groupCopy() const noexcept{
		return Kdbx::Database::Group::Ptr(new Kdbx::Database::Group(*fgroup));
	}

	void redo() override;
	void undo() override;
};

class GroupProperties: public InsertIcons{
private:
	const Kdbx::Database::Group* fgroup;
	Kdbx::Database::Group::Properties::Ptr fproperties;
public:

	GroupProperties(const Kdbx::Database::Group* group,
			  Kdbx::Database::Group::Properties::Ptr properties,
			  QKdbxDatabase* model,
			  QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;
};

class DatabaseSettingsCommand: public DatabaseCommand{
private:
	Kdbx::Database::Settings::Ptr fsettings;
public:

	DatabaseSettingsCommand(Kdbx::Database::Settings::Ptr settings,
							QKdbxDatabase* model,
							QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;
};

class SetRecycleBinCommand: public DatabaseCommand{
private:
	const Kdbx::Database::Group* recycleBin;
	std::time_t changed;
public:

	SetRecycleBinCommand(const Kdbx::Database::Group* bin,
						 std::time_t changed,
						 QKdbxDatabase* model,
						 QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;
};

class SetTemplatesCommand: public DatabaseCommand{
private:
	const Kdbx::Database::Group* templates;
	std::time_t changed;
public:

	SetTemplatesCommand(const Kdbx::Database::Group* templates,
						std::time_t changed,
						QKdbxDatabase* model,
						QUndoCommand * parent = 0) noexcept;

	void redo() override;
	void undo() override;
};

#endif // UNDOCOMMANDS_H
