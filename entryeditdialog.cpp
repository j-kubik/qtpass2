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
#include "entryeditdialog.h"
#include "ui_entryeditdialog.h"

#include "passwordchangedialog.h"
#include "utils.h"

#include "qkdbxgroup.h"
#include "qkdbxdatabase.h"

#include "icondialog.h"

#include <QToolTip>
#include <QFileDialog>
#include <QColorDialog>
#include <QMessageBox>
#include <QStringListModel>

#include <fstream>

class BinariesModel: public QAbstractTableModel{
private:
	std::vector<std::pair<QString, std::shared_ptr<SafeVector<uint8_t>>>> items;

	static QString sizeHumanReadable(std::size_t size){
		if (size < 1024)
			return QString("%1 B").arg(size);
		if (size < 1024*1024)
			return QString("%1 KB").arg(double(size)/1024, 0, 'f', 1);
		if (size < 1024*1024*1024)
			return QString("%1 MB").arg(double(size)/(1024*1024), 0, 'f', 1);
		return QString("%1 GB").arg(double(size)/(1024*1024*1024), 0, 'f', 1);
	}

public:

	BinariesModel(const std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>>& its, QObject* parent)
		:QAbstractTableModel(parent)
	{
		items.reserve(its.size());
		for (const std::pair<std::string, std::shared_ptr<SafeVector<uint8_t>>>& item: its){
			items.emplace_back(
				QString::fromUtf8(item.first.c_str(), item.first.size()),
				item.second);
		}
	}

	BinariesModel(QObject* parent)
		:QAbstractTableModel(parent)
	{}

	std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>> takeBianaries(){
		std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>> result;
		beginResetModel();
		for (std::pair<QString, std::shared_ptr<SafeVector<uint8_t>>>& item: items){
			QByteArray name = item.first.toUtf8();
			result.emplace(std::string(name.data(), name.size()), std::move(item.second));
		}
		items.clear();
		endResetModel();

		return result;
	}

	QModelIndex insertFile(QString filename, std::shared_ptr<SafeVector<uint8_t>> data){
		items.reserve(items.size() + 1);
		size_t index = items.size();
		beginInsertRows(QModelIndex(), index, index);
		items.emplace_back(std::move(filename), std::move(data));
		endInsertRows();
		return createIndex(index, 0);
	}

	QString getFilename(const QModelIndex& index) const{
		if (index.isValid()){
			return items.at(index.row()).first;
		}
		return QString();
	}

	std::shared_ptr<SafeVector<uint8_t>> getFile(const QModelIndex& index) const{
		if (index.isValid()){
			return items.at(index.row()).second;
		}
		return std::shared_ptr<SafeVector<uint8_t>>();
	}

	void eraseFile(const QModelIndex& index){
		if (!index.isValid())
			return;
		beginRemoveRows(QModelIndex(), index.row(), index.row());
		items.erase(items.begin()+index.row());
		endRemoveRows();
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override{
		if (parent.isValid())
			return 0;
		return items.size();
	}

	int columnCount(const QModelIndex &parent = QModelIndex()) const override{
		if (parent.isValid())
			return 0;
		return 2;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override{
		if (!index.isValid())
			return QVariant();

		switch (index.column()){
		case 0:
			switch (role){
			case Qt::EditRole:
				if (items.at(index.row()).first.isEmpty())
					return tr("Set filename...");
			case Qt::DisplayRole:
				return items.at(index.row()).first;

			case Qt::ForegroundRole:
				if (items.at(index.row()).first.isEmpty())
					return QBrush(QApplication::style()->standardPalette().color(QPalette::Disabled, QPalette::Text));
			default:
				return QVariant();
			}

		case 1:
			if (role == Qt::DisplayRole)
				return sizeHumanReadable(items.at(index.row()).second->size());

		default:
			return  QVariant();
		}
	}

	QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override{
		if (role != Qt::DisplayRole)
			return QVariant();

		switch (section){
		case 0:
			return tr("Filename");
		case 1:
			return tr("Size");
		default:
			return QVariant();
		}
	}

	Qt::ItemFlags flags(const QModelIndex &index) const override{
		Qt::ItemFlags flags = QAbstractTableModel::flags(index);
		if (index.isValid() && index.column() == 0)
			flags |= Qt::ItemIsEditable;
		return flags;
	}

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole){
		if (!index.isValid() || index.column() != 0 || !value.canConvert<QString>() || role != Qt::EditRole)
			return false;
		items.at(index.row()).first = value.toString();
		emit dataChanged(index, index);
		return true;
	}

};

class TagsModel: public QAbstractListModel{
private:
	QStringList items;

public:

