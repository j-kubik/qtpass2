#ifndef VERSIONEDITOR_H
#define VERSIONEDITOR_H

#include <QTabWidget>

#include <libkeepass2pp/databasemodel.h>

namespace Ui {
class VersionEditor;
}

class QKdbxDatabase;

class VersionEditor : public QTabWidget
{
	Q_OBJECT

public:
	explicit VersionEditor(QWidget *parent = 0);
	~VersionEditor();

	void fromVersion(QKdbxDatabase* db, const Uuid& uuid, const Kdbx::Database::Version* version);
	void fromVersion(Kdbx::DatabaseModel<QKdbxDatabase>::Version version);
	void newVersion(QKdbxDatabase* db, const Uuid& uuid, Kdbx::Times times = Kdbx::Times::nowTimes());


	void toVersion(Kdbx::Database::Version* version);

private slots:
	void onPasswordEdited(QString pass);
	void onIconSelected(Kdbx::Icon icon);

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

private:
	class Private;

	Private *ui;


};

#endif // VERSIONEDITOR_H
