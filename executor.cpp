#include "executor.h"
#include <type_traits>

namespace {

	class DestroyEvent: public QEvent{
	public:
		static constexpr QEvent::Type DestroyType = static_cast<QEvent::Type>(QEvent::User+2);

		inline DestroyEvent() noexcept
			:QEvent(DestroyType)
		{}
	};

}

class Executor::QSite: public QObject{
private:
	Executor::WeakPtr executor;

	bool event(QEvent *e) noexcept override{
		switch (std::underlying_type_t<QEvent::Type>(e->type())){
		case Executor::CallEventBase::CallType:
			e->accept();
			static_cast<Executor::CallEventBase*>(e)->invoke();
			return true;

		case DestroyEvent::DestroyType:
			e->accept();
			deleteLater(); // Just to be extra sure.
			return true;

		default:
			return QObject::event(e);

		}
	}

	void finish() noexcept{
		Executor::Ptr e = executor.lock();
		if (e){
			std::unique_lock<NoexceptMutex> lock(e->mutex);
			e->site = nullptr;
		}
		executor.reset();
	}

public:

	QSite(Executor::WeakPtr executor, QObject* parent) noexcept
		:QObject(parent),
		  executor(std::move(executor))
	{}

	~QSite() noexcept{
		finish();
	}

};

Executor::Executor()
	:site(nullptr)
{}

void Executor::callback(CallEventBase* event){
	std::unique_lock<NoexceptMutex> lock(mutex);
	if (site){
		QCoreApplication::postEvent(site, event);
	}else{
		delete event;
	}
}

Executor::~Executor(){
	finish();
}

void Executor::finish(){
	std::unique_lock<NoexceptMutex> lock(mutex);
	if (site){
		QCoreApplication::postEvent(site, new DestroyEvent());
		site = nullptr;
	}
}

Executor::Ptr Executor::create(QObject* parent){
	Executor::Ptr result(new Executor());
	result->site = new QSite(result, parent);
	return result;
}

//------------------------------------------------------------------------------------
