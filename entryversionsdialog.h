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
#ifndef ENTRYVERSIONSDIALOG_H
#define ENTRYVERSIONSDIALOG_H

#include <QDialog>
#include <QAbstractTableModel>

#include <libkeepass2pp/databasemodel.h>

namespace Ui {
class EntryVersionsDialog;
}

class QKdbxDatabase;

class EntryVersionsModel: public QAbstractTableModel{
	Q_OBJECT
public:

	EntryVersionsModel(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry, QObject* parent);

	Qt::ItemFlags flags(const QModelIndex & index) const override;
	QVariant data(const QModelIndex & index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	int rowCount(const QModelIndex & parent) const override;
	int columnCount(const QModelIndex & parent) const override;


	inline const Kdbx::DatabaseModel<QKdbxDatabase>::Entry& entry() const noexcept{
		return fentry;
	}

	inline Kdbx::DatabaseModel<QKdbxDatabase>::Version version(const QModelIndex& index) const noexcept{
		return fentry.version(index.row());
	}

private slots:
	void groupRemoved(QModelIndex index, int begin, int end);
	void entryRemove(Kdbx::DatabaseModel<QKdbxDatabase>::Group parent, size_t index);
	void beginVersionAdd(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry, size_t index);
	void endVersionAdd(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry, size_t index);
	void beginVersionRemove(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry, size_t index);
	void endVersionRemove(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry, size_t index);

private:
	Kdbx::DatabaseModel<QKdbxDatabase>::Entry fentry;
};


class EntryVersionsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EntryVersionsDialog(Kdbx::DatabaseModel<QKdbxDatabase>::Entry entry, QWidget *parent = 0);
	~EntryVersionsDialog();


private slots:
	void showCurrentItem();
	void itemContChanged();
	void showItem(const QModelIndex& index);
	void currentChanged(const QModelIndex& current, const QModelIndex& previous);

	void on_restore_clicked();

	void on_erase_clicked();

private:
	Ui::EntryVersionsDialog *ui;
	EntryVersionsModel* entryVersions;
};

#endif // ENTRYVERSIONSDIALOG_H
