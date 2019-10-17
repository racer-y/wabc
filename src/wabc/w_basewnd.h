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

#include "w_std.h"

namespace wabc
{
	SCROLLINFO get_scroll_info(HWND hWnd, int nBar, size_t nMask);
	BOOL set_LWA(HWND hWnd, DWORD key, BYTE bAlpha, DWORD dwFlags);

	void set_font_to_children(HWND hWnd, HFONT hFont, bool bRedraw);
	extern int (WINAPI *pfnMessageBox)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
	extern INT_PTR(WINAPI*pfnDialogBoxIndirectParam)(
		 HINSTANCE hInstance,
		LPCDLGTEMPLATEW hDialogTemplate,
		HWND hWndParent,
		DLGPROC lpDialogFunc,
		LPARAM dwInitParam);

	class dcclass;

	// --------------------------------------------------------------------

	class _basewnd
	{
	public:
		// --------------------------------------------------------------------

		template< size_t N, size_t Mask = 0>
		struct style_template : wabc::union_attr<_basewnd>
		{
			operator bool()const
			{
				const size_t n = _this->style();
				return (n & N) == N;
			}

			bool operator()()const { return *this; }

			style_template< N, Mask > & operator=(bool b)
			{
				size_t n = _this->style();
				if (b)
					n = (n & ~Mask) | N;
				else
					n &= ~(Mask | N);

				me()->style = n;
				return *this;
			}
		};

		// --------------------------------------------------------------------

		template< size_t N, size_t Mask = 0>
		struct exstyle_template  : wabc::union_attr<_basewnd>
		{
			operator bool()const
			{
				const size_t n = _this->exstyle();
				return (n & N) == N;
			}
			bool operator()()const { return *this; }

			exstyle_template< N, Mask > & operator=(bool b)
			{
				size_t n = _this->exstyle();
				if (b)
					n = (n & ~Mask) | N;
				else
					n &= ~(Mask | N);

				__this->exstyle = n;
				return *this;
			}
		};

		// --------------------------------------------------------------------

		struct union_property
		{
			HWND	m_hWnd;
		};

		// --------------------------------------------------------------------	

		struct text_t : union_property
		{
			size_t get(string &result)const;

			size_t get(string::pointer text, size_t n)
			{
				const size_t ret= ::GetWindowText(m_hWnd, text, int(n));
				return ret;
			}

			template<size_t N>
			size_t get(wchar_t (&text)[N])
			{
				const size_t ret= ::GetWindowText(m_hWnd, text, N);
				return ret;
			}

			operator string()const 
			{ 
				string s; 
				get(s); 
				return s; 
			}

			string operator()()const { return *this; }

			size_t size()const
			{
				const size_t n = ::GetWindowTextLength(m_hWnd);
				return n;
			}

			void operator=(const string &strText)
			{
				const BOOL result = ::SetWindowText(m_hWnd, strText.c_str());
			}

			void operator=(string::const_pointer szText)
			{
				BOOL result = ::SetWindowText(m_hWnd, szText);
			}
		};

		// --------------------------------------------------------------------

		struct timer_t : union_property
		{
			void enable(LONG ID, size_t nInterval)
			{
				INT_PTR n = ::SetTimer(m_hWnd, ID, UINT(nInterval), 0);
				assert(n == ID);
			}

			void disable(LONG ID)
			{
				::KillTimer(m_hWnd, ID);
			}
		};

		// --------------------------------------------------------------------

		template< size_t TimerId >
		struct timer_template : union_property
		{
			enum{ ID = TimerId };

			void enable(size_t nInterval) // million second
			{
				INT_PTR n = ::SetTimer(m_hWnd, ID, UINT(nInterval), 0);
				assert(n == ID);
				//api_check(n != 0, "SetTimer");
			}

			void disable()
			{
				::KillTimer(m_hWnd, ID);
			}
		};

		// --------------------------------------------------------------------

