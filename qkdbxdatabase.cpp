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
#include "qkdbxdatabase.h"
#include "undocommands.h"
#include "utils.h"

#include <QMessageBox>

#include <QMimeData>
#include <QPixmap>
#include <QBuffer>
#include <QtConcurrentRun>
#include <QFutureWatcher>

#include <exception>

using namespace Kdbx;

const QString QKdbxDatabase::dataMimeType = "application/x-keepass-uuid-list";

QKdbxDatabase::Icons::Icons(QKdbxDatabase* database) noexcept
		:QAbstractListModel(database),
		  fdatabase(database)
{
	for (size_t i=0; i<fdatabase->get()->icons(); ++i){
		const Kdbx::CustomIcon::Ptr& icon = fdatabase->get()->icon(i);
		entries.emplace(icon->uuid(), Icons::customQIcon(icon));
	}

}

void QKdbxDatabase::Icons::reset(Kdbx::Database* newDatabase){
	beginResetModel();
	for (size_t i=0; i<newDatabase->icons(); ++i){
		const Kdbx::CustomIcon::Ptr& icon = newDatabase->icon(i);
		entries.emplace(icon->uuid(), Icons::customQIcon(icon));
	}
	endResetModel();
}

void QKdbxDatabase::Icons::insertIcon(QIcon icon, Kdbx::CustomIcon::Ptr customIcon){
	entries.emplace(customIcon->uuid(), std::move(icon));
	int idx = fdatabase->get()->icons();
	beginInsertRows(QModelIndex(), idx, idx);
	fdatabase->DatabaseModel::insertIcon(std::move(customIcon));
	endInsertRows();
}

void QKdbxDatabase::Icons::insertIcon(Kdbx::CustomIcon::Ptr customIcon){
	insertIcon(customQIcon(customIcon), customIcon);
}


void QKdbxDatabase::Icons::eraseIcon(size_t index){
	beginRemoveRows(QModelIndex(), index, index);
	entries.erase(fdatabase->get()->icon(index)->uuid());
	fdatabase->DatabaseModel::eraseIcon(index);
	endRemoveRows();

}


int QKdbxDatabase::Icons::rowCount(const QModelIndex &) const{
	return entries.size();
}

QVariant QKdbxDatabase::Icons::data(const QModelIndex &index, int role) const{
	if (role == Qt::DecorationRole){
		return entries.at(fdatabase->get()->icon(index.row())->uuid());
	}
	return QVariant();
}

Uuid QKdbxDatabase::Icons::add(QIcon icon){
	return fdatabase->insertIcon(std::move(icon));
}

QIcon QKdbxDatabase::Icons::icon(Kdbx::Icon i) const noexcept{
	switch (i.type()){
	case Kdbx::Icon::Type::Custom:{
		auto result = entries.find(i.custom()->uuid());
		if (result != entries.end())
			return result->second;
	}
	case Kdbx::Icon::Type::Null:
	default:
		i = Kdbx::StandardIcon::Key;
	case Kdbx::Icon::Type::Standard:{
		return Icons::standardIcon(i.standard());
	}

	}
}

Kdbx::Icon QKdbxDatabase::Icons::kdbxIcon(const QModelIndex& index){
	if (index.isValid())
		return kdbxIcon(index.row());
	return Kdbx::Icon();
}

Kdbx::Icon QKdbxDatabase::Icons::kdbxIcon(int index){
	return Kdbx::Icon(fdatabase->get()->icon(index));
}


QIcon QKdbxDatabase::Icons::customQIcon(const Kdbx::CustomIcon::Ptr& customIcon){
	QPixmap p;
	const std::vector<uint8_t>& data = customIcon->data();
	if (p.loadFromData(data.data(), data.size(), "PNG", Qt::AutoColor| Qt::NoOpaqueDetection) == false){
		QByteArray ar = tr("Failed conversion of icon data.").toUtf8();
		throw std::runtime_error(std::string(ar.data(), ar.size()));
	}
	return QIcon(p);
}

QIcon QKdbxDatabase::Icons::standardIcon(Kdbx::StandardIcon icon) noexcept{
	unsigned int iconId = (unsigned int)icon;
	QString resourceName = ":/kdbx/icons/%1/C%2.png";

	QIcon result = QIcon(resourceName.arg("16x16").arg(iconId, 2, 10, QChar('0')));
	result.addFile(resourceName.arg("HighRes").arg(iconId, 2, 10, QChar('0')), QSize(48,48));

	return result;
}

