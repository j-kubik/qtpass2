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
#include "undocommands.h"

inline Kdbx::Database::Version* DatabaseCommand::addVersion(Kdbx::Database::Entry* entry,
																   Kdbx::Database::Version::Ptr version,
																   size_t index){
	return fmodel->addVersionCommand(entry, std::move(version), index);
}

inline Kdbx::Database::Version::Ptr DatabaseCommand::takeVersion(Kdbx::Database::Entry* entry,
																		size_t index){
	return fmodel->takeVersionCommand(entry, index);
}

inline Kdbx::Database::Entry* DatabaseCommand::addEntry(Kdbx::Database::Group* group,
														Kdbx::Database::Entry::Ptr entry,
														size_t index){
	return fmodel->addEntryCommand(group, std::move(entry), index);
}

inline Kdbx::Database::Entry::Ptr DatabaseCommand::takeEntry(Kdbx::Database::Group* group,
															 size_t index){
	return fmodel->takeEntryCommand(group, index);
}

inline Kdbx::Database::Group* DatabaseCommand::addGroup(Kdbx::Database::Group* parent,
														Kdbx::Database::Group::Ptr group,
														size_t index){
	return fmodel->addGroupCommand(parent, std::move(group), index);
}

inline bool DatabaseCommand::moveGroup(Kdbx::Database::Group* oldParent,
									   size_t oldIndex,
									   size_t count,
									   Kdbx::Database::Group* newParent,
									   size_t newIndex){
	return fmodel->moveGroupCommand(oldParent, oldIndex, count, newParent, newIndex);
}

inline Kdbx::Database::Group::Ptr DatabaseCommand::takeGroup(Kdbx::Database::Group* parent,
															 size_t index){
	return fmodel->takeGroupCommand(parent, index);
}

inline void DatabaseCommand::setProperties(Kdbx::Database::Group* group,
										   Kdbx::Database::Group::Properties::Ptr& properties){
	fmodel->setPropertiesCommand(group, properties);
}

inline void DatabaseCommand::swapSettings(Kdbx::Database::Settings::Ptr& settings){
	fmodel->swapSettingsCommand(settings);
}

inline void DatabaseCommand::setTemplates(QKdbxDatabase::Group templ, std::time_t changed){
	fmodel->setTemplatesCommand(templ, changed);
}

inline void DatabaseCommand::setRecycleBin(QKdbxDatabase::Group bin, std::time_t changed){
	fmodel->setRecycleBinCommand(bin, changed);
}

inline DatabaseCommand::DatabaseCommand(QKdbxDatabase* model,
										const QString& text,
										QUndoCommand * parent) noexcept
	:QUndoCommand(text, parent),
	  fmodel(model)
{
	assert(fmodel != nullptr);
}

inline DatabaseCommand::DatabaseCommand(QKdbxDatabase* model,
										QUndoCommand * parent) noexcept
	:QUndoCommand(parent),
	  fmodel(model)
{
	assert(fmodel != nullptr);
}

//-----------------------------------------------------------------------------------