	inline TagsModel(const std::vector<std::string>& tags, QObject* parent)
		:QAbstractListModel(parent)
	{
		for (const std::string& tag: tags){
			items.push_back(QString::fromUtf8(tag.c_str(), tag.size()));
		}
	}

	inline TagsModel(QObject* parent)
		:QAbstractListModel(parent)
	{}

	std::vector<std::string> takeTags(){
		std::vector<std::string> result;
		beginResetModel();
		for (const QString& item: items){
			QByteArray tag = item.toUtf8();
			result.emplace_back(tag.data(), tag.size());
		}
		items.clear();
		endResetModel();
		return result;
	}

	QModelIndex add(QString tag = QString()){
		size_t index = items.size();
		items.reserve(index + 1);
		beginInsertRows(QModelIndex(), index, index);
		items.push_back(tag);
		endInsertRows();
		return createIndex(index, 0);
	}

	void remove(const QModelIndex& index){
		beginRemoveRows(QModelIndex(), index.row(), index.row());
		items.removeAt(index.row());
		endRemoveRows();
	}

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override{
		if (role != Qt::EditRole || !value.canConvert<QString>())
			return false;

		QString val = value.toString();
		if (!QRegExp("[a-zA-Z0-9]+").exactMatch(val))
			return false;

		for (int i=0; i<items.size(); i++){
			if (i!=index.row() && items.at(i) == val)
				return false;
		}

		items[index.row()] = val;
		emit dataChanged(index, index);
		return true;
	}

	QVariant headerData(int section, Qt::Orientation, int role) const{
		if (role != Qt::DisplayRole || section > 0)
			return QVariant();

		return tr("Tag");
	}

	Qt::ItemFlags flags(const QModelIndex &index) const{
		return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
	}