		template< size_t nIconType>
		struct msgbox_template : union_property
		{
			template<typename T1, typename T2>
			int yes_no(const T1 &strText, const T2 &strCaption, UINT nDef = 0)const
			{
				const int n = pfnMessageBox(m_hWnd, &(strText[0]), &(strCaption[0]),
					MB_YESNO | nIconType | (nDef * 16));
				return (n - IDYES);
			}

			template<typename T1, typename T2>
			int yes_no_cancel(const T1 &strText, const T2 &strCaption,
				UINT nDef = 0)const
			{
				const int n = pfnMessageBox(m_hWnd, &(strText[0]), &(strCaption[0]),
					MB_YESNOCANCEL | nIconType | (nDef * 16));
				return (n == IDCANCEL) ? 2 : (n - IDYES);
			}

			template<typename T1, typename T2>
			int retry_cancel(const T1 &strText, const T2 &strCaption,
				UINT nDef = 0) const
			{
				const int n = pfnMessageBox(m_hWnd, &(strText[0]), &(strCaption[0]),
					MB_RETRYCANCEL | nIconType | (nDef * 16));
				return (n == IDRETRY) ? 0 : 1;
			}

			template<typename T1, typename T2>
			int abort_retry_ignore(const T1 &strText, const T2 &strCaption,
				UINT nDef = 0)const
			{
				const int n = pfnMessageBox(m_hWnd, &(strText[0]), &(strCaption[0]),
					MB_ABORTRETRYIGNORE | nIconType | (nDef * 16));
				return (n - IDABORT);
			}

			template<typename T1, typename T2>
			int ok(const T1 &strText, const T2 &strCaption)const
			{
				const int n = pfnMessageBox(m_hWnd, &(strText[0]), &(strCaption[0]),
					MB_OK | nIconType);
				return (n - IDOK);
			}

			template<typename T1, typename T2>
			int ok_cancel(const T1 &strText, const T2 &strCaption,
				UINT nDef = 0)const
			{
				const int n = pfnMessageBox(m_hWnd, &(strText[0]), &(strCaption[0]),
					MB_OKCANCEL | nIconType | (nDef * 16));
				return (n - IDOK);
			}

			template<typename T>
			int yes_no(const T &strText, UINT nDef = 0)const
			{
				return yes_no(strText, cast<text_t>()(), nDef);
			}

			template<typename T>
			int yes_no_cancel(const T &strText, UINT nDef = 0)const
			{
				return yes_no_cancel(strText, cast<text_t>()(), nDef);
			}

			template<typename T>
			int retry_cancel(const T &strText, UINT nDef = 0) const
			{
				const string s= cast<text_t>();
				return retry_cancel(strText, s, nDef);
			}

			template<typename T>
			int abort_retry_ignore(const T &strText, UINT nDef = 0)const
			{
				const string s= cast<text_t>();
				return abort_retry_ignore(strText, s, nDef);
			}

			template<typename T>
			int ok(const T &strText)const
			{
				TCHAR szText[128];
				::GetWindowText(m_hWnd, szText, countof(szText));
				return ok(strText, szText);
			}

			template<typename T>
			int ok_cancel(const T &strText, UINT nDef = 0)const
			{
				TCHAR szText[128];
				::GetWindowText(m_hWnd, szText, countof(szText));
				return ok_cancel(strText, szText, nDef);
			}
		};

	public:

		typedef timer_template<0x80808081> timer1_t;
		typedef timer_template<0x80808082> timer2_t;
		typedef timer_template<0x80808083> timer3_t;
		typedef timer_template<0x80808084> timer4_t;
		typedef timer_template<0x80808085> timer5_t;
		typedef timer_template<0x80808086> timer6_t;
		typedef timer_template<0x80808087> timer7_t;
		typedef timer_template<0x80808088> timer8_t;
		typedef timer_template<0x80808089> timer9_t;

		// --------------------------------------------------------------------	

