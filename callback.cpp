#include "callback.h"

bool CallbackSite::event(QEvent *e){
	if (e->type() == CallEventBase::CallType){
		CallEventBase* ce = static_cast<CallEventBase*>(e);
		ce->invoke();
		return true;
	}
	return QObject::event(e);
}

void CallbackSite::doDeleteLater(CallbackSite* target) noexcept{
	target->deleteLater();
}

CallbackSite::Ptr CallbackSite::create(){
	return Ptr(new CallbackSite(), &CallbackSite::doDeleteLater);
}
