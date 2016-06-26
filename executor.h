#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <QCoreApplication>
#include <QSharedPointer>
#include <QEvent>
#include <future>
#include <functional>

#include "utils.h"

class Executor{
private:
	class QSite;

	class CallEventBase: public QEvent{
	public:

		static constexpr QEvent::Type CallType = static_cast<QEvent::Type>(QEvent::User+1);

		inline CallEventBase() noexcept
			:QEvent(CallType)
		{}

		virtual void invoke() noexcept=0;

	};

	template <typename Result, typename Func>
	class CallEvent: public CallEventBase{
	private:

		Func fcallback;
		std::promise<Result> fpromise;

		void invoke() noexcept override{
			try{
				fcallback(std::move(fpromise));
			}catch(...){
				fpromise.set_exception(std::current_exception());
			}
		}

	public:
		inline CallEvent(Func&& callback) noexcept
			:fcallback(std::forward<Func>(callback))
		{}

		std::future<Result> get_future() noexcept{
			return fpromise.get_future();
		}
	};

	Executor();

	void callback(CallEventBase* event);

	NoexceptMutex mutex;
	QSite* site;

public:
	typedef std::shared_ptr<Executor> Ptr;
	typedef std::weak_ptr<Executor> WeakPtr;

	~Executor();

	void finish();

	template <typename Result, typename Func>
	std::future<Result> callback(Func&& func){
		auto event = new CallEvent<Result, Func>(std::forward<Func>(func));
		std::future<Result> result = event->get_future();
		callback(event);
		return result;
	}

	template <typename Result, typename Func, typename T, typename ...Args>
	std::future<Result> callback(Func&& func, T&& t, Args&&...args){
		using namespace std::placeholders;
		return callback(std::bind(std::forward<Func>(func), _1, std::forward<T>(t), std::forward<Args>(args)...));
	}

	static Ptr create(QObject* parent = 0);

	friend class QSite;
};

#endif // EXECUTOR_H
