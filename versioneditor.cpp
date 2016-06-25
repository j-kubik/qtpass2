#include "versioneditor.h"
#include "ui_versioneditor.h"

#include <QAbstractTableModel>
#include <QAbstractListModel>
#include <QMessageBox>
#include <QFileDialog>
#include <QColorDialog>

#include "qkdbxdatabase.h"
#include "passwordchangedialog.h"
#include "icondialog.h"

#include "utils.h"

#include <fstream>



//--------------------------------------------------------------------------------------

namespace{

class StringsModel: public QAbstractTableModel{
private:
	QVector<QPair<QString, XorredBuffer>> items;
public:

	StringsModel(QObject* parent)
		:QAbstractTableModel(parent)
	{}

	StringsModel(const std::map<std::string, XorredBuffer>& strings, QObject* parent)
		:QAbstractTableModel(parent)
	{
		setStrings(strings);
	}

	template <typename T>
	static bool isStandardField(const T& name) noexcept{
		return name == Kdbx::Database::Version::userNameString ||
			   name == Kdbx::Database::Version::titleString ||
			   name == Kdbx::Database::Version::passwordString ||
			   name == Kdbx::Database::Version::urlString ||
			   name == Kdbx::Database::Version::notesString;
	}

	template <typename T>
	bool hasString(const T& name) const noexcept{
		for (int i=0; i<items.size(); ++i){
			if (items.at(i).first == name)
				return true;
		}
		return false;
	}

	void setStrings(const std::map<std::string, XorredBuffer>& strings){
		beginResetModel();
		items.clear();
		items.reserve(strings.size());

		for (const std::pair<std::string, XorredBuffer>& string: strings){

			if (isStandardField(string.first))
				continue;

			QString name = QString::fromUtf8(string.first.c_str(), string.first.size());

			items.push_back(QPair<QString, XorredBuffer>(name, string.second));
		}
		endResetModel();
	}

	void clear(){
		beginResetModel();
		items.clear();
		endResetModel();
	}

	std::map<std::string, XorredBuffer> strings(){
		std::map<std::string, XorredBuffer> result;
		for (QPair<QString, XorredBuffer>& string: items){
			QByteArray name = string.first.toUtf8();
			result.emplace(std::string(name.data(), name.size()),
						   string.second);
		}
		return result;
	}

