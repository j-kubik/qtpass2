#ifndef CALLBACK_H
#define CALLBACK_H

#include <QCoreApplication>
#include <QSharedPointer>
#include <QEvent>
#include <future>
#include <functional>

//ToDo: callbackSite needs proper locking

class CallbackSite: public QObject{
private:

	class CallEventBase: public QEvent{
	public:

		static constexpr QEvent::Type CallType = static_cast<QEvent::Type>(QEvent::User+1);

		inline CallEventBase() noexcept
			:QEvent(CallType)
		{}

		virtual void invoke() noexcept=0;

	};

	template <typename Result, typename Func/*, typename ...Args*/>
	class CallEvent: public CallEventBase{
	private:

		Func fcallback;
		std::promise<Result> fpromise;
		//std::tuple<Args...> args;

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

	static void doDeleteLater(CallbackSite* target) noexcept;

	bool event(QEvent *e) override;

public:

	typedef QSharedPointer<CallbackSite> Ptr;
	typedef QWeakPointer<CallbackSite> WeakPtr;

	template <typename Result, typename Func>
	std::future<Result> callback(Func&& func) noexcept{
		auto event = new CallEvent<Result, Func>(std::forward<Func>(func));
		std::future<Result> result = event->get_future();
		QCoreApplication::postEvent(this, event);
		return result;
	}

	template <typename Result, typename Func, typename T, typename ...Args>
	std::future<Result> callback(Func&& func, T&& t, Args&&...args){
		using namespace std::placeholders;
		return callback(std::bind(std::forward<Func>(func), _1, std::forward<T>(t), std::forward<Args>(args)...));
	}


	static Ptr create();

};

#endif // CALLBACK_H