Kdbx::CustomIcon::Ptr QKdbxDatabase::Icons::customIcon(QIcon icon){
	QPixmap p(icon.pixmap(icon.actualSize(QSize(64,64))));
	QBuffer buffer;
	if (p.save(&buffer, "PNG") == false){
		QByteArray errMsg = tr("Failed saving icon to database.").toUtf8();
		throw std::runtime_error(std::string(errMsg.data(), errMsg.size()));
	}

	QByteArray& buf = buffer.buffer();
	return Kdbx::CustomIcon::Ptr(new Kdbx::CustomIcon(Uuid::generate(), std::vector<uint8_t>(buf.begin(), buf.end())));
}

//---------------------------------------------------------------------------------------

std::exception_ptr QKdbxDatabase::saveAsAsync(std::shared_ptr<Kdbx::Database> database, QString filename){
	try{
		database->saveToFile(utf8QString(filename));
	} catch(...){
		return std::current_exception();
	}
	return std::exception_ptr();
}

QKdbxDatabase::QKdbxDatabase(Kdbx::Database::Ptr database, QObject* parent)
	:QAbstractItemModel(parent),
	  fdatabase(database.release()),
	  ficons(new Icons(this)),
	  fundoStack(new QUndoStack(this)),
	  finsertIconsCommand(nullptr),
	  ffreezeCount(0)
{}

QKdbxDatabase::~QKdbxDatabase() noexcept{}

Kdbx::Database* QKdbxDatabase::getDatabase() const noexcept{
	return fdatabase.get();
}

QModelIndex QKdbxDatabase::rootIndex(int column) const noexcept{
	return index(get()->root(), 0, column);
}

QModelIndex QKdbxDatabase::index(const Kdbx::Database::Group* group, int column) const noexcept{
	if (!group)
		return QModelIndex();

	if (!group->parent())
		return index(group, 0, column);

	return index(group, group->index(), column);
}

QModelIndex QKdbxDatabase::index(const Kdbx::Database::Group* group, int row, int column) const noexcept{
	assert(group != nullptr);
	return createIndex(row, column, const_cast<Kdbx::Database::Group*>(group));
}

QKdbxDatabase::Group QKdbxDatabase::group(const QModelIndex& index) noexcept{
	if (index.isValid())
		return group(index.internalPointer());
	return Group();
}

const Kdbx::Database::Group* QKdbxDatabase::group(const QModelIndex& index) const noexcept{
	if (index.isValid())
		return group(index.internalPointer());
	return nullptr;
}

QKdbxDatabase::Icons* QKdbxDatabase::icons() const noexcept{
	return ficons;
}

void QKdbxDatabase::moveEntry(Entry entry, Group newParent, size_t newIndex){
	assert(entry);
	assert(entry.parent());
	assert(newParent);
	assert(newIndex <= newParent.entries());
	size_t idx = entry.index();
	assert(entry.parent() != newParent || (idx != newIndex  && idx+1 != newIndex));
	fundoStack->push(new EntryMove(entry.parent().item(), idx, newParent.item(), newIndex, this));
}

void QKdbxDatabase::moveGroup(Group group, Group newParent, size_t newIndex){
	assert(group);
	assert(group.parent());
	assert(newParent);
	assert(newIndex <= newParent.groups());
	size_t idx = group.index();
	assert(group.parent() != newParent || (idx != newIndex && idx+1 != newIndex));
	assert(!newParent->ancestor(group.get()));

	fundoStack->push(new GroupMove(group.parent().item(), group.index(), newParent.item(), newIndex, this));
}

void QKdbxDatabase::setProperties(const Kdbx::Database::Group* group, Kdbx::Database::Group::Properties::Ptr properties){
	finsertIconsCommand = new GroupProperties(group, std::move(properties), this);
	fundoStack->push(finsertIconsCommand);
	finsertIconsCommand = nullptr;
}

void QKdbxDatabase::setSettings(Kdbx::Database::Settings::Ptr settings){
	fundoStack->push(new DatabaseSettingsCommand(std::move(settings), this));
}

void QKdbxDatabase::setTemplates(const Kdbx::Database::Group* templ, std::time_t changed){
	if (templ != get()->templates())
		fundoStack->push(new SetTemplatesCommand(templ, changed, this));
}