		union msgbox_t
		{
			msgbox_template< MB_ICONSTOP>			error;
			msgbox_template< MB_ICONWARNING>		warning;
			msgbox_template< MB_ICONQUESTION>		question;
			msgbox_template< MB_ICONINFORMATION>	information;

			void show_last_error(DWORD dwError)
			{
				const string s = last_error_to_str(dwError);
				information.ok(s, L"Last Error");
			}

			void show_last_error(){ show_last_error(::GetLastError()); }

			void show(const string & strText)
			{
				information.ok(strText);
			}

			void show(const string & strText, const string &strCaption)
			{
				information.ok(strText, strCaption);
			}
		};

		// --------------------------------------------------------------------	

		struct class_name_t : union_property
		{
			operator string()const;
			string operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		struct font_t : union_property
		{
			operator HFONT()const
			{
				HFONT hFont = (HFONT)::SendMessage(m_hWnd, WM_GETFONT, 0, 0);
				return hFont;
			}

			HFONT operator()()const { return *this; }

			font_t & operator=(HFONT hFont)
			{
				set(hFont); return *this;
			}

			void set(HFONT hFont, bool bRedraw = true)
			{
				::SendMessage(m_hWnd, WM_SETFONT, (WPARAM)hFont, LPARAM(bRedraw));
			}
		};

		// --------------------------------------------------------------------

		struct clipboard_t : union_property
		{
			bool get(string &s, size_t max_len=-1)const;
			bool get(string::pointer buf, size_t n)const;

			bool has_text()const;

			template<size_t N>
			bool get(wchar_t(&a)[N])
			{
				return get(a, N);
			}

			bool put(string::const_pointer lpszText, size_t n)const;

			bool put(const string &s)const
			{
				return put(s.c_str(), s.size());
			}
		};

		// --------------------------------------------------------------------	

		struct big_icon_t : union_property
		{
			HICON operator()()
			{
				return (HICON)::SendMessage(m_hWnd, WM_GETICON, ICON_BIG, 0);
			}

			HICON operator=(HICON hIcon)
			{
				LRESULT ret = ::SendMessage(m_hWnd, WM_SETICON, ICON_BIG, LPARAM(hIcon));
				return HICON(ret);
			}
		};

		// --------------------------------------------------------------------	

		struct small_icon_t : union_property
		{
			HICON operator()()
			{
				return (HICON)::SendMessage(m_hWnd, WM_GETICON, ICON_SMALL, 0);
			}

			HICON operator=(HICON hIcon)
			{
				LRESULT ret = ::SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, LPARAM(hIcon));
				return HICON(ret);
			}
		};

		// --------------------------------------------------------------------	

		struct client_rect_t : union_property
		{
			RECT & get(RECT &rt)const
			{
				const bool result = ::GetClientRect(m_hWnd, &rt) != 0;
				assert(result);
				return rt;
			}
			operator rect()const
			{
				RECT rt;
				return static_cast<rect &>(get(rt));
			}
			rect operator()()const { return *this; }

			LONG width()const { return operator()().width(); }
			LONG height()const { return operator()().height(); }
		};

		// --------------------------------------------------------------------	

		struct window_rect_t : union_property
		{
			RECT & get(RECT &rt)const
			{
				const bool result = ::GetWindowRect(m_hWnd, &rt) != 0;
				assert(result);
				return rt;
			}

			operator rect()const
			{
				RECT rt;
				return static_cast<rect &>(get(rt));
			}
			rect operator()()const { return *this; }

			LONG width()const { return operator()().width(); }
			LONG height()const { return operator()().height(); }
		};

		// --------------------------------------------------------------------

		struct style_t : union_property
		{
			operator DWORD()const
			{
				return ::GetWindowLongPtr(m_hWnd, GWL_STYLE);
			}

			DWORD operator()()const { return *this; }

			DWORD operator()(DWORD f)const
			{
				const DWORD dwStyle = ::GetWindowLongPtr(m_hWnd, GWL_STYLE);
				return dwStyle & f;
			}

