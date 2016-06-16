/*Copyright (C) 2016 Jaroslaw Kubik
 *
   This file is part of QtPass2.

QtPass2 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QtPass2 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QtPass2.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QCoreApplication>
#include <QAbstractNativeEventFilter>

#ifdef XCB_EVENTS_SSL_ENTROPY_SOURCE
#include <xcb/xcb.h>
#elif WIN32_EVENTS_SSL_ENTROPY_SOURCE
#include <windows.h>
#endif

#include <openssl/rand.h>

void attachSSLEntropyEventFilter(QCoreApplication* app) noexcept;

#ifdef XCB_EVENTS_SSL_ENTROPY_SOURCE

namespace {

class SSLRandEventFilter: public QAbstractNativeEventFilter{
private:
	inline SSLRandEventFilter(QCoreApplication* app) noexcept
		:app(app),
		  lastDetail(0),
		  lastX(0),
		  lastY(0),
		  lastdX(0),
		  lastdY(0)
	{
		app->installNativeEventFilter(this);
	}

	inline ~SSLRandEventFilter() noexcept{
		app->removeNativeEventFilter(this);
	}

	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override{
		if (eventType == "xcb_generic_event_t"){
			double entropy = 0;

			xcb_generic_event_t* event = reinterpret_cast<xcb_generic_event_t*>(message);
			switch (event->response_type){
			case XCB_MOTION_NOTIFY:{
				xcb_motion_notify_event_t* mnEvent = reinterpret_cast<xcb_motion_notify_event_t*>(event);
				int16_t dx = mnEvent->root_x - lastX;
				int16_t dy = mnEvent->root_y - lastY;
				if (dx!=0 && dy!= 0 && dx!=lastdX && dy!=lastdY)
					entropy = 0.2;

				lastX = mnEvent->root_x;
				lastY = mnEvent->root_y;
				lastdX = dx;
				lastdY = dy;
				RAND_add(&mnEvent->root_x, sizeof(mnEvent->root_x), entropy);
				RAND_add(&mnEvent->root_y, sizeof(mnEvent->root_y), 0);
				RAND_add(&mnEvent->time, sizeof(mnEvent->time), 0);
			}
			case XCB_KEY_PRESS:{
				xcb_key_press_event_t* kpEvent = reinterpret_cast<xcb_key_press_event_t*>(event);
				if (kpEvent->detail != lastDetail)
					entropy = 0.05;
				lastDetail = kpEvent->detail;
				RAND_add(&kpEvent->detail, sizeof(kpEvent->detail), entropy);
				RAND_add(&kpEvent->time, sizeof(kpEvent->time), 0);
			}



			}
		}

		if (RAND_status() == 1)
			delete this;

		return false;

	}

	QCoreApplication* app;
	xcb_keycode_t lastDetail;
	int16_t lastX;
	int16_t lastY;
	int16_t lastdX;
	int16_t lastdY;

public:
	static inline void attach(QCoreApplication* app) noexcept{
		if (RAND_status() == 0)
			new SSLRandEventFilter(app);
	}
};

}

void attachSSLEntropyEventFilter(QCoreApplication* app) noexcept{
	SSLRandEventFilter::attach(app);
}



#elif WIN32_EVENTS_SSL_ENTROPY_SOURCE

namespace {

class SSLRandEventFilter: public QAbstractNativeEventFilter{
private:
	inline SSLRandEventFilter(QCoreApplication* app) noexcept
		:app(app)
	{
		app->installNativeEventFilter(this);
	}

	inline ~SSLRandEventFilter() noexcept{
		app->removeNativeEventFilter(this);
	}

	virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) override{
		if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG"){

			MSG* msg = reinterpret_cast<MSG*>(message);

			if (RAND_event() == 1){
				delete this;
			}

		}

		if (RAND_status() == 1)
			delete this;

		return false;

	}

	QCoreApplication* app;

public:
	static inline void attach(QCoreApplication* app) noexcept{
		if (RAND_status() == 0)
			new SSLRandEventFilter(app);
	}
};

}

void attachSSLEntropyEventFilter(QCoreApplication* app) noexcept{
	SSLRandEventFilter::attach(app);
}
#else

void attachSSLEntropyEventFilter(QCoreApplication*) noexcept{}

#endif



