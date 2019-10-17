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

#include "w_wndbase.h"
//#include "w_typetraits.h"

#define WABC_DECLARE_MSG_MAPEX(N)	private: wabc::mapslot	m_mapslot_wabc[N];
#define WABC_DECLARE_MSG_MAP()	WABC_DECLARE_MSG_MAPEX(1)

#define WABC_BEGIN_MSG_MAP(thisClass) \
	{ typedef thisClass map_class; \
	static wabc::msgmap_t entries[] = {

#define WABC_END_MSG_MAPEX(mapTo, mapFrom, slotIndex) \
	};  \
	enum{ slot_count= sizeof(m_mapslot_wabc)/sizeof(m_mapslot_wabc[0]) }; \
	__wabc_static_assert(slotIndex < slot_count); \
	(mapTo).map_msg<map_class>(m_mapslot_wabc + (slotIndex), &(mapFrom), entries, countof(entries)); }

#define WABC_END_MSG_MAP() WABC_END_MSG_MAPEX(*this, *this, 0)

#define WABC_DECLARE_MSG_PREVIEWEX(N)	private: friend wabc::wndbase; wabc::mapslot	m_previewslot_wabc[N];
#define WABC_DECLARE_MSG_PREVIEW() WABC_DECLARE_MSG_PREVIEWEX(1)

#define WABC_BEGIN_MSG_PREVIEW(thisClass) WABC_BEGIN_MSG_MAP(thisClass)

#define WABC_END_MSG_PREVIEWEX(mapFrom, mapTo, slotIndex) \
	};  \
enum{ slot_count = sizeof(m_mapslot_wabc) / sizeof(m_mapslot_wabc[0]) }; \
	__wabc_static_assert(slotIndex < slot_count); \
	(mapFrom).preview_msg<map_class>((mapFrom).m_previewslot_wabc + (slotIndex), &(mapTo), entries, countof(entries)); }

#define WABC_END_MSG_PREVIEW() WABC_END_MSG_PREVIEWEX(*this, *this, 0)

namespace wabc
{
	class dcclass;

	// --------------------------------------------------------------------

	template <typename T>
	struct stdmsg_template
	{
		inline bool inherited(){ return reinterpret_cast<_msg_struct &>(*this).inherited(); }

		operator msg_struct &() {
			__wabc_static_assert(sizeof(T) == sizeof(msg_struct));
			return reinterpret_cast<_msg_struct &>(*this);
		}
		operator const msg_struct &()const {
			__wabc_static_assert(sizeof(T) == sizeof(msg_struct));
			return reinterpret_cast<const _msg_struct &>(*this);
		}
	};

	// --------------------------------------------------------------------

	struct msg_create
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;

		union
		{
			LPCREATESTRUCT	lpcs;
			LPARAM	lParam;
		};