	QVariant data(const QModelIndex &index, int role) const{

		if (!index.isValid())
			return QVariant();

		const QString& item = items.at(index.row());
		switch (role){
		case Qt::ForegroundRole:
			if (item.isEmpty())
				return QBrush(QApplication::style()->standardPalette().color(QPalette::Disabled, QPalette::Text));
			return QVariant();
		case Qt::DisplayRole:
			if (item.isEmpty())
				return tr("Set tag...");
			return item;
		case Qt::EditRole:
			return item;
		default:
			return QVariant();
		}
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const{
		if (parent.isValid())
			return 0;
		return items.size();
	}

};


StringsModel::StringsModel(QObject* parent)
	:QAbstractTableModel(parent)
{}

StringsModel::StringsModel(const std::map<std::string, XorredBuffer>& strings, QObject* parent)
	:QAbstractTableModel(parent)
{
	for (const std::pair<std::string, XorredBuffer>& string: strings){

		if (isStandardField(string.first))
			continue;

		QString name = QString::fromUtf8(string.first.c_str(), string.first.size());

		items.push_back(QPair<QString, XorredBuffer>(name, string.second));
	}
}

std::map<std::string, XorredBuffer> StringsModel::takeStrings(){
	std::map<std::string, XorredBuffer> result;
	beginResetModel();
	for (QPair<QString, XorredBuffer>& string: items){
		QByteArray name = string.first.toUtf8();
		result.emplace(std::string(name.data(), name.size()),
					   std::move(string.second));
	}
	items.clear();
	endResetModel();
	return result;
}

QModelIndex StringsModel::appendString(QString name, XorredBuffer value) noexcept{
	if (!name.isEmpty() && hasString(name))
		return QModelIndex();
	size_t index = items.size();
	beginInsertRows(QModelIndex(), index, index);
	items.push_back(QPair<QString, XorredBuffer>(name, value));
	endInsertRows();
	return createIndex(index, 0);
}

void StringsModel::removeString(size_t index) noexcept{
	beginRemoveRows(QModelIndex(), index, index);
	items.removeAt(index);
	endRemoveRows();
}

//QAbstractItemModel
int StringsModel::rowCount(const QModelIndex &parent) const{
	if (parent.isValid())
		return 0;
	return items.size();
}

int StringsModel::columnCount(const QModelIndex &parent) const{
	if (parent.isValid())
		return 0;
	return 2;
}

QVariant StringsModel::data(const QModelIndex &index, int role) const{
	if (!index.isValid())
		return QVariant();

//	if (role != Qt::DisplayRole && role != Qt::EditRole && role != Qt::ForegroundRole)
//		return QVariant();

	const QPair<QString, XorredBuffer>& entry = items.at(index.row());
	if (index.column() == 0){
		switch (role){
		case Qt::ForegroundRole:
			if (entry.first.isEmpty())
				return QBrush(QApplication::style()->standardPalette().color(QPalette::Disabled, QPalette::Text));
			return QVariant();
		case Qt::DisplayRole:
			if (entry.first.isEmpty())
				return tr("Set name...");
			return entry.first;
		case Qt::EditRole:
			return entry.first;
		default:
			return QVariant();
		}
	} else if (index.column() == 1){
		switch (role){
		case Qt::DisplayRole:
		case Qt::EditRole:{
			SafeString<char> value = entry.second.plainString();
			return QString::fromUtf8(value.c_str(), value.size());
		}
		default:
			return QVariant();
		}
	}
	return QVariant();
}

QVariant StringsModel::headerData(int section, Qt::Orientation, int role) const{
	if (role != Qt::DisplayRole || section > 1)
		return QVariant();

	if (section == 0)
		return tr("Name");
	return tr("Value");
}

Qt::ItemFlags StringsModel::flags(const QModelIndex &index) const{
	return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool StringsModel::setData(const QModelIndex &index, const QVariant &value, int role){
	if (!index.isValid() || role != Qt::EditRole || !value.canConvert<QString>())
		return false;

	//QString

	if (index.column() == 0){

		QString name = value.toString();

		// ToDo: display a hint for why it didn't work?
		if (isStandardField(name)){
			emit editFailed(index, tr("Name is not allowed."));
			return false;
		}

		if (name.isEmpty()){
			emit editFailed(index, tr("Name cannot be empty."));
			return false;
		}

		for (int i=0; i<items.size(); i++){
			if (index.row() != i && items.at(i).first == name){
				emit editFailed(index, tr("Name already exists."));
				return false;
			}
		}

		items[index.row()].first = name;
		return true;
	}
	if (index.column() == 1){
		QByteArray data = value.toString().toUtf8();
		items[index.row()].second = XorredBuffer(SafeVector<uint8_t>(data.begin(), data.end()));
		return true;
	}

	return false;
}

static void setButtonColor(const QColor color, QToolButton* btn){
	QPixmap p(btn->size());
	p.fill(color);
	QIcon icon(p);
	icon.addPixmap(p, QIcon::Disabled);
	btn->setIcon(icon);
}

void EntryEditDialog::disable(){
	fparentGroup = QKdbxDatabase::Group();
	fentry = QKdbxDatabase::Entry();
	setWindowTitle(windowTitle() + tr(" (deleted)"));
	ui->updateButton->setEnabled(false);
	ui->tabWidget->setEnabled(false);
}


EntryEditDialog::EntryEditDialog(QKdbxDatabase::Version version, QWidget *parent) :
	QDialog(parent),
	fentry(version.parent()),
	ftimes(version->times),
	currentIcon(version->icon),
	stringsModel(new StringsModel(version->strings, this)),
	binariesModel(new BinariesModel(version->binaries, this)),
	tagsModel(new TagsModel(version->tags, this)),
	fgColor(QString::fromUtf8(version->fgColor.c_str(), version->fgColor.size())),
	bgColor(QString::fromUtf8(version->bgColor.c_str(), version->bgColor.size())),
	ui(new Ui::EntryEditDialog)
{
	ui->setupUi(this);
	setWindowTitle(tr("Entry: %1").arg(QKdbxDatabase::entryString(version, Kdbx::Database::Version::titleString)));

	ui->iconButton->setIcon(version.model()->icon(version));
	ui->titleEdit->setText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::titleString));
	ui->usernameEdit->setText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::userNameString));
	passwordBuffer = QKdbxDatabase::entryStringBuffer(version, Kdbx::Database::Version::passwordString);
	ui->passwordLabel->setText(QString(passwordBuffer.size(), QChar('*')));
	ui->urlEdit->setText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::urlString));
	ui->expiresCheckBox->setChecked(ftimes.expires);
	ui->expireEdit->setDateTime(QDateTime::fromTime_t(ftimes.expiry, Qt::UTC));
	ui->notesEdit->setPlainText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::notesString));

	ui->stringView->setModel(stringsModel);
	ui->filesView->setModel(binariesModel);
	connect(stringsModel, &StringsModel::editFailed, this, &EntryEditDialog::onStringsEditFailed, Qt::QueuedConnection);

	std::string uuid = std::string(version.parent()->uuid());
	ui->uuidLabel->setText(QString::fromUtf8(uuid.c_str(), uuid.size()));
	ui->overrideUrlEdit->setText(QString::fromUtf8(version->overrideUrl.c_str(), version->overrideUrl.size()));
	ui->tagsView->setModel(tagsModel);
	if (fgColor.isValid()){
		ui->fgColorCheckbox->setChecked(true);
		setButtonColor(fgColor, ui->fgColorButton);
	} else{
		QPixmap p(ui->fgColorButton->size());
		setButtonColor(Qt::black, ui->fgColorButton);
	}
	if (bgColor.isValid()){
		ui->bgColorCheckbox->setChecked(true);
		setButtonColor(bgColor, ui->bgColorButton);
	} else{
		setButtonColor(Qt::white, ui->bgColorButton);
	}

	if (ftimes.creation)
		ui->createdLabel->setText(QDateTime::fromTime_t(ftimes.creation).toString(Qt::SystemLocaleLongDate));
	if (ftimes.lastModification)
		ui->lastModifiedLabel->setText(QDateTime::fromTime_t(ftimes.lastModification).toString(Qt::SystemLocaleLongDate));
	if (ftimes.lastAccess)
		ui->lastAccessedLabel->setText(QDateTime::fromTime_t(ftimes.lastAccess).toString(Qt::SystemLocaleLongDate));
	if (ftimes.locationChanged)
		ui->locationChangedLabel->setText(QDateTime::fromTime_t(ftimes.locationChanged).toString(Qt::SystemLocaleLongDate));
	ui->usageCountLabel->setText(QString("%1").arg(ftimes.usageCount));

	connect(version.model(), &QKdbxDatabase::modelAboutToBeReset, this, &EntryEditDialog::onModelReset);
	connect(version.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &EntryEditDialog::onGroupRemove);
	connect(version.model(), &QKdbxDatabase::beginEntryRemove, this, &EntryEditDialog::onEntryRemove);
}