			DWORD operator=(const DWORD & dwStyle)
			{
				::SetWindowLongPtr(m_hWnd, GWL_STYLE, dwStyle);
				::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				return dwStyle;
			}
		};

		// --------------------------------------------------------------------

		struct exstyle_t : union_property
		{
			operator DWORD()const
			{
				return ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
			}

			DWORD operator()()const { return *this; }

			DWORD operator()(DWORD f)const
			{
				const DWORD dwStyle = ::GetWindowLongPtr(m_hWnd, GWL_EXSTYLE);
				return dwStyle & f;
			}

			const DWORD & operator=(const DWORD & dwStyle)
			{
				::SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, dwStyle);
				::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				return dwStyle;
			}
		};

		// --------------------------------------------------------------------	

		struct visible_t : union_property
		{
			operator bool()const
			{
				return ::IsWindowVisible(m_hWnd) != 0;
			}
			bool operator()()const { return *this; }

			bool operator= (bool b)
			{
				::ShowWindow(m_hWnd, b ? SW_SHOW : SW_HIDE);
				return b;
			}
		};

		// --------------------------------------------------------------------	

		struct enabled_t : union_property
		{
			operator bool()const
			{
				return ::IsWindowEnabled(m_hWnd) != 0;
			}

			bool operator()()const { return *this; }

			bool operator=(bool b)
			{
				const bool result = ::EnableWindow(m_hWnd, b) != 0;
				return b;
			}
		};

		// --------------------------------------------------------------------	

		struct maximized_t : union_property
		{
			operator bool()const
			{
				return ::IsZoomed(m_hWnd) != FALSE;
			}