VersionUpdate::VersionUpdate(Kdbx::Database::Entry* entry,
							 Kdbx::Database::Version::Ptr version,
							 size_t index,
							 QKdbxDatabase* model,
							 QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  fversion(std::move(version)),
	  findex(index),
	  fentry(entry)
{
	assert(fversion != nullptr);
	assert(findex <= fentry->versions());
	assert(fentry != nullptr);

	auto fnd = fversion->strings.find(Kdbx::Database::Version::titleString);
	if (fnd != fversion->strings.end()){
		auto plain = fnd->second.plainString();
		setText(QObject::tr("add version '%1'.").arg(QString::fromUtf8(plain.c_str(), plain.size())));
	}else{
		setText(QObject::tr("add a version."));
	}
}

void VersionUpdate::redo(){
	assert(fversion != nullptr);
	assert(findex <= fentry->versions());
	addVersion(fentry, std::move(fversion), findex);
#ifndef NDEBUG
	fversion = nullptr;
#endif
}

void VersionUpdate::undo(){
	assert(fversion == nullptr);
	assert(findex < fentry->versions());
	fversion = takeVersion(fentry, findex);
}

//-----------------------------------------------------------------------------------

VersionTake::VersionTake(Kdbx::Database::Entry* entry,
							 size_t index,
							 QKdbxDatabase* model,
							 QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  findex(index),
	  fentry(entry)
{
	assert(fentry != nullptr);
	assert(findex < fentry->versions());

	Kdbx::Database::Version* version = fentry->version(findex);
	assert(version != nullptr); // ToDo: does this even make sense?

	auto fnd = version->strings.find(Kdbx::Database::Version::titleString);
	if (fnd != version->strings.end()){
		auto plain = fnd->second.plainString();
		setText(QObject::tr("remove version '%1'.").arg(QString::fromUtf8(plain.c_str(), plain.size())));
	}else{
		setText(QObject::tr("remove a version."));
	}
}

void VersionTake::redo(){
	assert(fversion == nullptr);
	assert(findex < fentry->versions());
	fversion = takeVersion(fentry, findex);
}

void VersionTake::undo(){
	assert(fversion != nullptr);
	assert(findex <= fentry->versions());
	addVersion(fentry, std::move(fversion), findex);
#ifndef NDEBUG
	fversion = nullptr;
#endif
}

//-----------------------------------------------------------------------------------

EntryAdd::EntryAdd(Kdbx::Database::Group* group,
				   Kdbx::Database::Entry::Ptr entry,
				   size_t index,
				   QKdbxDatabase* model,
				   QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  fentry(std::move(entry)),
	  findex(index),
	  fgroup(group)
{
	assert(fgroup != nullptr);
	assert(findex <= fgroup->entries());
	assert(fentry != nullptr);

	Kdbx::Database::Version* version = fentry->latest();
	assert(version != nullptr);
	auto fnd = version->strings.find(Kdbx::Database::Version::titleString);
	if (fnd != version->strings.end()){
		auto plain = fnd->second.plainString();
		setText(QObject::tr("add entry '%1'.").arg(QString::fromUtf8(plain.c_str(), plain.size())));
	}else{
		setText(QObject::tr("add an entry."));
	}
}

void EntryAdd::redo(){
	assert(findex <= fgroup->entries());
	assert(fentry != nullptr);
	addEntry(fgroup, std::move(fentry), findex);
#ifndef NDEBUG
	fentry = nullptr;
#endif
}

void EntryAdd::undo(){
	assert(findex < fgroup->entries());
	assert(fentry == nullptr);
	fentry = takeEntry(fgroup, findex);
}

//-----------------------------------------------------------------------------------

EntryMove::EntryMove(Kdbx::Database::Group* oldParent,
		  size_t oldIndex,
		  Kdbx::Database::Group* newParent,
		  size_t newIndex,
		  QKdbxDatabase* model,
		  QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  foldParent(oldParent),
	  fnewParent(newParent),
	  foldIndex(oldIndex),
	  fnewIndex(newIndex)
{
	assert(foldParent != nullptr);
	assert(fnewParent != nullptr);
	assert(foldIndex < foldParent->entries());
	assert(fnewIndex < fnewParent->entries());
	assert(foldParent != fnewParent || (foldIndex != fnewIndex && foldIndex+1 != fnewIndex));

	Kdbx::Database::Version* version = foldParent->entry(foldIndex)->latest();
	assert(version != nullptr);
	auto fnd = version->strings.find(Kdbx::Database::Version::titleString);
	if (fnd != version->strings.end()){
		auto plain = fnd->second.plainString();
		setText(QObject::tr("move entry '%1'.").arg(QString::fromUtf8(plain.c_str(), plain.size())));
	}
}

void EntryMove::redo(){

	Kdbx::Database::Entry::Ptr e = takeEntry(foldParent, foldIndex);
	if (foldParent == fnewParent && foldIndex < fnewIndex){
		addEntry(fnewParent, std::move(e), fnewIndex -1);
	}else{
		addEntry(fnewParent, std::move(e), fnewIndex);
	}
}

void EntryMove::undo(){
	Kdbx::Database::Entry::Ptr e = takeEntry(fnewParent, fnewIndex);
	if (foldParent == fnewParent && fnewIndex < foldIndex){
		addEntry(foldParent, std::move(e), foldIndex -1);
	}else{
		addEntry(foldParent, std::move(e), foldIndex);
	}
}


//-----------------------------------------------------------------------------------

EntryTake::EntryTake(Kdbx::Database::Group* group,
					 size_t index,
					 QKdbxDatabase* model,
					 QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  findex(index),
	  fgroup(group)
{
	assert(fgroup != nullptr);
	assert(findex < fgroup->entries());

	Kdbx::Database::Version* version = fgroup->entry(findex)->latest();
	assert(version != nullptr);
	auto fnd = version->strings.find(Kdbx::Database::Version::titleString);
	if (fnd != version->strings.end()){
		auto plain = fnd->second.plainString();
		setText(QObject::tr("remove entry '%1'.").arg(QString::fromUtf8(plain.c_str(), plain.size())));
	}else{
		setText(QObject::tr("remove an entry."));
	}
}

void EntryTake::redo(){
	assert(findex < fgroup->entries());
	assert(fentry == nullptr);
	fentry = takeEntry(fgroup, findex);
}

void EntryTake::undo(){
	assert(findex <= fgroup->entries());
	assert(fentry != nullptr);
	addEntry(fgroup, std::move(fentry), findex);
#ifndef NDEBUG
	fentry = nullptr;
#endif
}

//-----------------------------------------------------------------------------------

GroupAdd::GroupAdd(Kdbx::Database::Group* parentGroup,
				   Kdbx::Database::Group::Ptr group,
				   size_t index,
				   QKdbxDatabase* model,
				   QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  fgroup(std::move(group)),
	  findex(index),
	  fparent(parentGroup)
{
	assert(fparent != nullptr);
	assert(findex <= fparent->groups());
	assert(fgroup != nullptr);

	const std::string& name = fgroup->properties().name;
	setText(QObject::tr("add group '%1'.").arg(QString::fromUtf8(name.c_str(), name.size())));
}

void GroupAdd::redo(){
	assert(findex <= fparent->groups());
	assert(fgroup != nullptr);
	addGroup(fparent, std::move(fgroup), findex);
#ifndef NDEBUG
	fgroup = nullptr;
#endif
}

void GroupAdd::undo(){
	assert(findex < fparent->groups());
	assert(fgroup == nullptr);
	fgroup = takeGroup(fparent, findex);
}


//-----------------------------------------------------------------------------------

GroupMove::GroupMove(Kdbx::Database::Group* sourceParent,
		 size_t sourceIndex,
		 size_t count,
		 Kdbx::Database::Group* destinationParent,
		 size_t destinationIndex,
		 QKdbxDatabase* model,
		 QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  fsourceIndex(sourceIndex),
	  fcount(count),
	  fdestinationIndex(destinationIndex),
	  fsourceParent(sourceParent),
	  fdestinationParent(destinationParent)
{
	assert(sourceParent != nullptr);
	assert(sourceIndex+count <= sourceParent->groups());
	assert(destinationParent != nullptr);
	assert(destinationIndex <= destinationParent->groups());
	assert(sourceParent != destinationParent || sourceIndex > destinationIndex || sourceIndex+count < destinationIndex);

	const std::string& name = fsourceParent->group(fsourceIndex)->properties().name;
	setText(QObject::tr("move group '%1'.").arg(QString::fromUtf8(name.c_str(), name.size())));

}

void GroupMove::redo(){
	//if (fsourceParent == fdestinationParent && fsourceIndex < fdestinationIndex)
	moveGroup(fsourceParent, fsourceIndex, fcount, fdestinationParent, fdestinationIndex);
}
void GroupMove::undo(){
	if (fsourceParent == fdestinationParent){
		if (fsourceIndex < fdestinationIndex){
			moveGroup(fdestinationParent, fdestinationIndex-fcount, fcount, fsourceParent, fsourceIndex);
		}else{
			moveGroup(fdestinationParent, fdestinationIndex, fcount, fsourceParent, fsourceIndex+fcount);
		}
	}else{
		moveGroup(fdestinationParent, fdestinationIndex, fcount, fsourceParent, fsourceIndex);
	}
}

//-----------------------------------------------------------------------------------

GroupTake::GroupTake(Kdbx::Database::Group* parentGroup,
					 size_t index,
					 QKdbxDatabase* model,
					 QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  findex(index),
	  fparent(parentGroup)
{
	assert(fparent != nullptr);
	assert(findex < fparent->groups());

	const std::string& name = fparent->group(findex)->properties().name;
	setText(QObject::tr("remove group '%1'.").arg(QString::fromUtf8(name.c_str(), name.size())));
}

void GroupTake::redo(){
	assert(findex < fparent->groups());
	assert(fgroup == nullptr);
	fgroup = takeGroup(fparent, findex);
}

void GroupTake::undo(){
	assert(findex <= fparent->groups());
	assert(fgroup != nullptr);
	addGroup(fparent, std::move(fgroup), findex);
#ifndef NDEBUG
	fgroup = nullptr;
#endif
}

//-----------------------------------------------------------------------------------

GroupProperties::GroupProperties(Kdbx::Database::Group* group,
		  Kdbx::Database::Group::Properties::Ptr properties,
		  QKdbxDatabase* model,
		  QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  fgroup(group),
	  fproperties(std::move(properties))
{
	assert(group != nullptr);
	const std::string& name = group->properties().name;
	setText(QObject::tr("change proprties of '%1' group.").arg(QString::fromUtf8(name.c_str(), name.size())));
}

void GroupProperties::redo(){
	setProperties(fgroup, fproperties);
}

void GroupProperties::undo(){
	setProperties(fgroup, fproperties);
}

//-----------------------------------------------------------------------------------

DatabaseSettingsCommand::DatabaseSettingsCommand(Kdbx::Database::Settings::Ptr settings,
		  QKdbxDatabase* model,
		  QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  fsettings(std::move(settings)){
	setText(QObject::tr("change database settings."));
}


void DatabaseSettingsCommand::redo(){
	swapSettings(fsettings);
}

void DatabaseSettingsCommand::undo(){
	swapSettings(fsettings);
}

//-----------------------------------------------------------------------------------

SetRecycleBinCommand::SetRecycleBinCommand(QKdbxDatabase::Group bin,
										   std::time_t changed,
										   QKdbxDatabase* model,
										   QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  recycleBin(bin),
	  changed(changed)
{
	setText(QObject::tr("Set recycle bin"));
}

void SetRecycleBinCommand::redo(){
	QKdbxDatabase::Group tmpBin = fmodel->recycleBin();
	std::time_t tmpChanged = fmodel->recycleBinChanged();
	setRecycleBin(recycleBin, changed);
	recycleBin = tmpBin;
	changed = tmpChanged;
}

void SetRecycleBinCommand::undo(){
	QKdbxDatabase::Group tmpBin = fmodel->recycleBin();
	std::time_t tmpChanged = fmodel->recycleBinChanged();
	setRecycleBin(recycleBin, changed);
	recycleBin = tmpBin;
	changed = tmpChanged;
}

//-----------------------------------------------------------------------------------

SetTemplatesCommand::SetTemplatesCommand(QKdbxDatabase::Group bin,
										   std::time_t changed,
										   QKdbxDatabase* model,
										   QUndoCommand * parent) noexcept
	:DatabaseCommand(model, parent),
	  templates(bin),
	  changed(changed)
{
	setText(QObject::tr("Set recycle bin"));
}

void SetTemplatesCommand::redo(){
	QKdbxDatabase::Group tmpTemplates = fmodel->templates();
	std::time_t tmpChanged = fmodel->templatesChanged();
	setTemplates(templates, changed);
	templates = tmpTemplates;
	changed = tmpChanged;
}

void SetTemplatesCommand::undo(){
	QKdbxDatabase::Group tmpTemplates = fmodel->templates();
	std::time_t tmpChanged = fmodel->templatesChanged();
	setTemplates(templates, changed);
	templates = tmpTemplates;
	changed = tmpChanged;
}

//-----------------------------------------------------------------------------------

