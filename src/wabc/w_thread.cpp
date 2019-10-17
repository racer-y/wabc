#include "w_thread.h"

namespace wabc
{
	// --------------------------------------------------------------------
	// thread::thread_param
	// --------------------------------------------------------------------

	thread::thread_param::thread_param(unsigned (WINAPI *f)(void *), void *p)
		: m_mutex(false), fun(f), param(p)
	{
	}

	// --------------------------------------------------------------------
	// thread
	// --------------------------------------------------------------------

	unsigned WINAPI thread::thread_proxy(LPVOID p)
	{
		thread_param &tp = *reinterpret_cast<thread_param *>(p);

		void *param = tp.param;
		unsigned (WINAPI *fun)(void *)= tp.fun;
		tp.m_mutex.set();

		const unsigned int ret = fun(param);

		_endthreadex(ret);

		return ret;
	}

	// --------------------------------------------------------------------

	void thread::join()
	{
		if (m_hThread)
		{
			const bool result = ::WaitForSingleObject(m_hThread, INFINITE) ==
				WAIT_OBJECT_0;

			if (result)
			{
				::CloseHandle(m_hThread);
				m_hThread = 0;
			}
		}
	}

	// --------------------------------------------------------------------

	void thread::detach()
	{
		if (m_hThread)
		{
			::CloseHandle(m_hThread);
			m_hThread = 0;
			m_thread_id = 0;
		}
	}
}