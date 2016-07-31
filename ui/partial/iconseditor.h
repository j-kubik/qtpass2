#ifndef ICONSEDITOR_H
#define ICONSEDITOR_H

#include <QTabWidget>
#include <QDialog>

#include <libkeepass2pp/icon.h>

namespace Ui {
class IconsEditor;
}

class QKdbxDatabase;
class StandardIconModel;

class IconsEditor : public QTabWidget
{
	Q_OBJECT

public:
	explicit IconsEditor(QWidget *parent = 0);
	IconsEditor(QKdbxDatabase* database, QWidget *parent = 0);
	~IconsEditor();

	Kdbx::Icon current() const noexcept;
	void setCurrent(const Kdbx::Icon& icon);

	inline QKdbxDatabase* database() const noexcept{
		return fdatabase;
	}

	void setDatabase(QKdbxDatabase* database);

private slots:
	void on_importButton_clicked();
	void on_exportButton_clicked();
	void on_IconsEditor_currentChanged();

	void onStandardCurrentChanged();
	void onDbCurrentChanged();

	void disable();

signals:
	void currentIconChanged(Kdbx::Icon current);

private:

	Ui::IconsEditor *ui;
	StandardIconModel* standardIconModel;
	QKdbxDatabase* fdatabase;
	Kdbx::Icon fcurrent;
};

class IconsEditorDialog: public QDialog{
private:
	IconsEditor* feditor;

public:
	IconsEditorDialog(QKdbxDatabase* database, QWidget* parent);
};


#endif // ICONSEDITOR_H