	std::map<std::string, XorredBuffer> takeStrings(){
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

	QModelIndex appendString(QString name = QString(), XorredBuffer value = XorredBuffer()) noexcept{
		if (!name.isEmpty() && hasString(name))
			return QModelIndex();
		size_t index = items.size();
		beginInsertRows(QModelIndex(), index, index);
		items.push_back(QPair<QString, XorredBuffer>(name, value));
		endInsertRows();
		return createIndex(index, 0);
	}

	void removeString(size_t index) noexcept{
		beginRemoveRows(QModelIndex(), index, index);
		items.removeAt(index);
		endRemoveRows();
	}


	//QAbstractItemModel
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

	QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override{
		if (role != Qt::DisplayRole || section > 1)
			return QVariant();

		if (section == 0)
			return tr("Name");
		return tr("Value");
	}

	Qt::ItemFlags flags(const QModelIndex &index) const override{
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	}


	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole){
		if (!index.isValid() || role != Qt::EditRole || !value.canConvert<QString>())
			return false;

		//QString

		if (index.column() == 0){

			QString name = value.toString();

			// ToDo: display a hint for why it didn't work?
			if (isStandardField(name)){
				//emit editFailed(index, tr("Name is not allowed."));
				return false;
			}

			if (name.isEmpty()){
				//emit editFailed(index, tr("Name cannot be empty."));
				return false;
			}

			for (int i=0; i<items.size(); i++){
				if (index.row() != i && items.at(i).first == name){
					//emit editFailed(index, tr("Name already exists."));
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

};

//--------------------------------------------------------------------------------------

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

	void setBinaries(const std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>>& its){
		beginResetModel();
		items.clear();
		items.reserve(its.size());
		for (const std::pair<std::string, std::shared_ptr<SafeVector<uint8_t>>>& item: its){
			items.emplace_back(
				QString::fromUtf8(item.first.c_str(), item.first.size()),
				item.second);
		}
		endResetModel();
	}

	void clear(){
		beginResetModel();
		items.clear();
		endResetModel();
	}

	std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>> binaries(){
		std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>> result;
		for (std::pair<QString, std::shared_ptr<SafeVector<uint8_t>>>& item: items){
			QByteArray name = item.first.toUtf8();
			result.emplace(std::string(name.data(), name.size()), item.second);
		}
		return result;
	}

	std::map<std::string, std::shared_ptr<SafeVector<uint8_t>>> takeBinaries(){
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

//--------------------------------------------------------------------------------------

class TagsModel: public QAbstractListModel{
private:
	QStringList items;

public:

	inline TagsModel(const std::vector<std::string>& tags, QObject* parent)
		:QAbstractListModel(parent)
	{
		setTags(tags);
	}

	inline TagsModel(QObject* parent)
		:QAbstractListModel(parent)
	{}

	void setTags(const std::vector<std::string>& tags){
		beginResetModel();
		items.clear();
		for (const std::string& tag: tags){
			items.push_back(QString::fromUtf8(tag.c_str(), tag.size()));
		}
		endResetModel();
	}

	void clear(){
		beginResetModel();
		items.clear();
		endResetModel();
	}

	std::vector<std::string> tags(){
		std::vector<std::string> result;
		for (const QString& item: items){
			QByteArray tag = item.toUtf8();
			result.emplace_back(tag.data(), tag.size());
		}
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

}
//--------------------------------------------------------------------------------------

class VersionEditor::Private: public Ui::VersionEditor{
public:
	StringsModel* stringsModel;
	BinariesModel* binariesModel;
	TagsModel* tagsModel;

	QKdbxDatabase* db;

	XorredBuffer password;
	Kdbx::Times times;
	Kdbx::Icon currentIcon;
	QColor fgColor;
	QColor bgColor;

	void setupUi(QTabWidget *VersionEditor){
		Ui::VersionEditor::setupUi(VersionEditor);
		stringsModel = new StringsModel(VersionEditor);
		stringView->setModel(stringsModel);
		binariesModel = new BinariesModel(VersionEditor);
		filesView->setModel(binariesModel);
		tagsModel = new TagsModel(VersionEditor);
		tagsView->setModel(tagsModel);
		db = nullptr;
	}
};

VersionEditor::VersionEditor(QWidget *parent) :
	QTabWidget(parent),
	ui(new Private)
{
	ui->setupUi(this);
}

VersionEditor::~VersionEditor()
{
	delete ui;
}

static void setButtonColor(const QColor color, QToolButton* btn){
	QPixmap p(btn->size());
	p.fill(color);
	QIcon icon(p);
	icon.addPixmap(p, QIcon::Disabled);
	btn->setIcon(icon);
}

static void emplaceString(std::map<std::string, XorredBuffer>& strings, const char* name, const QByteArray& data, bool protect, QWidget* widget){
	if (protect){
		std::vector<uint8_t> mask = getRandomBytes(data.size(), widget);
		strings.emplace(name, XorredBuffer::fromRaw(data.begin(), data.end(), mask.begin()));
	}else{
		strings.emplace(name, XorredBuffer(data.begin(), data.end()));
	}
}

void VersionEditor::fromVersion(QKdbxDatabase* dbx, const Uuid& uuid, const Kdbx::Database::Version* version){
	ui->db = dbx;

	ui->times = version->times;
	ui->currentIcon = version->icon;
	ui->stringsModel->setStrings(version->strings);
	ui->binariesModel->setBinaries(version->binaries);
	ui->tagsModel->setTags(version->tags);
	ui->fgColor = QColor(QString::fromUtf8(version->fgColor.c_str(), version->fgColor.size()));
	ui->bgColor = QColor(QString::fromUtf8(version->bgColor.c_str(), version->bgColor.size()));


	ui->iconButton->setIcon(ui->db->icons()->icon(ui->currentIcon));
	ui->titleEdit->setText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::titleString));
	ui->usernameEdit->setText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::userNameString));
	ui->password = QKdbxDatabase::entryStringBuffer(version, Kdbx::Database::Version::passwordString);
	ui->passwordLabel->setText(QString(ui->password.size(), QChar('*')));
	ui->urlEdit->setText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::urlString));
	ui->expiresCheckBox->setChecked(ui->times.expires);
	ui->expireEdit->setDateTime(QDateTime::fromTime_t(ui->times.expiry, Qt::UTC));
	ui->notesEdit->setPlainText(QKdbxDatabase::entryString(version, Kdbx::Database::Version::notesString));

	//connect(stringsModel, &StringsModel::editFailed, this, &EntryEditDialog::onStringsEditFailed, Qt::QueuedConnection);

