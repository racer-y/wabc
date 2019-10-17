/*
* Copyright[yyyy][name of copyright owner]
* Copyright (c) 2019, Yuan zhi wei <Racer_y@126.com>. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http ://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#pragma once

#include <memory.h>
#include <assert.h>
#include <process.h>
#include "w_std.h"

inline LONG int_atomic_inc(volatile LONG & x)
{ return ::InterlockedIncrement( &x ); }

inline LONG int_atomic_dec(volatile LONG & x)
{ return ::InterlockedDecrement(&x); }

inline LONG int_atomic_get(volatile LONG & x)
{ 
	LONG result= 0;
	::InterlockedExchange(&result,x); 
	return result;
}

// cas
inline LONG cmpxchg(volatile LONG *dest, const LONG & compare, const LONG & new_)
{ 
#if _MSC_VER >= 1300
	return ::InterlockedCompareExchange( dest, new_, compare );
#else
	return (LONG)::InterlockedCompareExchange( (PVOID *)dest, (PVOID)(new_), (PVOID)(compare) );
#endif
	//		if( *dest == compare)
	//		{  
	//			const LONG old= *dest;
	//			*dest= new_; 
	//			return old;
	//		}
	//		return *dest;
}

namespace wabc
{
	// --------------------------------------------------------------------

	struct mutex
	{
		class guard
		{
			mutex	&m_mutex;
		public:
			explicit guard(mutex &m) : m_mutex(m)
			{
				m_mutex.lock();
			}

			~guard()
			{
				m_mutex.unlock();
			}
		};

		CRITICAL_SECTION m_crtical_section;

		bool try_lock()
		{
			const BOOL ret= ::TryEnterCriticalSection(&m_crtical_section);
			return ret != FALSE;
		}

		bool lock()
		{
			::EnterCriticalSection(&m_crtical_section);
			return true;
		}

		void unlock()
		{
			::LeaveCriticalSection(&m_crtical_section);
		}

		mutex()
		{
			::InitializeCriticalSection(&m_crtical_section);
		}

		~mutex()
		{
			::DeleteCriticalSection(&m_crtical_section);
		}

	private:
		mutex(const mutex &);
		void operator=(const mutex &);
	};

	// --------------------------------------------------------------------

	struct semaphore
	{
		HANDLE	m_handle;

		semaphore(LONG init_count, LONG max_count)
		{
			m_handle= ::CreateSemaphore(0, init_count, max_count, 0);
		}

		~semaphore()
		{
			if (m_handle)
				::CloseHandle(m_handle);
		}

		void release()
		{
			::ReleaseSemaphore(m_handle, 1, 0);
		}

		void release(LONG n)
		{
			::ReleaseSemaphore(m_handle, n, 0);
		}

		bool wait(DWORD dwMillionSeconds)
		{
			return ::WaitForSingleObject(m_handle, dwMillionSeconds) == WAIT_OBJECT_0;
		}

		bool wait()
		{
			bool result = ::WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0;
			return result;
		}

	private:
		semaphore(const semaphore &);
		void operator=(const semaphore &);
	};

	// --------------------------------------------------------------------

	struct event_mutex
	{
		HANDLE m_handle;

		bool lock(DWORD dwMillionSeconds)
		{
			return wait(dwMillionSeconds);
		}

		bool lock()
		{
			return wait();
		}

		void unlock()
		{
			set();
		}

		bool wait(DWORD dwMillionSeconds)
		{
			return ::WaitForSingleObject(m_handle, dwMillionSeconds) == WAIT_OBJECT_0;
		}

		bool wait()
		{
			bool result = ::WaitForSingleObject(m_handle, INFINITE) == WAIT_OBJECT_0;
			return result;
		}

		void set()
		{
			::SetEvent(m_handle);
		}

		void reset()
		{
			::ResetEvent(m_handle);
		}

		explicit event_mutex(BOOL bInitialState = true,
			BOOL bManualReset = true, LPCTSTR pName = 0)
		{
			m_handle = ::CreateEvent(0, bManualReset, bInitialState, pName);
		}

		~event_mutex()
		{
			if (m_handle != 0)
				::CloseHandle(m_handle);
		}
	};

	// --------------------------------------------------------------------

	class thread
	{
		// --------------------------------------------------------------------

		struct thread_param
		{
			event_mutex	m_mutex;
			unsigned (WINAPI *fun)(void *);
			void *param;

			thread_param(unsigned (WINAPI *f)(void *), void *p);
		};

		static unsigned WINAPI thread_proxy(LPVOID);

		thread(const thread &);
		void operator=(const thread &);
	public:
		HANDLE	m_hThread;
		unsigned int	m_thread_id;

		thread()
		{
			::memset(this, 0, sizeof(*this));
		}

		~thread(){ detach(); }

		bool operator !()const { return m_hThread == 0; }
		operator bool()const { return m_hThread != 0; }

		bool run(unsigned WINAPI fun(void *), void *p = 0)
		{
			typedef unsigned int (WINAPI *fun_t)(void *);
			assert(m_hThread == 0);
			thread_param param(fun, p);
			m_hThread = (HANDLE)_beginthreadex(0, 0, &thread_proxy, &param, 0, &m_thread_id);
			if (m_hThread)
			{
				param.m_mutex.wait();
				return true;
			}
			return false;
		}

		template<typename T>
		bool run(unsigned (WINAPI *fun)(T *), T * p)
		{
			typedef unsigned (WINAPI *fun_t)(void *);
			return run(*reinterpret_cast<fun_t*>(&fun), p);
		}

		template<typename T>
		bool run(unsigned (WINAPI *fun)(T ), T  p)
		{
			typedef unsigned (WINAPI *fun_t)(void *);
			return run(*reinterpret_cast<fun_t*>(&fun), (void *)p);
		}

		template<typename T>
		bool run(unsigned (WINAPI *fun)(T &), T  &p)
		{
			typedef unsigned (WINAPI *fun_t)(void *);
			return run(*reinterpret_cast<fun_t*>(&fun), (void *)&p);
		}

		bool joinable()const
		{
			const bool result = m_hThread && ::WaitForSingleObject(m_hThread, 0) !=
				WAIT_OBJECT_0;
			return result;
		}

		void join();

		void detach();
	};
}