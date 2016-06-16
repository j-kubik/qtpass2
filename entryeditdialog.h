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
#ifndef ENTRYEDITDIALOG_H
#define ENTRYEDITDIALOG_H

#include <QDialog>

#include <libkeepass2pp/databasemodel.h>

#include <QAbstractTableModel>
//#include <QTreeView>

namespace Ui {
class EntryEditDialog;
}

class QKdbxGroup;
class QKdbxDatabase;

class BinariesModel;
class TagsModel;

class StringsModel: public QAbstractTableModel{
	Q_OBJECT
private:
	QVector<QPair<QString, XorredBuffer>> items;
public:

	StringsModel(QObject* parent);
	StringsModel(const std::map<std::string, XorredBuffer>& strings, QObject* parent);

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

	std::map<std::string, XorredBuffer> takeStrings();
	QModelIndex appendString(QString name = QString(), XorredBuffer value = XorredBuffer()) noexcept;
	void removeString(size_t index) noexcept;


	//QAbstractItemModel
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

signals:
	void editFailed(const QModelIndex& index, QString reason);
};

class EntryEditDialog : public QDialog{
	Q_OBJECT
private:
	void disable();

	XorredBuffer passwordBuffer;
	Kdbx::DatabaseModel<QKdbxDatabase>::Entry fentry;
	Kdbx::DatabaseModel<QKdbxDatabase>::Group fparentGroup;
	Kdbx::Times ftimes;
	Kdbx::Icon currentIcon;
	StringsModel* stringsModel;
	BinariesModel* binariesModel;
	TagsModel* tagsModel;
	QColor fgColor;
	QColor bgColor;
public:
	explicit EntryEditDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Version version, QWidget *parent = 0);
	explicit EntryEditDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Group parentGroup, QWidget *parent = 0);

	~EntryEditDialog();

private slots:
	void onPasswordEdited(QString pass);

	void onModelReset();
	void onGroupRemove(QModelIndex parent, int begin, int end);
	void onEntryRemove(Kdbx::DatabaseModel<QKdbxDatabase>::Group group, size_t index);
	void onIconSelected(Kdbx::Icon icon);
	void onStringsEditFailed(const QModelIndex& index, QString reason);

	void on_showPasswordButton_toggled(bool checked);

	void on_changePasswordButton_clicked();

	void on_iconButton_clicked();

	void on_stringAdd_clicked();

	void on_stringRemove_clicked();

	void on_fileAttach_clicked();

	void on_fileExport_clicked();

	void on_fileRemove_clicked();

	void on_addTagButton_clicked();

	void on_removeTagButton_clicked();

	void on_fgColorButton_clicked();

	void on_bgColorButton_clicked();

	void on_updateButton_clicked();

private:
	Ui::EntryEditDialog *ui;
};

#endif // ENTRYEDITDIALOG_H
