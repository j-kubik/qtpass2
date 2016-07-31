#include "iconseditor.h"
#include "ui_iconseditor.h"

#include "qkdbxdatabase.h"

#include <QAbstractListModel>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>

class StandardIconModel: public QAbstractListModel{
public:
	using QAbstractListModel::QAbstractListModel;

	int rowCount(const QModelIndex &) const override{
		return 69;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DecorationRole) const{
		if (role == Qt::DecorationRole && index.row() < 69){
			return QKdbxDatabase::Icons::standardIcon(Kdbx::StandardIcon(index.row()));
		}
		return QVariant();
	}
};

IconsEditor::IconsEditor(QWidget *parent)
	:QTabWidget(parent),
	  ui(new Ui::IconsEditor),
	  standardIconModel(new StandardIconModel(this)),
	  fdatabase(nullptr)
{
	ui->setupUi(this);
	ui->standardIconView->setModel(standardIconModel);

	connect(ui->standardIconView->selectionModel(), &QItemSelectionModel::currentChanged, this, &IconsEditor::onStandardCurrentChanged);
}

IconsEditor::IconsEditor(QKdbxDatabase* database, QWidget *parent)
	:QTabWidget(parent),
	  ui(new Ui::IconsEditor),
	  standardIconModel(new StandardIconModel(this))
{
	ui->setupUi(this);
	ui->standardIconView->setModel(standardIconModel);
	setDatabase(database);

	connect(ui->standardIconView->selectionModel(), &QItemSelectionModel::currentChanged, this, &IconsEditor::onStandardCurrentChanged);
	connect(ui->databaseIconView->selectionModel(), &QItemSelectionModel::currentChanged, this, &IconsEditor::onDbCurrentChanged);
}

Kdbx::Icon IconsEditor::current() const noexcept{
	switch (currentIndex()) {
	case 0:{
		QModelIndex i = ui->standardIconView->currentIndex();
		if (i.isValid())
			return Kdbx::Icon(Kdbx::StandardIcon(i.row()));
		return Kdbx::Icon();
	}
	case 1:
		if (fdatabase)
			return fdatabase->icons()->kdbxIcon(ui->databaseIconView->currentIndex());
	default:
		return Kdbx::Icon();
	}
}


void IconsEditor::setCurrent(const Kdbx::Icon& icon){
	switch (icon.type()){
	case Kdbx::Icon::Type::Standard:
		setCurrentIndex(0);
		ui->standardIconView->setCurrentIndex(standardIconModel->index(int(icon.standard())));
		break;
	case Kdbx::Icon::Type::Null:
		setCurrentIndex(0);
		ui->standardIconView->setCurrentIndex(QModelIndex());
		break;
	case Kdbx::Icon::Type::Custom:
		setCurrentIndex(1);
		if (fdatabase){
			ui->databaseIconView->setCurrentIndex(fdatabase->icons()->modelIndex(icon.custom()->uuid()));
		}else{
			ui->databaseIconView->setCurrentIndex(QModelIndex());
		}
		break;
	}
	emit currentIconChanged(current());
}

void IconsEditor::setDatabase(QKdbxDatabase* database){
	fdatabase = database;
	ui->databaseIconView->setModel(fdatabase->icons());
	connect(ui->databaseIconView->selectionModel(), &QItemSelectionModel::currentChanged, this, &IconsEditor::onDbCurrentChanged);
	connect(fdatabase, &QObject::destroyed, this, &IconsEditor::disable);
}


IconsEditor::~IconsEditor()
{
	delete ui;
}

void IconsEditor::on_importButton_clicked(){

	QString filename = QFileDialog::getOpenFileName(this, tr("Open icon file..."), QString(), tr("PNG image files (*.png)"));
	if (!fdatabase || filename.isEmpty())
		return;

	QPixmap p;
	if (!p.load(filename)){
		QMessageBox::critical(this, tr("Error opening icon file."), tr("Failed opening icon file."));
		return;
	}

	fdatabase->icons()->add(QIcon(p));
}

void IconsEditor::on_exportButton_clicked()
{

	QPersistentModelIndex index = ui->databaseIconView->currentIndex();

	QString filename = QFileDialog::getSaveFileName(this, tr("Save icon file..."), QString(), tr("PNG image files (*.png)"));
	if (!fdatabase || filename.isEmpty())
		return;

	if (!index.isValid() || index.row() >= fdatabase->icons()->size() )
		return;

	QIcon icon = fdatabase->icons()->icon(index.row());
	QPixmap p = icon.pixmap(icon.actualSize(QSize(64,64)));
	if (p.save(filename, "PNG") == false){
		QMessageBox::critical(this, tr("Error saving icon file."), tr("Failed saving icon file."));
	}
}

void IconsEditor::on_IconsEditor_currentChanged(){
	emit currentIconChanged(current());
}

void IconsEditor::onStandardCurrentChanged(){
	if (currentIndex() == 0)
		emit currentIconChanged(current());
}

void IconsEditor::onDbCurrentChanged(){
	if (currentIndex() == 1)
		emit currentIconChanged(current());
}

void IconsEditor::disable(){
	fdatabase = 0;
	ui->databaseIconView->setModel(nullptr);
	onDbCurrentChanged();
}

//------------------------------------------------------------------------------

IconsEditorDialog::IconsEditorDialog(QKdbxDatabase* database, QWidget* parent)
	:QDialog(parent),
	  feditor(new IconsEditor(database, this))
{
	setAttribute(Qt::WA_DeleteOnClose);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addWidget(feditor);
	setLayout(layout);

	setWindowTitle(tr("Database icons"));
}