EntryEditDialog::EntryEditDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Group parentGroup, QWidget *parent) :
	QDialog(parent),
	fparentGroup(parentGroup),
	currentIcon(Kdbx::StandardIcon::Key),
	stringsModel(new StringsModel(this)),
	binariesModel(new BinariesModel(this)),
	tagsModel(new TagsModel(this)),
	ui(new Ui::EntryEditDialog)
{
	ui->setupUi(this);
	setWindowTitle(tr("New entry..."));

	ui->iconButton->setIcon(QKdbxDatabase::Icons::standardIcon(Kdbx::StandardIcon::Key));
	ui->stringView->setModel(stringsModel);
	ui->filesView->setModel(binariesModel);
	connect(stringsModel, &StringsModel::editFailed, this, &EntryEditDialog::onStringsEditFailed, Qt::QueuedConnection);

	std::string uuid = std::string(Uuid::generate());
	ui->uuidLabel->setText(QString::fromUtf8(uuid.c_str(), uuid.size()));
	ui->tagsView->setModel(tagsModel);
	if (fgColor.isValid()){
		ui->fgColorCheckbox->setChecked(true);
		setButtonColor(fgColor, ui->fgColorButton);
	} else{
		QPixmap p(ui->fgColorButton->size());
		setButtonColor(Qt::black, ui->fgColorButton);
	}
	if (bgColor.isValid()){
		ui->bgColorCheckbox->setChecked(true);
		setButtonColor(bgColor, ui->bgColorButton);
	} else{
		setButtonColor(Qt::white, ui->bgColorButton);
	}

	ftimes.creation = time(nullptr);
	ui->createdLabel->setText(QDateTime::fromTime_t(ftimes.creation).toString(Qt::SystemLocaleLongDate));
	ftimes.lastModification = time(nullptr);
	ui->lastModifiedLabel->setText(QDateTime::fromTime_t(ftimes.lastModification).toString(Qt::SystemLocaleLongDate));
	ftimes.lastAccess = time(nullptr);
	ui->lastAccessedLabel->setText(QDateTime::fromTime_t(ftimes.lastAccess).toString(Qt::SystemLocaleLongDate));
	ftimes.locationChanged = 0;
	ui->usageCountLabel->setText(QString("0"));

	connect(parentGroup.model(), &QKdbxDatabase::modelAboutToBeReset, this, &EntryEditDialog::onModelReset);
	connect(parentGroup.model(), &QKdbxDatabase::rowsAboutToBeRemoved, this, &EntryEditDialog::onGroupRemove);
	connect(parentGroup.model(), &QKdbxDatabase::beginEntryRemove, this, &EntryEditDialog::onEntryRemove);
}



