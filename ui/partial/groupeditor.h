#ifndef GROUPEDITOR_H
#define GROUPEDITOR_H

#include <QTabWidget>

#include "qkdbxdatabase.h"

namespace Ui {
class GroupEditor;
}

class GroupEditor : public QTabWidget{
	Q_OBJECT

public:
	explicit GroupEditor(QWidget *parent = 0);
	~GroupEditor();

	void setDefaults(QKdbxDatabase* db, const Kdbx::Uuid& uuid);

	void fromGroup(QKdbxDatabase* db, const Kdbx::Database::Group::Properties& properties, const Kdbx::Uuid& uuid);
	void fromGroup(QKdbxDatabase::Group group);

	Kdbx::Database::Group::Properties::Ptr toGroup();

private:
	Ui::GroupEditor *ui;
	QKdbxDatabase* fdb;
	Kdbx::Database::Group::Properties::Ptr fproperties;
};

#endif // GROUPEDITOR_H