		LRESULT result;
	};

	struct map_create : msgmap_t
	{
		typedef msg_create msg_type;

		static bool on_map(const msgmap_t &a, void * _this, msg_struct &);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_create &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef msg_struct msg_destroy;

	struct map_destroy : msgmap_t
	{
		typedef msg_destroy msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_size : stdmsg_template<msg_size>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;

		union
		{
			LPARAM	lParam;

			struct
			{
				WORD	width;
				WORD	height;
			};
		};

		LRESULT result;

		bool is_changed()const { return wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED; }
	};

	struct map_size : msgmap_t
	{
		typedef msg_size msg_type;

		static bool on_map(const msgmap_t &a, void * _this, msg_struct &);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_setfocus : stdmsg_template<msg_setfocus>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		union
		{
			HWND	lost_focus_wnd;	// for WM_SETFOCUS
			HWND	recv_focus_wnd;	// for WM_KILLFOCUS
		};

		LPARAM	lParam;

		LRESULT result;		
	};

	struct map_setfocus : msgmap_t
	{
		typedef msg_setfocus msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef msg_setfocus	msg_killfocus;
	typedef map_setfocus	map_killfocus;

	// --------------------------------------------------------------------

	struct msg_enable : stdmsg_template<msg_enable>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	is_enabled; //  TRUE or FALSE
		LPARAM	lParam;		// not used

		LRESULT result;
	};

	struct map_enable: msgmap_t
	{
		typedef msg_enable msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_settext : stdmsg_template<msg_settext>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;		// not used
		const wchar_t * text;	// null-terminated string

		LRESULT result;
	};

	struct map_settext : msgmap_t
	{
		typedef msg_settext msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_gettext : stdmsg_template<msg_gettext>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	text_size;	// including the terminating null character
		wchar_t * text;		// null-terminated string

		LRESULT result;	// The return value is the number of TCHARs copied, not including the terminating null character		
	};

	struct map_gettext : msgmap_t
	{
		typedef msg_gettext msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_gettextlength : stdmsg_template<msg_gettextlength>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;		// not used
		LPARAM	lParam;		// not used

		LRESULT result;	// The return value is the number of TCHARs copied, not including the terminating null character		
	};

	struct map_gettextlength : msgmap_t
	{
		typedef msg_gettextlength msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef msg_struct msg_paint;

	struct map_paint : msgmap_t
	{
		typedef msg_paint msg_type;

		static bool on_map(const msgmap_t &a, void * _this, msg_struct &);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(dcclass &, const rect &, msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct map_paint32 : msgmap_t
	{
		typedef msg_paint msg_type;

		static bool on_map(const msgmap_t &a, void * _this, msg_struct &);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(dcclass &, const rect &, msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct map_draw : msgmap_t
	{
		typedef msg_struct msg_type;

		static bool on_map(const msgmap_t &a, void * _this, msg_struct &);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(dcclass &, const rect &, msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef msg_struct msg_close;

	struct map_close : msgmap_t
	{
		typedef msg_close msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_erasebkgnd : stdmsg_template<msg_erasebkgnd>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		HDC		hdc;
		LPARAM	lParam;		// not used;

		LRESULT result;	// An application should return nonzero if it erases the background; otherwise, it should return zero. 		
	};

	struct map_erasebkgnd : msgmap_t
	{
		typedef msg_erasebkgnd msg_type;

		static bool not_erase_background(const msgmap_t &a, void * _this, msg_struct &);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_setcursor : stdmsg_template<msg_setcursor>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		HWND	cursor_wnd;	// Handle to the window that contains the cursor. 

		WORD	hit_test_code;
		WORD	id_of_mouse_msg;

		LRESULT result;	// an application processes this message, it should return TRUE to halt further processing or FALSE to continue.
	};

	struct map_setcursor : msgmap_t
	{
		typedef msg_setcursor msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_mouseactive : stdmsg_template<msg_mouseactive>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		HWND	active_wnd;	// A handle to the top-level parent window of the window being activated 

		WORD	hit_test_code;
		WORD	id_of_mouse_msg;

		LRESULT result;	// MA_ACTIVATE,MA_ACTIVATEANDEAT,MA_NOACTIVATE,MA_NOACTIVATEANDEAT.
	};

	struct map_mouseactive : msgmap_t
	{
		typedef msg_mouseactive msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_getminmaxinfo : stdmsg_template<msg_getminmaxinfo>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;	// not used
		LPMINMAXINFO	lpmmi;

		LRESULT result;	// If an application processes this message, it should return zero.
	};

	struct map_getminmaxinfo : msgmap_t
	{
		typedef msg_getminmaxinfo msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_drawitem : stdmsg_template<msg_drawitem>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	id_of_sender;

		LPDRAWITEMSTRUCT	lpDrawItem;

		LRESULT result;
	};

	struct map_drawitem : msgmap_t
	{
		typedef msg_drawitem msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_measureitem : stdmsg_template<msg_measureitem>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	id_of_sender;

		LPMEASUREITEMSTRUCT	lpItem;

		LRESULT result;
	};

	struct map_measureitem : msgmap_t
	{
		typedef msg_measureitem msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_setfont : stdmsg_template<msg_setfont>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		HFONT	hFont;

		LPARAM	bRedraw;	// TRUE or FALSE

		LRESULT result;
	};

	struct map_setfont : msgmap_t
	{
		typedef msg_setfont msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_getfont : stdmsg_template<msg_getfont>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;		// not used
		LPARAM	lParam;		// not used

		HFONT result;
	};

	struct map_getfont : msgmap_t
	{
		typedef msg_getfont msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_poschanging : stdmsg_template<msg_poschanging>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;		// not used
		LPWINDOWPOS pwp;

		HFONT result;
	};

	struct map_windowposchanging : msgmap_t
	{
		typedef msg_poschanging msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};


	// --------------------------------------------------------------------

	typedef msg_create msg_nccreate;
	typedef map_create map_nccreate;

	// --------------------------------------------------------------------

	typedef msg_destroy msg_ncdestroy;
	typedef map_destroy map_ncdestroy;

	// --------------------------------------------------------------------

	struct msg_nchittest : stdmsg_template<msg_nchittest>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM 	wParam;
		short int	x;
		short int	y;

		LRESULT result;	// HTCAPTION
	};

	struct map_nchittest : msgmap_t
	{
		typedef msg_nchittest msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_getdlgcode : stdmsg_template<msg_getdlgcode>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;		// not used
		MSG		*lpMSG;

		LRESULT result;
	};

	struct map_getdlgcode : msgmap_t
	{
		typedef msg_getdlgcode msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_setscrollinfo : stdmsg_template<msg_setscrollinfo>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	is_redrawn;
		const SCROLLINFO 		*si;

		LRESULT result;
	};

	struct map_setscrollinfo : msgmap_t
	{
		typedef msg_setscrollinfo msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_getscrollinfo : stdmsg_template<msg_getscrollinfo>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	wParam;
		SCROLLINFO 		*si;

		LRESULT result;
	};

	struct map_getscrollinfo : msgmap_t
	{
		typedef msg_getscrollinfo msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_key : stdmsg_template<msg_key>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	key_code;
		LPARAM	repeat_count : 16;
		LPARAM	scan_code : 8;
		LPARAM	is_extended_key : 1;
		LPARAM	reserve : 4;
		LPARAM	context_code : 1;			// The value is always 0 for a WM_KEYUP message
		LPARAM	previous_key_state : 1;		// The value is always 1 for a WM_KEYUP message
		LPARAM	transition_state : 1;		// The value is always 1 for a WM_KEYUP message

		LRESULT result;
	};

	typedef msg_key msg_keydown;
	struct map_keydown : msgmap_t
	{
		typedef msg_keydown msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef msg_keydown msg_keyup;
	typedef map_keydown map_keyup;

	// --------------------------------------------------------------------

	typedef msg_keydown msg_char;
	typedef map_keydown map_char;

	// --------------------------------------------------------------------

	typedef msg_keydown msg_syskeydown;
	typedef map_keydown map_syskeydown;

	// --------------------------------------------------------------------

	typedef msg_keydown msg_syskeyup;
	typedef map_keydown map_syskeyup;

	// --------------------------------------------------------------------

	typedef msg_keydown msg_syschar;
	typedef map_keydown map_syschar;

	// --------------------------------------------------------------------

	struct msg_initdialog : stdmsg_template<msg_initdialog>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		HWND	focus_ctrl;
		LPARAM	data;

		LRESULT result;
	};

	struct map_initdialog : msgmap_t
	{
		typedef msg_initdialog msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_command : stdmsg_template<msg_initdialog>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		union
		{
			struct
			{
				WORD	ctrlid;
				WORD	code;
			};
			WPARAM wParam;
		};

		union
		{
			HWND	hSender;
			LPARAM lParam;
		};

		LRESULT result;
	};

	struct map_command : msgmap_t
	{
		typedef msg_command msg_type;

		bool reflect(const msgmap_t &a, void * _this, msg_struct &msg);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_syscommand : stdmsg_template<msg_syscommand>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	sc;
		WORD	cursor_x;
		WORD	cursor_y;

		LRESULT result;
	};

	struct map_syscommand : msgmap_t
	{
		typedef msg_syscommand msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_timer : stdmsg_template<msg_timer>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM	id;

		void (CALLBACK *proc)(HWND hwnd,
			UINT uMsg,
			UINT_PTR idEvent,
			DWORD dwTime);

		LRESULT result;
	};

	struct map_timer : msgmap_t
	{
		typedef msg_timer msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_scroll : stdmsg_template<msg_scroll>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WORD	code;
		WORD	pos;
		HWND	hScroll;

		LRESULT result;
	};

	typedef msg_scroll msg_hscroll;

	struct map_hscroll : msgmap_t
	{
		typedef msg_hscroll msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef msg_hscroll msg_vscroll;
	typedef map_hscroll	map_vscroll;

	// --------------------------------------------------------------------

	struct msg_mouse : stdmsg_template<msg_mouse>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WORD 	key_code;
		short int delta;
		short int	x;
		short int	y;

		LRESULT result;
	};

	struct map_mousemove : msgmap_t
	{
		typedef msg_mouse msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct msg_notify : stdmsg_template<msg_notify>
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;

		WPARAM 	ctrlid;
		LPNMHDR	pnmh;

		LRESULT result;
	};

	struct map_notify : msgmap_t
	{
		typedef msg_notify msg_type;

		bool reflect(const msgmap_t &a, void * _this, msg_struct &msg);

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};
	
	// --------------------------------------------------------------------

	struct msg_hook
	{
		union
		{
			HWND	hWnd;
			int code;
		};

		UINT	message;

		WPARAM 	wParam;

		union
		{
			LPMSG	pmsg;
			LPARAM lParam;
		};

		LRESULT result;	// return 1 if process.
	};

	struct map_hook : msgmap_t
	{
		typedef msg_hook msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	struct map_custom : msgmap_t
	{
		typedef msg_struct msg_type;
		typedef map_create other;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_struct) == sizeof(msg_type));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	// --------------------------------------------------------------------

	typedef map_custom map_mouseleave;

	
#define WABC_ON_CREATE(f) \
	{ 0,  WM_CREATE, 0, wabc::map_create::map_fun_addr<map_class>(f),	&wabc::map_create::on_map },

#define WABC_ON_DESTROY(f) \
	{ 0,  WM_DESTROY, 0, wabc::map_destroy::map_fun_addr<map_class>(f),	&wabc::map_create::on_map },

#define WABC_ON_SIZE(f) \
	{ 0,  WM_SIZE, 0, wabc::map_size::map_fun_addr<map_class>(f),	&wabc::map_size::on_map },

#define WABC_ON_SETFOCUS(f) \
	{ 0,  WM_SETFOCUS, 0, wabc::map_setfocus::map_fun_addr<map_class>(f),	&wabc::map_size::on_map },

#define WABC_ON_KILLFOCUS(f) \
	{ 0,  WM_KILLFOCUS, 0, wabc::map_killfocus::map_fun_addr<map_class>(f),	&wabc::map_size::on_map },

#define WABC_ON_ENABLE(f) \
	{ 0, WM_ENABLE, 0, wabc::map_enable::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SETTEXT(f) \
	{ 0, WM_SETTEXT, 0, wabc::map_settext::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_GETTEXT(f) \
	{ 0, WM_GETTEXT, 0, wabc::map_gettext::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_GETTEXTLENGTH(f) \
	{ 0, WM_GETTEXTLENGTH, 0, wabc::map_gettextlength::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_PAINT(f) \
	{ 0, WM_PAINT, 0, wabc::map_paint::map_fun_addr<map_class>(f), &wabc::map_paint::on_map },

#define WABC_ON_PAINT32(f) \
	{ 0, WM_PAINT, 0, wabc::map_paint32::map_fun_addr<map_class>(f), &wabc::map_paint32::on_map },

#define WABC_ON_CLOSE(f) \
	{ 0, WM_CLOSE, 0, wabc::map_close::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_ERASEBKGND(f) \
	{ 0, WM_ERASEBKGND, 0, wabc::map_erasebkgnd::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_NOT_ERASEBKGND() \
	{ 0, WM_ERASEBKGND, 0, 0, &wabc::map_erasebkgnd::not_erase_background },

#define WABC_ON_SETCURSOR(f) \
	{ 0, WM_SETCURSOR, 0, wabc::map_setcursor::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_MOUSEACTIVE(f) \
	{ 0, WM_MOUSEACTIVATE, 0, wabc::map_mouseactive::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_GETMINMAXINFO(f) \
	{ 0, WM_GETMINMAXINFO, 0, wabc::map_getminmaxinfo::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_DRAWITEM(ctrlid, f) \
	{ ctrlid, WM_DRAWITEM, 0, wabc::map_drawitem::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_MEASUREITEM(f) \
	{ 0, WM_MEASUREITEM, 0, wabc::map_measureitem::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SETFONT(f) \
	{ 0, WM_SETFONT, 0, wabc::map_setfont::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_GETFONT(f) \
	{ 0, WM_GETFONT, 0, wabc::map_getfont::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_POSCHANGING(f) \
	{ 0, WM_WINDOWPOSCHANGING, 0, wabc::map_windowposchanging::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_POSCHANGED(f) \
	{ 0, WM_WINDOWPOSCHANGED, 0, wabc::map_windowposchanging::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_NOTIFY(ctrlid, code, f) \
	{ code, WM_NOTIFY, ctrlid, wabc::map_notify::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_REFLECT_NOTIFY(ctrlid, code) \
	{ code, WM_NOTIFY, ctrlid, 0, &wabc::map_notify::reflect },

#define WABC_ON_NCCREATE(f) \
	{ 0, WM_NCCREATE, 0, wabc::map_nccreate::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_NCDESTROY(f) \
	{ 0, WM_NCDESTROY, 0, wabc::map_ncdestroy::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_NCHITTEST(f) \
	{ 0, WM_NCHITTEST, 0, wabc::map_nchittest::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_GETDLGCODE(f) \
	{ 0, WM_GETDLGCODE, 0, wabc::map_getdlgcode::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SETSCROLLINFO(f) \
	{ 0, SBM_SETSCROLLINFO, 0, wabc::map_setscrollinfo::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_GETSCROLLINFO(f) \
	{ 0, SBM_GETSCROLLINFO, 0, wabc::map_getscrollinfo::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_KEYDOWN(f) \
	{ 0, WM_KEYDOWN, 0, wabc::map_keydown::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_KEYUP(f) \
	{ 0, WM_KEYUP, 0, wabc::map_keyup::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_CHAR(f) \
	{ 0, WM_CHAR, 0, wabc::map_char::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SYSKEYDOWN(f) \
	{ 0, WM_SYSKEYDOWN, 0, wabc::map_syskeydown::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SYSKEYUP(f) \
	{ 0, WM_SYSKEYUP, 0, wabc::map_syskeyup::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SYSCHAR(f) \
	{ 0, WM_SYSCHAR, 0, wabc::map_syschar::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_INITDIALOG(f) \
	{ 0, WM_INITDIALOG, 0, wabc::map_initdialog::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_COMMAND(id, code, f) \
	{ MAKEWPARAM(id,code), WM_COMMAND, 0, wabc::map_command::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_REFLECT_COMMAND(id, code) \
	{ MAKEWPARAM(id, code), WM_COMMAND, 0, 0, &wabc::map_command::reflect },

#define WABC_ON_SYSCOMMAND(sc, f) \
	{ sc, WM_SYSCOMMAND, 0, wabc::map_syscommand::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_TIMER(id, f) \
	{ id, WM_TIMER, 0, wabc::map_timer::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_HSCROLL(f) \
	{ 0, WM_HSCROLL, 0, wabc::map_hscroll::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_VSCROLL(f) \
	{ 0, WM_VSCROLL, 0, wabc::map_vscroll::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_MOUSEMOVE(f) \
	{ 0, WM_MOUSEMOVE, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_LBUTTONDOWN(f) \
	{ 0, WM_LBUTTONDOWN, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_LBUTTONUP(f) \
	{ 0, WM_LBUTTONUP, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_LBUTTONDBLCLK(f) \
	{ 0, WM_LBUTTONDBLCLK, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_RBUTTONDOWN(f) \
	{ 0, WM_RBUTTONDOWN, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_RBUTTONUP(f) \
	{ 0, WM_RBUTTONUP, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_RBUTTONDBLCLK(f) \
	{ 0, WM_RBUTTONDBLCLK, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_MBUTTONDOWN(f) \
	{ 0, WM_MBUTTONDOWN, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_MBUTTONUP(f) \
	{ 0, WM_MBUTTONUP, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_MBUTTONDBLCLK(f) \
	{ 0, WM_MBUTTONDBLCLK, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

// 0x020E
#define WABC_ON_MOUSEWHEEL(f) \
	{ 0, WM_MOUSEWHEEL, 0, wabc::map_mousemove::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

// 0x02A3
#define WABC_ON_MOUSELEAVE(f) \
	{ 0, WM_MOUSELEAVE, 0, wabc::map_mouseleave::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_SYSMSG(msg, f) \
	{ msg, wabc::WM_WTL, 0, wabc::map_custom::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

#define WABC_ON_HOOK_GETMSG(msg, f) \
	{ wabc::SM_HOOK_GETMSG, wabc::WM_WTL, msg, wabc::map_hook::map_fun_addr<map_class>(f), &wabc::map_size::on_map },

//#define WABC_ON_HOOK_PROCMSG(msg, f) \
//	{ wabc::SM_HOOK_PROCMSG, wabc::WM_WTL, msg, wabc::map_msghook::map_fun_addr<map_class>(f), &wabc::map_size::on_map },
//
//#define WABC_ON_DRAW(f) \
//	{ wabc::SM_DRAW, wabc::WM_WTL, 0, wabc::map_draw::map_fun_addr<map_class>(f), &wabc::map_draw::on_map },

#define WABC_ON_MSG(msg,f) \
	{ 0, msg, 0, wabc::map_custom::map_fun_addr<map_class>(f), &wabc::map_size::on_map },
}