EntryEditDialog::~EntryEditDialog()
{
	delete ui;
}

//QObject
//bool EntryEditDialog::eventFilter(QObject * watched, QEvent * event){
//	if (event->type() == QEvent::ToolTip){
//		event->ignore();
//		return true;
//	}
//	return false;
//}

void EntryEditDialog::onPasswordEdited(QString pass){

	QByteArray tmp = pass.toUtf8();
	passwordBuffer = XorredBuffer::fromRaw(SafeVector<uint8_t>(tmp.begin(), tmp.end()), getRandomBytes(tmp.size(), this));

	if (ui->showPasswordButton->isChecked()){
		ui->passwordLabel->setText(pass);
	}else{
		ui->passwordLabel->setText(QString(pass.size(), QChar('*')));
	}
}

void EntryEditDialog::onModelReset(){
	disable();
}

void EntryEditDialog::onGroupRemove(QModelIndex parent, int begin, int end){
	QKdbxDatabase::Group group;
	QKdbxDatabase::Group ancestor;

	if (fentry){
		group = fentry.model()->group(parent);
		ancestor = fentry.parent();
	} else if (fparentGroup){
		group = fparentGroup.model()->group(parent);
		ancestor = fparentGroup;
	}
	while (ancestor && ancestor.parent() != group){
		ancestor = ancestor.parent();
	}
	if (ancestor){
		for (; begin<= end; ++begin){
			if (group.group(begin) == ancestor){
				disable();
				return;
			}
		}
	}
}


void EntryEditDialog::onEntryRemove(QKdbxDatabase::Group group, size_t index){
	if (fentry == group.entry(index))
		disable();
}

void EntryEditDialog::onIconSelected(Kdbx::Icon icon){
	currentIcon = std::move(icon);
	if (fentry)
		ui->iconButton->setIcon(fentry.model()->icons()->icon(currentIcon));
}

void EntryEditDialog::onStringsEditFailed(const QModelIndex& index, QString reason){
	QRect visRect = ui->stringView->visualRect(index);
	QPoint pt (visRect.left()/2 + visRect.right()/2, visRect.top());
	pt = ui->stringView->viewport()->mapToGlobal(pt);
	QToolTip::showText(pt, reason, this, QRect(), 5000);
}