			bool operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		struct minimized_t : union_property
		{
			operator bool()const
			{
				return ::IsIconic(m_hWnd) != FALSE;
			}

			bool operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	
		// vscrollbar
		// --------------------------------------------------------------------	

		struct vscrollbar_track_pos_t : union_property
		{
			operator int()const
			{
				return get_scroll_info(m_hWnd, SB_VERT, SIF_TRACKPOS).nTrackPos;
			}

			int operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		struct vscrollbar_pos_t : union_property
		{
			operator int()const
			{
				return get_scroll_info(m_hWnd, SB_VERT, SIF_POS).nPos;
			}

			int operator()()const { return *this; }

			int operator=(int n);

			int operator+=(int n);
			int operator++() { return *this += 1; }

			int operator--() { return *this += -1; }
			int operator-=(int n) { return *this += -n; }
		};

		// --------------------------------------------------------------------	

		struct vscrollbar_page_t : union_property
		{
			operator int()const
			{
				return get_scroll_info(m_hWnd, SB_VERT, SIF_PAGE).nPage;
			}

			int operator()()const { return *this; }

			int operator=(int n);
		};

		// --------------------------------------------------------------------	

		struct vscrollbar_range_t : union_property
		{
			void set(int nMin, int nMax);
			void get(int &nMin, int &nMax);

			SCROLLINFO operator()()const
			{
				const SCROLLINFO si = get_scroll_info(m_hWnd, SB_VERT, SIF_RANGE);
				return si;
			}
		};

		// --------------------------------------------------------------------	

		struct vscrollbar_width_t : union_property
		{
			operator int()const
			{
				return GetSystemMetrics(SM_CXVSCROLL);
			}

			int operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		struct vscrollbar_height_t : union_property
		{
			operator int()const
			{
				return GetSystemMetrics(SM_CYVSCROLL);
			}

			int operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		union vscrollbar_t
		{
			vscrollbar_track_pos_t		track_pos;
			vscrollbar_pos_t			pos;
			vscrollbar_page_t			page;
			vscrollbar_range_t			range;

			vscrollbar_width_t			width;
			vscrollbar_height_t			height;

			void show(bool bShow = TRUE)
			{
				BOOL b = ::ShowScrollBar(width.m_hWnd, SB_VERT, bShow);
				//api_check(b != FALSE, "ShowScrollBar");
			}

			void hide() { show(FALSE); }

			SCROLLINFO operator()()const
			{ 
				return get_scroll_info(range.m_hWnd, SB_VERT, SIF_ALL);
			}
		};

		// --------------------------------------------------------------------	
		// hscrollbar
		// --------------------------------------------------------------------	

		struct hscrollbar_track_pos_t : union_property
		{
			operator int()const
			{
				return get_scroll_info(m_hWnd, SB_HORZ, SIF_TRACKPOS).nTrackPos;
			}

			int operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		struct hscrollbar_pos_t : union_property
		{
			operator int()const
			{
				return get_scroll_info(m_hWnd, SB_HORZ, SIF_POS).nPos;
			}

			int operator()()const { return *this; }

			int operator=(int n);

			int operator+=(int n);
			int operator++() { return *this += 1; }

			int operator--() { return *this += -1; }
			int operator-=(int n){ return *this += -n; }
		};

		// --------------------------------------------------------------------	

		struct hscrollbar_page_t : union_property
		{
			operator int()const
			{
				return get_scroll_info(m_hWnd, SB_HORZ, SIF_PAGE).nPage;
			}

			int operator()()const { return *this; }

			int operator=(int n);
		};

		// --------------------------------------------------------------------	

		struct hscrollbar_range_t : union_property
		{
			void set(int nMin, int nMax);
			void get(int &nMin, int &nMax);

			SCROLLINFO operator()()const
			{
				const SCROLLINFO si = get_scroll_info(m_hWnd, SB_HORZ, SIF_RANGE);
				return si;
			}
		};

		// --------------------------------------------------------------------	

		struct hscrollbar_width_t : union_property
		{
			operator int()const
			{
				return GetSystemMetrics(SM_CXHSCROLL);
			}

			int operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		struct hscrollbar_height_t : union_property
		{
			operator int()const
			{
				return GetSystemMetrics(SM_CYHSCROLL);
			}

			int operator()()const { return *this; }
		};

		// --------------------------------------------------------------------	

		union hscrollbar_t
		{
			hscrollbar_track_pos_t		track_pos;
			hscrollbar_pos_t			pos;
			hscrollbar_page_t			page;
			hscrollbar_range_t			range;

			hscrollbar_width_t			width;
			hscrollbar_height_t			height;

			void show(bool bShow = TRUE)
			{
				BOOL b = ::ShowScrollBar(pos.m_hWnd, SB_HORZ, bShow);
				//api_check(b != FALSE, "ShowScrollBar");
			}

			void hide() { show(FALSE); }

			SCROLLINFO operator()()const
			{ 
				return get_scroll_info(range.m_hWnd, SB_HORZ, SIF_ALL);
			}
		};

		// --------------------------------------------------------------------	

		struct alpha_t : union_property
		{
			bool set(unsigned char a)
			{ 
				return set_LWA(m_hWnd, 0, a, LWA_ALPHA) != FALSE; 
			}
			void operator=(unsigned char a) { set(a); }
		};

		// --------------------------------------------------------------------	

		struct color_key_t : union_property
		{
			bool set(DWORD key, unsigned char a){
				return set_LWA(m_hWnd, key, a, LWA_COLORKEY) != FALSE;
			}

			void operator=(DWORD key) { set(key, 0); }
		};

		// --------------------------------------------------------------------	

		union layer_t
		{
			alpha_t	alpha;
			color_key_t color_key;
		};

	public:

		union
		{
			HWND	m_hWnd;
			text_t			text;
			font_t			font;
			text_t			caption;
			msgbox_t		msgbox;
			style_t			style;
			exstyle_t		exstyle; 
			enabled_t		enabled;
			visible_t		visible;
			big_icon_t		big_icon;
			small_icon_t	small_icon;
			layer_t			layer;
			timer_t			timer;
			timer1_t		timer1;
			timer2_t		timer2;
			timer3_t		timer3;
			timer4_t		timer4;
			timer5_t		timer5;
			timer6_t		timer6;
			timer7_t		timer7;
			timer8_t		timer8;
			timer9_t		timer9;
			clipboard_t		clipboard;
			hscrollbar_t	hscrollbar;
			vscrollbar_t	vscrollbar;

			client_rect_t	client_rect;
			window_rect_t	window_rect;
			maximized_t	maximized;
			minimized_t	minimized;
			class_name_t	class_name;
		};

	public:
		typedef _basewnd self;

		typedef wabc::string string;
		typedef wabc::rect rect;
		typedef wabc::dcclass dcclass;

		operator HWND()const { return m_hWnd; }

		void show(int nCmdShow = SW_SHOW)
		{
			::ShowWindow(m_hWnd, nCmdShow);
		}

		void close()
		{
			::CloseWindow(m_hWnd);
		}

		void maximize()
		{
			::ShowWindow(m_hWnd, SW_MAXIMIZE);
		}

		void minimize()
		{
			::ShowWindow(m_hWnd, SW_MINIMIZE);
		}

		void restore()
		{
			::ShowWindow(m_hWnd, SW_RESTORE);
		}

		void hide()
		{
			::ShowWindow(m_hWnd, SW_HIDE);
		}

		void update()
		{
			::UpdateWindow(m_hWnd);
		}

		void invalidate()
		{
			::InvalidateRect(m_hWnd, 0, TRUE);
		}

		void invalidate(const RECT &rt, BOOL bEraseBk = TRUE)
		{
			::InvalidateRect(m_hWnd, &rt, bEraseBk);
		}

		void validate()
		{
			::ValidateRect(m_hWnd, 0);
		}

		void move(LONG x, LONG y)
		{
			const BOOL result = ::SetWindowPos(m_hWnd, HWND_TOP, x, y, 0, 0,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE);
			assert(result != FALSE);
		}

		void resize(LONG w, LONG h)
		{
			BOOL result = ::SetWindowPos(m_hWnd, HWND_TOP, 0, 0, w, h,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOMOVE);
			//api_check(result, "SetWindowPos");
		}

		void move_and_resize(LONG x, LONG y, LONG nWidth, LONG nHeight)
		{
			bool result = ::SetWindowPos(m_hWnd, HWND_TOP, x, y, nWidth, nHeight,
				SWP_NOACTIVATE | SWP_NOOWNERZORDER) != 0;
			//api_check(result, "SetWindowPos");
		}

		void move(const RECT & rt)
		{
			move_and_resize(rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top);
		}

		void center_screen()
		{
			wabc::center_screen(m_hWnd);
		}

		void set_focus() 
		{ ::SetFocus(m_hWnd); }

		POINT & screen_to_client(POINT &pt)const
		{
			::ScreenToClient(m_hWnd, &pt);
			return pt;
		}

		POINT & client_to_screen(POINT &pt)const
		{
			::ClientToScreen(m_hWnd, &pt);
			return pt;
		}

		rect & screen_to_client(RECT & rt)const
		{
			::ScreenToClient(m_hWnd, reinterpret_cast<POINT *>(&rt.left));
			::ScreenToClient(m_hWnd, reinterpret_cast<POINT *>(&rt.right));
			return static_cast<rect &>(rt);
		}

		rect & client_to_screen(RECT & rt)const
		{
			::ClientToScreen(m_hWnd, reinterpret_cast<POINT *>(&rt.left));
			::ClientToScreen(m_hWnd, reinterpret_cast<POINT *>(&rt.right));
			return static_cast<rect &>(rt);
		}		
	};

	// --------------------------------------------------------------------

	class basewnd : public _basewnd
	{
	public:
		basewnd() { m_hWnd = 0; }
		virtual ~basewnd(){}
	};
}