	std::string suuid = std::string(uuid);
	ui->uuidLabel->setText(QString::fromUtf8(suuid.c_str(), suuid.size()));
	ui->overrideUrlEdit->setText(QString::fromUtf8(version->overrideUrl.c_str(), version->overrideUrl.size()));
	if (ui->fgColor.isValid()){
		ui->fgColorCheckbox->setChecked(true);
		setButtonColor(ui->fgColor, ui->fgColorButton);
	} else{
		setButtonColor(Qt::black, ui->fgColorButton);
	}
	if (ui->bgColor.isValid()){
		ui->bgColorCheckbox->setChecked(true);
		setButtonColor(ui->bgColor, ui->bgColorButton);
	} else{
		setButtonColor(Qt::white, ui->bgColorButton);
	}

	if (ui->times.creation)
		ui->createdLabel->setText(QDateTime::fromTime_t(ui->times.creation).toString(Qt::SystemLocaleLongDate));
	if (ui->times.lastModification)
		ui->lastModifiedLabel->setText(QDateTime::fromTime_t(ui->times.lastModification).toString(Qt::SystemLocaleLongDate));
	if (ui->times.lastAccess)
		ui->lastAccessedLabel->setText(QDateTime::fromTime_t(ui->times.lastAccess).toString(Qt::SystemLocaleLongDate));
	if (ui->times.locationChanged)
		ui->locationChangedLabel->setText(QDateTime::fromTime_t(ui->times.locationChanged).toString(Qt::SystemLocaleLongDate));
	ui->usageCountLabel->setText(QString("%1").arg(ui->times.usageCount));
}

void VersionEditor::fromVersion(Kdbx::DatabaseModel<QKdbxDatabase>::Version version){
	fromVersion(version.model(), version.parent()->uuid(), version.get());
}

void VersionEditor::newVersion(QKdbxDatabase* db, const Uuid& uuid, Kdbx::Times times){
	ui->db = db;
	ui->times = times;
	std::string suuid = std::string(uuid);
	ui->uuidLabel->setText(QString::fromUtf8(suuid.c_str(), suuid.size()));
}

void VersionEditor::toVersion(Kdbx::Database::Version* version){
	version->times = ui->times;
	version->icon = ui->currentIcon;
	version->strings = ui->stringsModel->strings();

	Kdbx::MemoryProtectionFlags memoryProtection = ui->db->get()->settings().memoryProtection;
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
		version->strings.emplace(Kdbx::Database::Version::passwordString, ui->password);
	}else{
		version->strings.emplace(Kdbx::Database::Version::passwordString, ui->password.plainBuffer());
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

	version->binaries = ui->binariesModel->binaries();

	QByteArray utf8Tmp =ui->overrideUrlEdit->text().toUtf8();
	version->overrideUrl = std::string(utf8Tmp.data(), utf8Tmp.size());
	version->tags = ui->tagsModel->tags();
	if (ui->fgColor.isValid()){
		utf8Tmp = ui->fgColor.name().toUtf8();
		version->fgColor = std::string(utf8Tmp.data(), utf8Tmp.size());
	}
	if (ui->bgColor.isValid()){
		utf8Tmp = ui->bgColor.name().toUtf8();
		version->bgColor = std::string(utf8Tmp.data(), utf8Tmp.size());
	}
	version->times.lastModification = std::time(nullptr);
	//ToDo: what is usage count anyway?
}

void VersionEditor::onPasswordEdited(QString pass){
	QByteArray tmp = pass.toUtf8();
	ui->password = XorredBuffer::fromRaw(SafeVector<uint8_t>(tmp.begin(), tmp.end()), safeGetRandomBytes(tmp.size(), this));

	if (ui->showPasswordButton->isChecked()){
		ui->passwordLabel->setText(pass);
	}else{
		ui->passwordLabel->setText(QString(pass.size(), QChar('*')));
	}
}

void VersionEditor::onIconSelected(Kdbx::Icon icon){
	ui->currentIcon = std::move(icon);
	if (ui->db)
		ui->iconButton->setIcon(ui->db->icons()->icon(ui->currentIcon));
}


void VersionEditor::on_showPasswordButton_toggled(bool checked)
{
	auto plain = ui->password.plainString();
	QString qplain = QString::fromUtf8(plain.c_str(), plain.size());

	if (checked){
		ui->passwordLabel->setText(qplain);
	}else{
		ui->passwordLabel->setText(QString(qplain.size(), QChar('*')));
	}
}

void VersionEditor::on_changePasswordButton_clicked()
{
	auto plain = ui->password.plainString();
	PasswordChangeDialog* dlg = new PasswordChangeDialog(QString::fromUtf8(plain.c_str(), plain.size()), this);
	dlg->setAttribute( Qt::WA_DeleteOnClose, true );
	dlg->setModal(true);
	connect(dlg, &PasswordChangeDialog::passwordChanged, this, &VersionEditor::onPasswordEdited);
	dlg->show();
}

void VersionEditor::on_iconButton_clicked()
{
	IconDialog* iconDialog = new IconDialog(ui->db, ui->currentIcon, this);
	connect(iconDialog, &IconDialog::iconAccepted, this, &VersionEditor::onIconSelected);
	iconDialog->show();
}

void VersionEditor::on_stringAdd_clicked()
{
	QModelIndex idx = ui->stringsModel->appendString();
	ui->stringView->setCurrentIndex(idx);
	ui->stringView->edit(idx);
}

void VersionEditor::on_stringRemove_clicked()
{
	QModelIndex idx = ui->stringView->currentIndex();
	if (idx.isValid()){
		ui->stringsModel->removeString(idx.row());
	}
}

void VersionEditor::on_fileAttach_clicked()
{
	QString filename = QFileDialog::getOpenFileName(this, tr("Attach a file to an entry."), QString(), tr("Any file")+" (*.*)");
	if (filename.isNull())
		return;

	QByteArray filename8 = filename.toUtf8();
	std::ifstream file;
	file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try{
		file.open(std::string(filename8.data(), filename8.size()), std::ios::in|std::ios::binary);
		QModelIndex index = ui->binariesModel->insertFile(QFileInfo(filename).fileName(),
								  std::make_shared<SafeVector<uint8_t>>(
									  std::istreambuf_iterator<char>(file),
									  std::istreambuf_iterator<char>()));
		ui->filesView->setCurrentIndex(index);
	}catch(std::exception& e){
		QMessageBox::critical(this, tr("Error opening file."), tr("Error opening file '%1'.\n%2").arg(filename).arg(QString::fromUtf8(e.what())));
	}
}

void VersionEditor::on_fileExport_clicked()
{
	QModelIndex index = ui->filesView->currentIndex();
	if (!index.isValid())
		return;

	QFileDialog dlg(this, tr("Export file."));
	dlg.setFileMode(QFileDialog::AnyFile);
	dlg.setAcceptMode(QFileDialog::AcceptSave);
	dlg.selectFile(ui->binariesModel->getFilename(index));
	if (dlg.exec() != QDialog::Accepted || dlg.selectedFiles().size() != 1)
		return;


	QByteArray filename = dlg.selectedFiles().first().toUtf8();
	std::ofstream file;
	file.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
	try{
		file.open(std::string(filename.data(), filename.size()), std::ios::out|std::ios::binary);
		auto data = ui->binariesModel->getFile(index);
		file.write(reinterpret_cast<const char*>(data->data()), data->size());
		file.close();
	}catch(std::exception& e){
		QMessageBox::critical(this, tr("Error writing file."), tr("Error writing file '%1'.\n%2").arg(dlg.selectedFiles().first()).arg(QString::fromUtf8(e.what())));
	}
}

void VersionEditor::on_fileRemove_clicked()
{
	QModelIndex index = ui->filesView->currentIndex();
	if (!index.isValid())
		return;
	ui->binariesModel->eraseFile(index);
}

void VersionEditor::on_addTagButton_clicked()
{
	ui->tagsView->setCurrentIndex(ui->tagsModel->add());
}

void VersionEditor::on_removeTagButton_clicked()
{
	QModelIndex index = ui->tagsView->currentIndex();
	if (index.isValid())
		ui->tagsModel->remove(index);
}

void VersionEditor::on_fgColorButton_clicked()
{
	QColor color = QColorDialog::getColor(ui->fgColor.isValid()?ui->fgColor:QColor(Qt::black), this);
	if (color.isValid()){
		ui->fgColor = color;
		setButtonColor(color, ui->fgColorButton);
	}
}

void VersionEditor::on_bgColorButton_clicked()
{
	QColor color = QColorDialog::getColor(ui->bgColor.isValid()?ui->bgColor:QColor(Qt::black), this);
	if (color.isValid()){
		ui->bgColor = color;
		setButtonColor(color, ui->bgColorButton);
	}
}
