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

#include "w_basewnd.h"
//#include "w_typetraits.h"

namespace wabc
{
	enum{ WM_WTL = WM_USER ,
		WM_WABC_USER,
	};

	enum{ 
		SM_HOOK_FIRST= 1,
		SM_HOOK_PROCMSG,	// LPARAM : the pointer of 'msg_hook', return 1 if process
		SM_HOOK_GETMSG = SM_HOOK_FIRST,	// LPARAM : the pointer of 'msg_hook'', return 1 if process
		SM_HOOK_LAST,
		SM_TAB_NEXT = SM_HOOK_LAST,	// LPARAM : handle of control, can be 0, return 1 if process
		SM_TAB_PRIOR,	// LPARAM : handle of control, can be 0, return 1 if process
		SM_SCROLL_MOVE,	// LPARAM : not used, return 0 if allow move, 1: x not move, 2 : y not move
		SM_DRAW, // LPARAM: points to 'msg_struct'
	};

	class wndbase;
	struct mapslot_node;
	struct msg_struct;

	class application;
	extern application *g_app;

	wndbase * from_handle(HWND hWnd);
	
	// --------------------------------------------------------------------

	struct msg_struct
	{
		union
		{
			HWND	hWnd;
			_basewnd	window;
		};

		UINT	message;
		union
		{
			WPARAM	wParam;

			struct
			{
				WORD	wParamLo;
				WORD	wParamHi;
			};
		};

		union
		{
			LPARAM	lParam;

			struct
			{
				WORD	lParamLo;
				WORD	lParamHi;
			};
		};

		LRESULT result;
	};


	// --------------------------------------------------------------------

	struct _msg_struct : msg_struct
	{
		// call this function ONLY in window message procedure
		bool inherited();

		wndbase *wnd;
		mapslot_node * next_slot;
		_msg_struct *next_msg;
	};

	// --------------------------------------------------------------------

	struct msgmap_t
	{
		WPARAM	wParam;
		UINT	message;

		DWORD	ctrlid;

		size_t	on_message;	// the address of map function

		bool(*invoke)(const msgmap_t &a, void * _this, msg_struct &);

		bool operator<(const msgmap_t &rhs)const
		{
			__wabc_static_assert((sizeof(WPARAM)+sizeof(message)) == sizeof(uint64));
			const uint64 &v1 = *reinterpret_cast<const uint64 *>(this);
			const uint64 &v2 = *reinterpret_cast<const uint64 *>(&rhs);
			return v1 < v2;
		}
	};

	// --------------------------------------------------------------------

	struct mapslot_node
	{
		mapslot_node *next, *prior;
		_msg_struct *cur_msg;

		mapslot_node() : cur_msg(0)
		{
			next = prior = this;
		}

		// 插入到链表头
		void link_before(mapslot_node *h)
		{
			assert(h);
			assert(this != h);
			next= h->next;
			h->next->prior= this;

			prior= h;
			h->next= this;
		}

		void unlink()
		{
			prior->next= next;
			next->prior= prior;

			next = prior = this;
		}

		void accept(_msg_struct *msg);
	};

	// --------------------------------------------------------------------

	struct mapslot : mapslot_node
	{
		enum{ flag_no_WM_COMMAND= 1 << 16,
			flag_no_WM_SYSCOMMAND= flag_no_WM_COMMAND << 1,
			flag_no_WM_TIMER= flag_no_WM_SYSCOMMAND << 1,
			flag_no_WM_NOTIFY= flag_no_WM_TIMER << 1,
			flag_no_WM_WTL = flag_no_WM_NOTIFY << 1,
			flag_no_SM_HOOK_PROCMSG = flag_no_WM_WTL << 1,
		};

		void	*wnd;
		const msgmap_t * entries;

		union
		{
			size_t	count;
			struct
			{
				uint16	entries_count;
				uint16	no_WM_COMMAND : 1;
				uint16	no_WM_SYSCOMMAND : 1;
				uint16	no_WM_TIMER : 1;
				uint16	no_WM_NOTIFY : 1;
				uint16	no_WM_WTL : 1;
				uint16	no_PROCMSG : 1;

			};
		};

		~mapslot(){ unmap(); }

		void assign(void *map_to, const msgmap_t *entries, size_t n);
		void unmap();
	};

	// --------------------------------------------------------------------

	class wndbase : public basewnd
	{
		uint8	*m_thunk;
		int m_hook_procmsg_count;
		mapslot_node	m_mapslot_head;

		friend class wndproc;

		wndbase(const wndbase &);
		void operator=(const wndbase &);
	protected:

		WNDPROC create_thunk(WNDPROC proc);
		void free_thunk();

	public:
		wndbase();
		virtual ~wndbase();
		
		template<typename W>
		void map_msg(mapslot *slot, W *map_to, const msgmap_t *entries, size_t n)
		{
			slot->assign(map_to, entries, n);
			slot->link_before(&m_mapslot_head);
		}

		HWND create(const CREATESTRUCT &cs);

		HWND create(HWND hParent, DWORD nStyle, DWORD dwStyleEx = 0, 
			const wchar_t *lpszCaption = 0, size_t id = 0, const RECT *rt = 0);
		
		HWND create(const wchar_t *lpszCaption, DWORD nStyle= WS_OVERLAPPEDWINDOW, DWORD dwStyleEx = 0, 
			const RECT *rt = 0, HMENU hMenu= 0);