void QKdbxDatabase::setRecycleBin(const Kdbx::Database::Group* bin, std::time_t changed){
	if (bin != get()->recycleBin())
		fundoStack->push(new SetRecycleBinCommand(bin, changed, this));
}


Kdbx::Database::Version* QKdbxDatabase::addVersion(Kdbx::Database::Entry* entry, Kdbx::Database::Version::Ptr version, size_t index){
	VersionUpdate* result = new VersionUpdate(entry, std::move(version), index, this);
	finsertIconsCommand = result;
	fundoStack->push(result);
	finsertIconsCommand = nullptr;
	assert(result->addedVersion() != nullptr);
	return result->addedVersion();
}

void QKdbxDatabase::removeVersion(Kdbx::Database::Entry* entry, size_t index){
	fundoStack->push(new VersionTake(entry, index, this));
}

Kdbx::Database::Version::Ptr QKdbxDatabase::takeVersion(Kdbx::Database::Entry* entry, size_t index){
	VersionTake* result = new VersionTake(entry, index, this);
	fundoStack->push(result);
	assert(result->version() != nullptr);
	return result->versionCopy();
}

Kdbx::Database::Entry* QKdbxDatabase::addEntry(Kdbx::Database::Group* group, Kdbx::Database::Entry::Ptr entry, size_t index){
	EntryAdd* result = new EntryAdd(group, std::move(entry), index, this);
	finsertIconsCommand = result;
	fundoStack->push(result);
	finsertIconsCommand = nullptr;
	assert(result->addedEntry() != nullptr);
	return result->addedEntry();
}

void QKdbxDatabase::removeEntry(Kdbx::Database::Group* group, size_t index){
	if (get()->settings().recycleBinEnabled &&
		get()->recycleBin() &&
		get()->recycleBin() != group){
		fundoStack->push(new EntryMove(group, index, getDatabase()->recycleBin(), getDatabase()->recycleBin()->entries(), this));
	}else{
		fundoStack->push(new EntryTake(group, index, this));
	}
}

Kdbx::Database::Entry::Ptr QKdbxDatabase::takeEntry(Kdbx::Database::Group* group, size_t index){
	EntryTake* result  = new EntryTake(group, index, this);
	fundoStack->push(result);
	return result->entryCopy();
}


Kdbx::Database::Group* QKdbxDatabase::addGroup(Kdbx::Database::Group* parent, Kdbx::Database::Group::Ptr group, size_t index){
	GroupAdd* result = new GroupAdd(parent, std::move(group), index, this);
	finsertIconsCommand = result;
	fundoStack->push(result);
	finsertIconsCommand = nullptr;
	assert(result->addedGroup() != nullptr);
	return result->addedGroup();
}

void QKdbxDatabase::removeGroup(Kdbx::Database::Group* parent, size_t index){
	fundoStack->push(new GroupTake(parent, index, this));
}

Kdbx::Database::Group::Ptr QKdbxDatabase::takeGroup(Kdbx::Database::Group* parent, size_t index){
	GroupTake* result = new GroupTake(parent, index, this);
	fundoStack->push(result);
	return result->groupCopy();
}

void QKdbxDatabase::insertIcon(Kdbx::CustomIcon::Ptr icon){
	if (finsertIconsCommand){
		finsertIconsCommand->addIcon(std::move(icon));
	}else{
		finsertIconsCommand = new InsertIcons(this);
		finsertIconsCommand->addIcon(std::move(icon));
		fundoStack->push(finsertIconsCommand);
		finsertIconsCommand = nullptr;
	}
}

void QKdbxDatabase::eraseIcon(size_t index){
#warning This needs to become an action too...
	ficons->eraseIcon(index);
}

Uuid QKdbxDatabase::insertIcon(QIcon icon){
	Uuid result;
	if (finsertIconsCommand){
		result = finsertIconsCommand->addIcon(std::move(icon));
	}else{
		finsertIconsCommand = new InsertIcons(this);
		result = finsertIconsCommand->addIcon(std::move(icon));
		fundoStack->push(finsertIconsCommand);
		finsertIconsCommand = nullptr;
	}
	return result;
}


