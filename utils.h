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
#ifndef UTILS_H
#define UTILS_H

#include <QWidget>
#include <vector>
#include <atomic>
#include <mutex>

#include <libkeepass2pp/util.h>

/** @brief Mutex class that doesn't throw.
 *
 * If it fails to aquire mutex it spin-locks instead.
 */
class NoexceptMutex{
private:
	std::mutex mutex;
	std::atomic_flag flag;
	bool mutexLocked;
public:

	inline NoexceptMutex() noexcept
		:flag(ATOMIC_FLAG_INIT),
		  mutexLocked(false)
	{}

	inline void lock() noexcept{
		try{
			mutex.lock();
			while(flag.test_and_set(std::memory_order_acquire));
			mutexLocked=true;
		}catch(...){
			while(flag.test_and_set(std::memory_order_acquire));
			mutexLocked=false;
		}
	}

	inline void unlock() noexcept{
		if(mutexLocked)
			mutex.unlock();
		flag.clear(std::memory_order_release);
	}
};


std::vector<uint8_t> getRandomBytes(std::size_t size, QWidget*);
SafeVector<uint8_t> safeGetRandomBytes(std::size_t size, QWidget*);

inline std::string utf8QString(QString s){
	QByteArray tmp = s.toUtf8();
	return std::string(tmp.data(), tmp.size());
}

#endif // UTILS_H