		void destroy();

		bool paint(dcclass &dc, const RECT &rtClip, size_t context= 1);
		bool paint(dcclass &dc, size_t context = 1) { return paint(dc, client_rect(), context); }

		bool paint(const wndbase &child, dcclass &dc, const RECT &rtClip, size_t context);
		bool paint(const wndbase &child, dcclass &dc, const RECT &rtClip)
		{
			return paint(child, dc, rtClip, size_t(&child));
		}

		bool paint(const wndbase &child, dcclass &dc, size_t context) { return paint(child, dc, client_rect(), context); }
		bool paint(const wndbase &child, dcclass &dc) { return paint(child, dc, client_rect(), size_t(&child)); }

		int run_modal();
		void end_modal(int exit_code);

		int hook_procmsg(){ return ++m_hook_procmsg_count; }
		int unhook_procmsg(){ return --m_hook_procmsg_count; }
	public:

		virtual void before_create(CREATESTRUCT &cs);
		virtual void after_create(){}
		virtual void after_destroy(){}
	};

	// --------------------------------------------------------------------

	class scwnd : public wndbase
	{
		// --------------------------------------------------------------------

		struct scinfo_t
		{
			WNDPROC	old_proc;
			UINT	class_name;
		} m_sc;

		static scinfo_t super_class(string::const_pointer lpszOldClassName,
			string::const_pointer lpszNewClassName);

		friend class wndproc;
	public:

		typedef scwnd self;
		typedef wndbase inherited;

		scwnd() { ::memset(&m_sc, 0, sizeof(m_sc)); }

		// supperclass window
		template< typename T >
		explicit scwnd(const T &t)
		{
			static scinfo_t this_scinfo = super_class(t.old_name(), t.new_name());
			m_sc = this_scinfo;
		}

		virtual ~scwnd(){}

		// subclass window
		WNDPROC attach(HWND hWnd);
		void detach();

	public:
		virtual void before_create(CREATESTRUCT &cs);

		void msg_default(msg_struct &msg)
		{
			assert(m_sc.old_proc);
			msg.result = ::CallWindowProc(m_sc.old_proc, msg.hWnd, msg.message, msg.wParam, msg.lParam);
		}
	};

	// --------------------------------------------------------------------

	class application
	{
		HINSTANCE	m_hInstance;

		void init();
	public:
		const ATOM		defclass;

		explicit application(HINSTANCE hInstance, const wchar_t *lpClassName = _T("wabc"));

		operator HINSTANCE()const { return m_hInstance; }
	};

	// --------------------------------------------------------------------

	struct hooker
	{
#pragma pack(push,1) //该结构必须以字节对齐
		struct thunk_struct
		{
			BYTE code[16];
			DWORD   m_relproc;      // relative jmp
		};
#pragma pack(pop)

		int code;	// 必须是第一个变量
		HHOOK m_hook;
		thunk_struct *m_thunk;

		typedef LRESULT(CALLBACK* fun_t)(hooker *p, WPARAM wParam, LPARAM lParam);

		hooker();
		~hooker();

		HHOOK hook(int idHook, fun_t lpfn, HINSTANCE hMod,DWORD dwThreadId);

		HHOOK hook(int idHook, fun_t lpfn)
		{
			return hook(idHook, lpfn, *g_app, ::GetCurrentThreadId());
		}

		void unhook()
		{
			if (m_hook)
			{
				UnhookWindowsHookEx(m_hook);
				m_hook = 0;
			}
		}
	};

	// --------------------------------------------------------------------

	class getmsg_hooker : hooker
	{
		static LRESULT CALLBACK hookproc(hooker *p, WPARAM wParam, LPARAM lParam);
		mapslot *m_slot;
	public:
		getmsg_hooker() : m_slot(0){}

		HHOOK hook(mapslot *slot);
		void unhook();

		HHOOK is_hook()const { return m_hook; }
	};
}

#define WABC_DECLARE_HOOK_GETMSG() private : getmsg_hooker m_wabc_getmsg_hooker;
#define WABC_HOOK_GETMSG() m_wabc_getmsg_hooker.hook(m_mapslot_wabc);
#define WABC_HOOK_GETMSGEX(N) {__wabc_static_assert(N<countof(m_mapslot_wabc)); m_wabc_getmsg_hooker.hook(m_mapslot_wabc+N);}
#define WABC_UNHOOK_GETMSG() m_wabc_getmsg_hooker.unhook();

//#define WABC_DECLARE_HOOK_GETMSG(thisClass, msg) private: \
//struct getmsg_##thisClass##msg : wabc::hooker{ \
//	getmsg_##thisClass##msg() : hooker(offsetof(thisClass, m_getmsg_##thisClass##msg), msg,0) { \
//wabc::wndbase &w = *reinterpret_cast<thisClass *>(0); } \
//}m_getmsg_##thisClass##msg;
//
//#define WABC_DECLARE_HOOK_PROCMSG(thisClass, msg) private: \
//struct procmsg_##thisClass##msg : wabc::hooker{ \
//	procmsg_##thisClass##msg() : hooker(offsetof(thisClass, m_procmsg_##thisClass##msg), msg,1) {\
//wabc::wndbase &w = *reinterpret_cast<thisClass *>(0);} \
//}m_procmsg_##thisClass##msg;
//