Kdbx::Database::Version* QKdbxDatabase::addVersionCommand(Kdbx::Database::Entry* entry, Kdbx::Database::Version::Ptr version, size_t index){
	emit beginVersionAdd(this->entry(entry), index);
	Kdbx::Database::Version* result = DatabaseModelCRTP::addVersion(entry, std::move(version), index);
	emit endVersionAdd(this->entry(entry), index);
	return result;
}

Kdbx::Database::Version::Ptr QKdbxDatabase::takeVersionCommand(Kdbx::Database::Entry* entry, size_t index){
	emit beginVersionRemove(this->entry(entry), index);
	Kdbx::Database::Version::Ptr result = DatabaseModelCRTP::takeVersion(entry, index);
	emit endVersionRemove(this->entry(entry), index);
	return result;
}

Kdbx::Database::Entry* QKdbxDatabase::addEntryCommand(Kdbx::Database::Group* group, Kdbx::Database::Entry::Ptr entry, size_t index){
	emit beginEntryAdd(this->group(group), index);
	Kdbx::Database::Entry* result = Base::addEntry(group, std::move(entry), index);
	emit endEntryAdd(this->group(group), index);
	QModelIndex idx = this->index(group, 1);
	emit dataChanged(idx, idx, QVector<int>(Qt::DisplayRole));
	return result;
}

void QKdbxDatabase::moveEntryCommand(Kdbx::Database::Group* oldParent, size_t oldIndex, Kdbx::Database::Group* newParent, size_t newIndex){
	emit beginEntryMove(group(oldParent), oldIndex, group(newParent), newIndex);
	Base::moveEntry(oldParent, oldIndex, newParent, newIndex);
	emit endEntryMove(group(oldParent), oldIndex, group(newParent), newIndex);
}

Kdbx::Database::Entry::Ptr QKdbxDatabase::takeEntryCommand(Kdbx::Database::Group* group, size_t index){
	emit beginEntryRemove(this->group(group), index);
	Kdbx::Database::Entry::Ptr result = Base::takeEntry(group, index);
	emit endEntryRemove(this->group(group), index);
	QModelIndex idx = this->index(group, 1);
	emit dataChanged(idx, idx, QVector<int>(Qt::DisplayRole));
	return result;
}

Kdbx::Database::Group* QKdbxDatabase::addGroupCommand(Kdbx::Database::Group* parent, Kdbx::Database::Group::Ptr group, size_t idx){
	emit beginInsertRows(index(parent,0), idx, idx);
	Kdbx::Database::Group* result = Base::addGroup(parent, std::move(group), idx);
	emit endInsertRows();
	return result;
}

void QKdbxDatabase::moveGroupCommand(Kdbx::Database::Group* oldParent, size_t oldIndex, Kdbx::Database::Group* newParent, size_t newIndex){
	beginMoveRows(index(oldParent, 0), oldIndex, oldIndex, index(newParent, 0), newIndex);
	Base::moveGroup(oldParent, oldIndex, newParent, newIndex);
	endMoveRows();
}


Kdbx::Database::Group::Ptr QKdbxDatabase::takeGroupCommand(Kdbx::Database::Group* parent, size_t idx){
	emit beginRemoveRows(index(parent, 0), idx, idx);
	Kdbx::Database::Group::Ptr result = DatabaseModelCRTP::takeGroup(parent, idx);
	emit endRemoveRows();
	return result;
}

void QKdbxDatabase::setPropertiesCommand(const Kdbx::Database::Group* group, Kdbx::Database::Group::Properties::Ptr& properties){
	DatabaseModelCRTP::swapProperties(group, properties);
	emit dataChanged(index(group, 0), index(group, columnCount(QModelIndex())-1));
	emit groupPropertiesChanged(this->group(group));
}

void QKdbxDatabase::swapSettingsCommand(Kdbx::Database::Settings::Ptr& settings){
	DatabaseModelCRTP::swapSettings(settings);
	emit settingsChanged();
}

void QKdbxDatabase::setTemplatesCommand(const Kdbx::Database::Group* templ, std::time_t changed){
	DatabaseModelCRTP::setTemplates(templ, changed);
	emit settingsChanged();
}

void QKdbxDatabase::setRecycleBinCommand(const Kdbx::Database::Group* bin, std::time_t changed){
	DatabaseModelCRTP::setRecycleBin(bin, changed);
	emit settingsChanged();
}

