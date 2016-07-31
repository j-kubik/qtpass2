#ifndef QMDIACTION_H
#define QMDIACTION_H

#include <QAction>

class QMdiAction : public QAction{
public:

	class State{
	public:
		bool checkable;
		bool checked;
		bool enabled;
		QString text;


	};

	QMdiAction(QObject* parent);
};

#endif // QMDIACTION_H
