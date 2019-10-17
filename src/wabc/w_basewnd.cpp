#include "w_basewnd.h"

namespace wabc
{
	int (WINAPI *pfnMessageBox)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) = &MessageBox;
	INT_PTR(WINAPI*pfnDialogBoxIndirectParam)(
		HINSTANCE hInstance,
		LPCDLGTEMPLATEW hDialogTemplate,
		HWND hWndParent,
		DLGPROC lpDialogFunc,
		LPARAM dwInitParam) = &DialogBoxIndirectParam;

	// --------------------------------------------------------------------

	SCROLLINFO get_scroll_info(HWND hWnd, int nBar, size_t nMask)
	{
		SCROLLINFO si = { 0 };
//		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = UINT(nMask);

		::GetScrollInfo(hWnd, nBar, &si);
		return si;
	}

	// --------------------------------------------------------------------

	void set_font_to_children(HWND hWnd, HFONT hFont, bool bRedraw)
	{
		struct find_window_and_set_font
		{
			typedef find_window_and_set_font self;

			const HFONT	m_hFont;
			const bool	m_redraw;

			find_window_and_set_font(HWND hWnd, HFONT hFont, bool bRedraw)
				: m_hFont(hFont), m_redraw(bRedraw)
			{
				::EnumChildWindows(hWnd, enum_window, LPARAM(this));
			}

			static BOOL CALLBACK enum_window(HWND hWnd, LPARAM lParam)
			{
				self &tmp = *reinterpret_cast<self *>(lParam);
				::SendMessage(hWnd, WM_SETFONT, WPARAM(tmp.m_hFont), tmp.m_redraw);
				return TRUE;
			}

		}_find_window_and_set_font(hWnd, hFont, bRedraw);
	}

	// --------------------------------------------------------------------

	BOOL set_LWA(HWND hWnd, DWORD key, BYTE bAlpha, DWORD dwFlags)
	{
#if(_MSC_VER <= 1400)
		//		const DWORD LWA_COLORKEY= 0x00000001;
		//		const DWORD LWA_ALPHA= 0x00000002;
		//		const DWORD WS_EX_LAYERED= 0x00080000;
#endif

		typedef BOOL(FAR PASCAL * FUNC1)(
			HWND hwnd,           // handle to the layered window
			COLORREF crKey,      // specifies the color key
			BYTE bAlpha,         // value for the blend function
			DWORD dwFlags        // action
			);

		HMODULE hModule = GetModuleHandle(_T("user32.dll"));
		FUNC1 SetLayeredWindowAttributes;
		if (hModule)
		{
			SetLayeredWindowAttributes = (FUNC1)GetProcAddress(hModule, "SetLayeredWindowAttributes");

			if (SetLayeredWindowAttributes)
			{
				// 设置分层扩展标记
				const DWORD dwExStyle = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED;
				::SetWindowLongPtr(hWnd, GWL_EXSTYLE, dwExStyle);
				return SetLayeredWindowAttributes(hWnd, key, bAlpha, dwFlags);
			}
		}
		return FALSE;
	}

	// --------------------------------------------------------------------
	// class _basewnd::text_t
	// --------------------------------------------------------------------	

	size_t _basewnd::text_t::get(string & strText)const
	{
		const size_t n = ::GetWindowTextLength(m_hWnd);
		strText.resize(n);
		const size_t ret = ::GetWindowText(m_hWnd, const_cast<string::pointer>(&strText[0]), int(n + 1));
		return ret;
	}

	// --------------------------------------------------------------------
	// class _basewnd::class_name_t
	// --------------------------------------------------------------------	

	_basewnd::class_name_t::operator string()const
	{
		string s(255, 0);

		const size_t n = ::GetClassName(m_hWnd, &s[0], 256);

		s.resize(n);
		return s;
	}

	// --------------------------------------------------------------------
	// _basewnd::vscrollbar_pos_t
	// --------------------------------------------------------------------	

	int _basewnd::vscrollbar_pos_t::operator=(int n)
	{
		SCROLLINFO si;
		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = n;

		::SetScrollInfo(m_hWnd, SB_VERT, &si, true);
		return n;
	}

	// --------------------------------------------------------------------

	int _basewnd::vscrollbar_pos_t::operator+=(int n)
	{
		SCROLLINFO si = get_scroll_info(m_hWnd, SB_VERT, SIF_POS);
		si.nPos += n;
		::SetScrollInfo(m_hWnd, SB_VERT, &si, true);
		return si.nPos;
	}

	// --------------------------------------------------------------------
	// class _basewnd::vscrollbar_page_t
	// --------------------------------------------------------------------	

	int _basewnd::vscrollbar_page_t::operator=(int n)
	{
		SCROLLINFO si;
		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE;
		si.nPage = n;

		::SetScrollInfo(m_hWnd, SB_VERT, &si, true);
		return n;
	}

	// --------------------------------------------------------------------
	// class _basewnd::vscrollbar_range_t
	// --------------------------------------------------------------------	

	void _basewnd::vscrollbar_range_t::set(int nMin, int nMax)
	{
		SCROLLINFO si;
		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE;
		si.nMin = nMin;
		si.nMax = nMax;

		::SetScrollInfo(m_hWnd, SB_VERT, &si, true);
	}

	// --------------------------------------------------------------------

	void _basewnd::vscrollbar_range_t::get(int &nMin, int &nMax)
	{
		const SCROLLINFO si = get_scroll_info(m_hWnd, SB_VERT, SIF_RANGE);
		nMin = si.nMin;
		nMax = si.nMax;
	}


	// --------------------------------------------------------------------
	// class _basewnd::hscrollbar_pos_t
	// --------------------------------------------------------------------	

	int _basewnd::hscrollbar_pos_t::operator=(int n)
	{
		SCROLLINFO si;
		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		si.nPos = n;

		::SetScrollInfo(m_hWnd, SB_HORZ, &si, true);
		return n;
	}

	// --------------------------------------------------------------------

	int _basewnd::hscrollbar_pos_t::operator+=(int n)
	{
		SCROLLINFO si = get_scroll_info(m_hWnd, SB_HORZ, SIF_POS);
		si.nPos += n;
		::SetScrollInfo(m_hWnd, SB_HORZ, &si, true);
		return si.nPos;
	}

	// --------------------------------------------------------------------
	// class _basewnd::hscrollbar_range_t
	// --------------------------------------------------------------------	

	void _basewnd::hscrollbar_range_t::set(int nMin, int nMax)
	{
		SCROLLINFO si;
		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE;
		si.nMin = nMin;
		si.nMax = nMax;

		::SetScrollInfo(m_hWnd, SB_HORZ, &si, true);
	}

	// --------------------------------------------------------------------

	void _basewnd::hscrollbar_range_t::get(int &nMin, int &nMax)
	{
		const SCROLLINFO si = get_scroll_info(m_hWnd, SB_HORZ, SIF_RANGE);
		nMin = si.nMin;
		nMax = si.nMax;
	}

	// --------------------------------------------------------------------
	// class _basewnd::hscrollbar_page_t
	// --------------------------------------------------------------------	

	int _basewnd::hscrollbar_page_t::operator=(int n)
	{
		SCROLLINFO si;
		::memset(&si, 0, sizeof(si));

		si.cbSize = sizeof(si);
		si.fMask = SIF_PAGE;
		si.nPage = n;

		::SetScrollInfo(m_hWnd, SB_HORZ, &si, true);
		return n;
	}

	// --------------------------------------------------------------------
	// class _basewnd::clipboard_t
	// --------------------------------------------------------------------	

	bool _basewnd::clipboard_t::put(string::const_pointer lpszText, size_t n)const
	{
		if (n > 0)
		{
			HGLOBAL	hGlobal = ::GlobalAlloc(GHND | GMEM_SHARE,
				(n + 1) * sizeof(lpszText[0]));

			bool result = hGlobal != 0;
			if (hGlobal != 0)
			{
				string::pointer lpsz = (string::pointer)::GlobalLock(hGlobal);

				if (!lpsz)
				{
					__wabc_trace(L"can't lock memory(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
					::GlobalFree(hGlobal);
					return false;
				}

				::memcpy(lpsz, lpszText, n  * sizeof(lpszText[0]));
				lpsz[n] = 0;
				::GlobalUnlock(hGlobal);

				if (::OpenClipboard(m_hWnd))
				{
					::EmptyClipboard();
					::SetClipboardData(CF_UNICODETEXT, hGlobal);
					::CloseClipboard();
					return true;
				}
				else
				{
					__wabc_trace(L"can't open clipboard(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
					::GlobalFree(hGlobal);
				}
			}
			else
				__wabc_trace(L"can't allocate memory(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
		}
		return false;
	}

	// --------------------------------------------------------------------	

	bool _basewnd::clipboard_t::has_text()const
	{
		if (::OpenClipboard(m_hWnd))
		{
			HGLOBAL hGlobal = ::GetClipboardData(CF_UNICODETEXT);
			::CloseClipboard();
			return hGlobal != 0;
		}
		return false;
	}

	// --------------------------------------------------------------------	

	bool _basewnd::clipboard_t::get(string &s, size_t max_len)const
	{
		if (::OpenClipboard(m_hWnd))
		{
			HGLOBAL hGlobal = ::GetClipboardData(CF_UNICODETEXT);

			const bool result = hGlobal != 0;
			if (hGlobal != 0)
			{
				LPVOID p = ::GlobalLock(hGlobal);

				size_t n = _min(max_len, size_t(::GlobalSize(hGlobal) / sizeof(s[0])));
				if (n > 0)
				{
					while (string::const_pointer(p)[n - 1] == 0)
						--n;
					if (n)
						s.assign(string::const_pointer(p), n);
				}
				::GlobalUnlock(hGlobal);
			}
			::CloseClipboard();
			return result;
		}
		else
		{
			__wabc_trace(L"can't open clipboard(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
			return false;
		}

	}
	// --------------------------------------------------------------------	

	bool _basewnd::clipboard_t::get(string::pointer buf, size_t n)const
	{
		if (n == 0)
			return true;

		if (::OpenClipboard(m_hWnd))
		{
			HGLOBAL hGlobal = ::GetClipboardData(CF_UNICODETEXT);

			bool result = hGlobal != 0;
			if (hGlobal != 0)
			{
				LPVOID p = ::GlobalLock(hGlobal);
				if (p)
				{
					size_t len = ::GlobalSize(hGlobal) / sizeof(buf[0]);
					if (len > (n - 1))
						len = n - 1;
					::memcpy(buf, p, len * sizeof(buf[0]));
					buf[len] = 0;

					::GlobalUnlock(hGlobal);
				}
				else
					result = false;
			}
			::CloseClipboard();
			return result;
		}
		else
		{
			__wabc_trace(L"can't open clipboard(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
			return false;
		}
	}
}