void EntryEditDialog::on_showPasswordButton_toggled(bool checked)
{
	auto plain = passwordBuffer.plainString();
	QString qplain = QString::fromUtf8(plain.c_str(), plain.size());

	if (checked){
		ui->passwordLabel->setText(qplain);
	}else{
		ui->passwordLabel->setText(QString(qplain.size(), QChar('*')));
	}
}

void EntryEditDialog::on_changePasswordButton_clicked(){
	auto plain = passwordBuffer.plainString();
	PasswordChangeDialog* dlg = new PasswordChangeDialog(QString::fromUtf8(plain.c_str(), plain.size()), this);
	dlg->setAttribute( Qt::WA_DeleteOnClose, true );
	dlg->setModal(true);
	connect(dlg, SIGNAL(passwordChanged(QString)), this, SLOT(onPasswordEdited(QString)));
	dlg->show();
}

void EntryEditDialog::on_iconButton_clicked(){
	IconDialog* iconDialog = new IconDialog(fentry.model(), currentIcon, this);
	connect(iconDialog, &IconDialog::iconAccepted, this, &EntryEditDialog::onIconSelected);
	iconDialog->show();
}

void EntryEditDialog::on_stringAdd_clicked(){
	QModelIndex idx = stringsModel->appendString();
	ui->stringView->setCurrentIndex(idx);
	ui->stringView->edit(idx);
}

void EntryEditDialog::on_stringRemove_clicked(){
	QModelIndex idx = ui->stringView->currentIndex();
	if (idx.isValid()){
		stringsModel->removeString(idx.row());
	}
}

void EntryEditDialog::on_fileAttach_clicked(){
	QString filename = QFileDialog::getOpenFileName(this, tr("Attach a file to an entry."), QString(), tr("Any file")+" (*.*)");
	if (filename.isNull())
		return;

	QByteArray filename8 = filename.toUtf8();
	std::ifstream file;
	file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try{
		file.open(std::string(filename8.data(), filename8.size()), std::ios::in|std::ios::binary);
		QModelIndex index = binariesModel->insertFile(QFileInfo(filename).fileName(),
								  std::make_shared<SafeVector<uint8_t>>(
									  std::istreambuf_iterator<char>(file),
									  std::istreambuf_iterator<char>()));
		ui->filesView->setCurrentIndex(index);
	}catch(std::exception& e){
		QMessageBox::critical(this, tr("Error opening file."), tr("Error opening file '%1'.\n%2").arg(filename).arg(QString::fromUtf8(e.what())));
	}

}

void EntryEditDialog::on_fileExport_clicked(){

	QModelIndex index = ui->filesView->currentIndex();
	if (!index.isValid())
		return;

	QFileDialog dlg(this, tr("Export file."));
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.selectFile(binariesModel->getFilename(index));
	if (dlg.exec() != QDialog::Accepted || dlg.selectedFiles().size() != 1)
		return;


	QByteArray filename = dlg.selectedFiles().first().toUtf8();
	std::ofstream file;
	file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try{
		file.open(std::string(filename.data(), filename.size()), std::ios::out|std::ios::binary);
		auto data = binariesModel->getFile(index);
		file.write(reinterpret_cast<const char*>(data->data()), data->size());
		file.close();
	}catch(std::exception& e){
		QMessageBox::critical(this, tr("Error writing file."), tr("Error writing file '%1'.\n%2").arg(dlg.selectedFiles().first()).arg(QString::fromUtf8(e.what())));
	}

}

void EntryEditDialog::on_fileRemove_clicked(){
	QModelIndex index = ui->filesView->currentIndex();
	if (!index.isValid())
		return;
	binariesModel->eraseFile(index);
}

void EntryEditDialog::on_addTagButton_clicked(){
	ui->tagsView->setCurrentIndex(tagsModel->add());
}

void EntryEditDialog::on_removeTagButton_clicked(){
	QModelIndex index = ui->tagsView->currentIndex();
	if (index.isValid())
		tagsModel->remove(index);
}

