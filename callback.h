#ifndef CALLBACK_H
#define CALLBACK_H

#include <QCoreApplication>
#include <QSharedPointer>
#include <QEvent>
#include <future>
#include <functional>

namespace helper{

template <int...>
class sequence{};

template <int i, int...n>
class generate: public generate<i-1, i-1, n...>{};

template <int...n>
class generate<0, n...>{
public:
	typedef sequence<n...> type;
};




}

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

	template <typename Result, typename Func, typename ...Args>
	class CallEvent: public CallEventBase{
	private:

		Func fcallback;
		std::promise<Result> fpromise;
		std::tuple<Args...> args;

		template <int ...n>
		void invoke(helper::sequence<n...>){
			try{
				fcallback(std::move(fpromise), std::move(std::get<n>(args))...);
			}catch(...){
				fpromise.set_exception(std::current_exception());
			}
		}

		void invoke() noexcept override{
			return invoke(typename helper::generate<sizeof...(Args)>::type());
		}

	public:
		inline CallEvent(Func&& callback, Args&&... args) noexcept
			:fcallback(std::forward<Func>(callback)),
			  args(std::make_tuple(std::forward<Args>(args)...))
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

	template <typename Result, typename Func, typename ...Args>
	auto callback(Func&& func, Args&&... args) noexcept -> std::future<Result> {

		auto event = new CallEvent<Result, Func, Args...>(std::forward<Func>(func), std::forward<Args>(args)...);
		std::future<Result> result = event->get_future();
		QCoreApplication::postEvent(this, event);
		return result;
	}

	static Ptr create();

};

#endif // CALLBACK_H