Qt::ItemFlags QKdbxDatabase::flags(const QModelIndex & index) const noexcept{

	Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	if (index.isValid()){
		result |= Qt::ItemIsDropEnabled;

		const Kdbx::Database::Group* g = group(index);
		if (g->parent()){
			result |= Qt::ItemIsDragEnabled;
		}

		if (index.column() == 0)
			result |= Qt::ItemIsEditable;
	}
	return result;
}

QVariant QKdbxDatabase::data(const QModelIndex & index, int role) const noexcept{

	//if (role == Qt::DisplayRole)
	//	std::cout << "data: " << index.row() << ", " << index.column() << ", "<< index.internalPointer() << std::endl;

	if (!index.isValid())
		return QVariant();

	const Kdbx::Database::Group* g = group(index);

	switch (role){
	case Qt::EditRole:
		if (index.column() == 0 && g->parent())
			return QString::fromUtf8(g->properties().name.c_str());
		break;

	case Qt::DisplayRole:
		switch (index.column()){
		default:
		case 0:
			if (g->parent())
				return QString::fromUtf8(g->properties().name.c_str());
			return tr("base");
		case 1:
			return QString("%1").arg(g->entries());
		}
		break;

	case Qt::DecorationRole:
		if (index.column() == 0){
			return icons()->icon(g->properties().icon);
		}
		break;

	default:
		break;
	}

	return QVariant();

}

bool QKdbxDatabase::setData(const QModelIndex &index, const QVariant &value, int role){
	if (!index.isValid() || !value.canConvert<QString>() || role != Qt::EditRole)
		return false;

	QByteArray val = value.value<QString>().toUtf8();

	Group g = group(index);
	if (!g->parent())
		return false;
	Kdbx::Database::Group::Properties::Ptr properties(new Kdbx::Database::Group::Properties(g->properties()));
	properties->name = std::string(val.data(), val.size());
	g.setProperties(std::move(properties));
	return true;
}


QVariant QKdbxDatabase::headerData(int section, Qt::Orientation orientation, int role) const noexcept{
	if (role != Qt::DisplayRole || section > 1)
		return QAbstractItemModel::headerData(section, orientation, role);

	switch (section){
	default:
	case 0:
		return tr("Group");
	case 1:
		return tr("Item count");

	}
}

int QKdbxDatabase::rowCount(const QModelIndex & parent) const noexcept{
	//std::cout << "rowCount: " << parent.row() << ", " << parent.column() << ", "<< parent.internalPointer() << std::endl;
	if (!parent.isValid()) return 1;
	const Kdbx::Database::Group* g = group(parent);
	if (!g) return 1;
	return g->groups();
}

int QKdbxDatabase::columnCount(const QModelIndex & parent) const noexcept{
	unused(parent);
	return 2;
}

QModelIndex QKdbxDatabase::index(int row, int column, const QModelIndex & parent) const noexcept{
	//std::cout << "index("<< row << ", " << column << "): " << parent.row() << ", " << parent.column() << ", "<< parent.internalPointer() << std::endl;
	if (!parent.isValid()){
		if (row == 0 && column < 2)
			return index(get()->root(), row, column);
		return QModelIndex();
	}

	const Kdbx::Database::Group* g = group(parent);
	if (column > 1 || row >= int(g->groups()))
		return QModelIndex();

	return index(g->group(row), row, column);
}

QModelIndex QKdbxDatabase::parent(const QModelIndex & idx) const noexcept{
	//std::cout << "parent: " << idx.row() << ", " << idx.column() << ", "<< idx.internalPointer() << std::endl;
	if (!idx.isValid())
		return QModelIndex();

	const Kdbx::Database::Group* g = group(idx);
	const Kdbx::Database::Group*  parent = g->parent();
	if (!parent)
		return QModelIndex();

	return index(parent, 0);
}

Qt::DropActions QKdbxDatabase::supportedDropActions() const{
	return Qt::MoveAction;
}

QStringList QKdbxDatabase::mimeTypes() const{
	return QStringList(dataMimeType);
}

QMimeData* QKdbxDatabase::mimeData(const QModelIndexList &indexes) const{
	QByteArray resultData;
	QBuffer buffer(&resultData);
	if (!buffer.open(QIODevice::WriteOnly))
		return 0;

	for (const QModelIndex& index: indexes){
		if (!index.isValid() || index.column() != 0)
			continue;
		const Kdbx::Database::Group* g = group(index);
		std::array<uint8_t, 16> uuid = g->uuid().raw();
		buffer.write(reinterpret_cast<const char*>(uuid.data()), uuid.size());
	}

	QMimeData* result = new QMimeData();
	result->setData(dataMimeType, resultData);
	return result;
}

