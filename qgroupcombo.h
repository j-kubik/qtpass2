#ifndef QGROUPCOMBO_H
#define QGROUPCOMBO_H

#include <QComboBox>

class QTreeView;
class QModelIndex;

class QGroupCombo : public QComboBox
{
public:
	QGroupCombo(QWidget* parent);

	inline QTreeView* view() const noexcept{
		return fview;
	}

	void setCurrentIndex(const QModelIndex& index);
private:
	bool eventFilter(QObject* object, QEvent* event) override;
	void showPopup() override;
	void hidePopup() override;


	QTreeView* fview;
	bool skipHide;

};

#endif // QGROUPCOMBO_H
