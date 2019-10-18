#include "w_std.h"
#include "w_vmem.h"
#include <assert.h>

namespace wabc
{
	static inline void move_and_resize(HWND hWnd, LONG x, LONG y, LONG w, LONG h)
	{
		const BOOL ret = ::SetWindowPos(hWnd, HWND_TOP, x, y, w, h,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER);
	}

	static vmem & instance()
	{
		static vmem g_vmem;
		return g_vmem;
	}

	// --------------------------------------------------------------------

	void * alloc(size_t new_size)
	{
		return instance().alloc(new_size);
	}

	// --------------------------------------------------------------------

	void * realloc(void *p, size_t new_size)
	{
		return instance().realloc(p, new_size);
	}

	// --------------------------------------------------------------------

	void free(void *p)
	{
		instance().free(p);
	}

	// --------------------------------------------------------------------

	string last_error_to_str(DWORD dwError)
	{
		// --------------------------------------------------------------------	

		struct helper
		{
			LPVOID lpMsgBuf;

			helper(DWORD dwError)
			{
				::FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER |
					FORMAT_MESSAGE_FROM_SYSTEM |
					FORMAT_MESSAGE_IGNORE_INSERTS,
					NULL,
					dwError,
					0, // Default language
					(LPTSTR)&lpMsgBuf,
					0,
					NULL
					);
			}

			~helper()
			{
				::LocalFree(lpMsgBuf);
			}

		} h(dwError);

		return h.lpMsgBuf ? string((LPTSTR)h.lpMsgBuf) : string();
	}

	// --------------------------------------------------------------------


	// --------------------------------------------------------------------
	// strcut rect
	// --------------------------------------------------------------------

	bool rect::intersect(const rect &rhs)const 
	{
	if (right <= rhs.left)
	return false;

	if (rhs.right <= left)
		return false;

	if (bottom <= rhs.top)
		return false;

	if (rhs.bottom <= top)
		return false;

	return true;
}

	// --------------------------------------------------------------------

	void rect::normalize()
	{
		if (left > right)
		std::swap(left, right);

		if (top > bottom)
			std::swap(top, bottom);
	}

	// --------------------------------------------------------------------

	rect & rect::operator+=(LONG n)
	{
		__wabc_static_assert(sizeof(LONG)* 4 == sizeof(rect));
		LONG *p = reinterpret_cast<LONG *>(this);
		for (size_t i = 0; i < 4; ++i)
			p[i] += n;
		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::operator&=(const RECT &rhs) 
	{
		if (left < rhs.left)
		left = rhs.left;

		if (top < rhs.top)
			top = rhs.top;

		if (right > rhs.right)
			right = rhs.right;

		if (bottom > rhs.bottom)
			bottom = rhs.bottom;
		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::operator|=(const RECT &rhs) 
	{
		if (left > rhs.left)
		left = rhs.left;

		if (top > rhs.top)
			top = rhs.top;

		if (right < rhs.right)
			right = rhs.right;

		if (bottom < rhs.bottom)
			bottom = rhs.bottom;

		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::inflate(LONG n) 
	{
		left -= n; top -= n;
		right += n; bottom += n;
		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::inflate(LONG nx, LONG ny) 
	{
		left -= nx; top -= ny;
		right += nx; bottom += ny;
		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::deflate(LONG n) 
	{
		left += n; top += n;
		right -= n; bottom -= n;

		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::deflate(LONG nx, LONG ny) 
	{
		left += nx; top += ny;
		right -= nx; bottom -= ny;

		return *this;
	}

	// --------------------------------------------------------------------

	rect & rect::offset(LONG cx, LONG cy) 
	{
		left += cx; top += cy;
		right += cx; bottom += cy;

		return *this;
	}
	// --------------------------------------------------------------------
	// layout
	// --------------------------------------------------------------------

	layout::layout(HWND hWnd)
	{
		const BOOL b = ::GetClientRect(hWnd, &m_bound);
		assert(b != FALSE);
	}

	// --------------------------------------------------------------------

	layout::layout(LONG w, LONG h)
	{
		m_bound.left = 0;
		m_bound.right = w;
		m_bound.top = 0;
		m_bound.bottom = h;
	}

	// --------------------------------------------------------------------

	layout::layout(LONG left, LONG top, LONG right, LONG bottom)
	{
		m_bound.left = left;
		m_bound.right = right;
		m_bound.top = top;
		m_bound.bottom = bottom;
	}

	// --------------------------------------------------------------------

	layout & layout::align_top(HWND hWnd)
	{
		RECT &rt = m_bound;
		RECT rtWnd;
		::GetWindowRect(hWnd, &rtWnd);

		move_and_resize(hWnd, rt.left, rt.top, rt.right - rt.left, rtWnd.bottom - rtWnd.top);

		rt.top += (rtWnd.bottom - rtWnd.top);
		if (rt.top > rt.bottom)
			rt.top = rt.bottom;
		return *this;
	}

	// --------------------------------------------------------------------

	layout & layout::align_bottom(HWND hWnd)
	{
		RECT &rt = m_bound;
		RECT rtWnd;
		::GetWindowRect(hWnd, &rtWnd);

		LONG top = rt.bottom - (rtWnd.bottom - rtWnd.top);
		if (top < rt.top)
			top = rt.top;

		move_and_resize(hWnd, rt.left, top, rt.right - rt.left, rtWnd.bottom - rtWnd.top);

		rt.bottom = top;
		return *this;
	}

	// --------------------------------------------------------------------

	layout & layout::align_left(HWND hWnd)
	{
		RECT &rt = m_bound;
		RECT rtWnd;
		::GetWindowRect(hWnd, &rtWnd);

		move_and_resize(hWnd, rt.left, rt.top, rtWnd.right - rtWnd.left, rt.bottom - rt.top);
		rt.left += (rtWnd.right - rtWnd.left);
		if (rt.left > rt.right)
			rt.left = rt.right;
		return *this;
	}

	// --------------------------------------------------------------------

	layout & layout::align_right(HWND hWnd)
	{
		RECT &rt = m_bound;
		RECT rtWnd;
		::GetWindowRect(hWnd, &rtWnd);

		LONG left = rt.right - (rtWnd.right - rtWnd.left);
		if (left < rt.left)
			left = rt.left;

		move_and_resize(hWnd, left, rt.top, rtWnd.right - rtWnd.left, rt.bottom - rt.top);
		rt.right = left;
		return *this;
	}

	// --------------------------------------------------------------------

	layout & layout::align_client(HWND hWnd)
	{
		move_and_resize(hWnd, m_bound.left, m_bound.top, m_bound.right - m_bound.left, m_bound.bottom - m_bound.top);
		m_bound.left = m_bound.right;
		m_bound.top = m_bound.bottom;
		return *this;
	}

	// --------------------------------------------------------------------

	void center_screen(HWND hWnd)
	{
		RECT rt;
		::GetWindowRect(hWnd, &rt);

		const LONG w = rt.right - rt.left;
		const LONG h = rt.bottom - rt.top;
		const LONG sx = ::GetSystemMetrics(SM_CXSCREEN);
		const LONG sy = ::GetSystemMetrics(SM_CYSCREEN);

		const bool result = ::SetWindowPos(hWnd, HWND_TOP, (sx - w) / 2, (sy - h) / 2, 0, 0,
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE) != 0;
	}

	// --------------------------------------------------------------------

	void throw_win32_exception(const wchar_t *lpsz)
	{
		assert(false);
	}
}