bool QKdbxDatabase::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent){
	unused(column);
	Group p = group(parent);
	if (!p || action != Qt::MoveAction)
		return false;

//	Group insertBefore;
//	if (row >= 0 && row < p.groups()){
//	    insertBefore = p.group(row);
//	}

	if (!data || !data->hasFormat(dataMimeType))
		return false;

	QByteArray rawData = data->data(dataMimeType);
	QBuffer buffer(&rawData);
	if (!buffer.open(QIODevice::ReadOnly))
		return false;

	std::set<Group> movedGroups;
	std::set<Entry> movedEntries;

	std::array<uint8_t, 16> raw;
	while (buffer.read(reinterpret_cast<char*>(raw.data()), raw.size()) == raw.size()){

		Uuid uuid(raw);
		Group g = group(uuid);
		if (g){
			if (!g.parent() || p->ancestor(g.get()))
				continue;

			if (g.parent() == p){
				size_t idx = g.index();
				if (row < 0 || idx == size_t(row) || idx == size_t(row+1))
					continue;
			}

			movedGroups.insert(g);
		}

		Entry e = entry(uuid);
		if (e){
			if (e.parent() == p)
				continue;
			movedEntries.insert(e);
		}

		//const std::string& name = g->properties().name;
		//QMessageBox::information(0, tr("Mime Drop %1").arg(int(action)), QString::fromUtf8(name.c_str(), name.size()));
	}

	if (movedGroups.size() && movedEntries.size()){
		fundoStack->beginMacro(QString("Move %1 elements.").arg(movedGroups.size() + movedEntries.size()));
	}else if (movedGroups.size() > 1){
		fundoStack->beginMacro(QString("Move %1 groups.").arg(movedGroups.size()));
	}else if (movedEntries.size() > 1){
		fundoStack->beginMacro(QString("Move %1 entries.").arg(movedEntries.size()));
	}

	if (row < 0)
		row = p.groups();

	for (const Group& g: movedGroups){
		moveGroup(g, p, row++);
		if (g.parent() != p || g.index() > size_t(row))
			row++;
	}

	for (const Entry& e: movedEntries){
		moveEntry(e, p, p.entries());
	}

	if (movedGroups.size() + movedEntries.size() > 1){
		fundoStack->endMacro();
	}

	return true;
}

QString QKdbxDatabase::entryString(const Kdbx::Database::Version* version, const char* name) noexcept{
	SafeString<char> tmp = entryStringBuffer(version, name).plainString();
	return QString::fromUtf8(tmp.c_str(), tmp.size());
}

Kdbx::XorredBuffer QKdbxDatabase::entryStringBuffer(const Kdbx::Database::Version* version, const char* name) noexcept{
	auto pos = version->strings.find(name);
	if (pos != version->strings.end()){
		return pos->second;
	}
	return Kdbx::XorredBuffer();
}

void QKdbxDatabase::saveAs(QString filename){
	QFutureWatcher<std::exception_ptr>* watcher = new QFutureWatcher<std::exception_ptr>(this);
	connect(watcher, &QFutureWatcher<std::exception_ptr>::finished, this, &QKdbxDatabase::onFutureFinished);
	connect(watcher, &QFutureWatcher<std::exception_ptr>::finished, watcher, &QObject::deleteLater);

	bool thawed = !frozen();
	++ffreezeCount;
	watcher->setFuture(QtConcurrent::run(&QKdbxDatabase::saveAsAsync, fdatabase, filename));

	if (thawed && frozen())
		emit frozenChanged(true);
}


void QKdbxDatabase::onFutureFinished(){
	assert(ffreezeCount > 0);
	--ffreezeCount;

	if (ffreezeCount == 0)
		emit frozenChanged(false);

	auto future = dynamic_cast<QFutureWatcher<std::exception_ptr>*>(sender());
	if (!future)
		return;

	std::exception_ptr excpt = future->result();
	if (!excpt)
		return;

	try{
		std::rethrow_exception(excpt);
	}catch(std::exception& e){
		emit saveError(QString::fromUtf8(e.what()));
	}

}