void EntryEditDialog::on_fgColorButton_clicked(){
	QColor color = QColorDialog::getColor(fgColor.isValid()?fgColor:QColor(Qt::black), this);
	if (color.isValid()){
		fgColor = color;
		setButtonColor(fgColor, ui->fgColorButton);

	}
}

void EntryEditDialog::on_bgColorButton_clicked(){
	QColor color = QColorDialog::getColor(bgColor.isValid()?bgColor:QColor(Qt::black), this);
	if (color.isValid()){
		bgColor = color;
		setButtonColor(bgColor, ui->bgColorButton);
	}
}

static void emplaceString(std::map<std::string, XorredBuffer>& strings, const char* name, const QByteArray& data, bool protect, QWidget* widget){
	if (protect){
		std::vector<uint8_t> mask = getRandomBytes(data.size(), widget);
		strings.emplace(name, XorredBuffer::fromRaw(data.begin(), data.end(), mask.begin()));
	}else{
		strings.emplace(name, XorredBuffer(data.begin(), data.end()));
	}
}

void EntryEditDialog::on_updateButton_clicked(){

	QKdbxDatabase* model;
	if (fentry){
		model = fentry.model();
	}else if (fparentGroup){
		model = fparentGroup.model();
	}else{
		reject();
		return;
	}

	Kdbx::Database::Version::Ptr version(new Kdbx::Database::Version());

	version->times = ftimes;
	version->icon = currentIcon;
	version->strings = stringsModel->takeStrings();

	Kdbx::MemoryProtectionFlags memoryProtection = model->get()->settings().memoryProtection;
	emplaceString(version->strings,
				  Kdbx::Database::Version::titleString,
				  ui->titleEdit->text().toUtf8(),
				  memoryProtection.test(Kdbx::MemoryProtection::Title),
				  this);
	emplaceString(version->strings,
				  Kdbx::Database::Version::userNameString,
				  ui->usernameEdit->text().toUtf8(),
				  memoryProtection.test(Kdbx::MemoryProtection::UserName),
				  this);
	if (memoryProtection.test(Kdbx::MemoryProtection::Title)){
		version->strings.emplace(Kdbx::Database::Version::passwordString, passwordBuffer);
	}else{
		version->strings.emplace(Kdbx::Database::Version::passwordString, passwordBuffer.plainBuffer());
	}
	emplaceString(version->strings,
				  Kdbx::Database::Version::urlString,
				  ui->urlEdit->text().toUtf8(),
				  memoryProtection.test(Kdbx::MemoryProtection::Url),
				  this);
	emplaceString(version->strings,
				  Kdbx::Database::Version::notesString,
				  ui->notesEdit->document()->toPlainText().toUtf8(),
				  memoryProtection.test(Kdbx::MemoryProtection::UserName),
				  this);
	if ((version->times.expires = ui->expiresCheckBox->isChecked())){
		version->times.expiry = ui->expireEdit->dateTime().toTime_t();
	}else{
		version->times.expiry = 0;
	}

	version->binaries = binariesModel->takeBianaries();

	QByteArray utf8Tmp =ui->overrideUrlEdit->text().toUtf8();
	version->overrideUrl = std::string(utf8Tmp.data(), utf8Tmp.size());
	version->tags = tagsModel->takeTags();
	if (fgColor.isValid()){
		utf8Tmp = fgColor.name().toUtf8();
		version->fgColor = std::string(utf8Tmp.data(), utf8Tmp.size());
	}
	if (bgColor.isValid()){
		utf8Tmp = bgColor.name().toUtf8();
		version->bgColor = std::string(utf8Tmp.data(), utf8Tmp.size());
	}
	version->times.lastModification = std::time(nullptr);
	//ToDo: what is usage count anyway?

	if (fentry){
		fentry.addVersion(std::move(version), fentry->versions());
	}else{
		Kdbx::Database::Entry* entry = new Kdbx::Database::Entry(Uuid(ui->uuidLabel->text().toStdString()), std::move(version));
		fentry = fparentGroup.addEntry(Kdbx::Database::Entry::Ptr(entry), fparentGroup.entries());
		fparentGroup = QKdbxDatabase::Group();
	}
	accept();


}

