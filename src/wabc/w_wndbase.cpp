#include "w_wndbase.h"
#include "w_thunk.h"
#include "w_dc.h"
#include "w_proc.h"

#include <algorithm>

namespace wabc
{
	application *g_app = 0;

	// --------------------------------------------------------------------

#ifdef _DEBUG
	static inline bool compare_by_notify(const msgmap_t &lhs, const msgmap_t &rhs)
	{
		const uint64 &v1 = *reinterpret_cast<const uint64 *>(&lhs);
		const uint64 &v2 = *reinterpret_cast<const uint64 *>(&rhs);
		if (v1 == v2)
			return lhs.ctrlid < rhs.ctrlid;
		return v1 < v2;
	}
#endif

	// --------------------------------------------------------------------
	// mapslot_node
	// --------------------------------------------------------------------

	void mapslot_node::accept(_msg_struct *msg)
	{
		// FIXME - 这里假设msg_struct都是在堆栈上分配的，比较的时候按照所在地址的大小比较，
		// 低地址在链表头，高地址在链表尾。这么做可以使msg_node减少一个成员变量。
		// 若msg_struct是在堆上分配的，这一技巧立刻失效。
		assert(msg);

		msg->next_slot = this;
		if (!cur_msg)
		{
			cur_msg = msg;
		}
		else
		{
			if (msg->next_msg)
				accept(msg->next_msg);

			if (msg < cur_msg)
			{
				// msg处于的堆栈比cur_msg深
				msg->next_msg = cur_msg;
				cur_msg = msg;
			}
			else
			{
				// 有序单向链表的插入算法
				_msg_struct *p = cur_msg;
				for (; p->next_msg; p = p->next_msg)
				{
					if (msg < p->next_msg)
					{
						msg->next_msg = p->next_msg;
						p->next_msg = msg;
						return;
					}
				}
				p->next_msg = msg;
			}
		}
	}

	// --------------------------------------------------------------------
	// mapslot
	// --------------------------------------------------------------------

	void mapslot::unmap()
	{
		if (cur_msg)
		{
			// 在当前mapslot指向的消息映射处理函数进行了析构，若不处理，
			// 进入下一个mapslot立刻崩溃
			assert(next != this && prior != this);
			next->accept(cur_msg);
			cur_msg = 0;
		}
		unlink();
	}

	// --------------------------------------------------------------------

	void mapslot::assign(void *map_to, const msgmap_t *entries1, size_t n)
	{
		// 必须保证slot没有链接到消息链表上
		assert(next == this);
		assert(prior == this);

		// n的值必须小于64k
		assert(n < 64 * 1024);

		wnd = map_to;
		entries = entries1;
		count = n;

#ifdef _DEBUG
		for (size_t i = 1; i < entries_count; ++i)
		{
			assert(compare_by_notify(entries[i - 1], entries[i]));
		}
#endif
	}

	// --------------------------------------------------------------------
	// wndbase
	// --------------------------------------------------------------------

	wndbase::wndbase() :m_thunk(0), m_hook_procmsg_count(0)
	{
		__wabc_static_assert(sizeof(HWND) == sizeof(_basewnd));
	}

	// --------------------------------------------------------------------

	wndbase::~wndbase()
	{
		mapslot_node *p = m_mapslot_head.next, *p1;
		while (p != &m_mapslot_head)
		{
			p1 = p;
			p = p->next;
			if (p1->cur_msg)
			{
				// 若还有消息未处理，将其推向下一个节点，最终推向头节点
				p->accept(p1->cur_msg);
				p1->cur_msg = 0;
			}

			// 还有节点在其链表上，使其断开。
			p1->next = p1->prior = p1;
		}
		
		for (_msg_struct *p = m_mapslot_head.cur_msg; p; p = p->next_msg)
		{
			assert(p->next_slot == &m_mapslot_head);
			// 将其设0，使窗口过程不再调用msg_defproc()函数，避免崩溃
			p->wnd = 0;
		}

		if (m_hWnd)
			::DestroyWindow(m_hWnd);

		if (m_thunk)
			thunk_free(m_thunk);
	}

	// --------------------------------------------------------------------

	WNDPROC wndbase::create_thunk(WNDPROC proc)
	{
		assert(!m_thunk);
		m_thunk = (uint8*)thunk_alloc(sizeof(wndproc_thunk));

		assert(m_thunk != 0);

		wndproc_thunk &thunk = *reinterpret_cast<wndproc_thunk*>(m_thunk);

		// mov dword ptr [esp+0x4], pThis
		// jmp (int)proc - ((int)&thunk+sizeof(thunk))

		thunk.m_mov = 0x042444C7;  //C7 44 24 0C
		thunk.m_this = (DWORD)this;

		thunk.m_jmp = 0xe9;
		thunk.m_relproc = (int)proc - ((int)(&thunk) + sizeof(wndproc_thunk));

		return WNDPROC(&thunk);
	}

	// --------------------------------------------------------------------

	void wndbase::free_thunk()
	{
		if (m_thunk)
		{
			thunk_free(m_thunk);
			m_thunk = 0;
		}
	}

	// --------------------------------------------------------------------

	HWND wndbase::create(HWND hParent, DWORD nStyle, DWORD dwStyleEx,
		const wchar_t *lpszCaption, size_t id, const RECT *rt)
	{
		CREATESTRUCT cs;
		::memset(&cs, 0, sizeof(cs));

		cs.hInstance = *g_app;

		// 为id取一个默认值，若为0，恐怕会在WM_NOTIFY的消息处理中带来麻烦
		// 见 process_WM_NOTIFY 的实现
		if (nStyle & WS_CHILD)
			cs.hMenu = id != 0 ? HMENU(id) : HMENU(this);
		else
			cs.hMenu = HMENU(id);

		before_create(cs);
		if (rt)
		{
			const LONG w = rt->right - rt->left;
			const LONG h = rt->bottom - rt->top;
			cs.x = rt->left;
			cs.y = rt->top;
			if (w)
				cs.cx = w;
			if (h)
				cs.cy = h;
		}

		cs.hwndParent = hParent;
		cs.style |= nStyle;
		cs.dwExStyle |= dwStyleEx;
				
		cs.lpszName = lpszCaption;

		return create(cs);
	}

	// --------------------------------------------------------------------

	HWND wndbase::create(const wchar_t *lpszCaption, DWORD nStyle, DWORD dwStyleEx,
		const RECT *rt, HMENU hMenu)
	{
		CREATESTRUCT cs;
		::memset(&cs, 0, sizeof(cs));

		cs.hInstance = *g_app;
		before_create(cs);

		cs.style |= nStyle;
		cs.dwExStyle |= dwStyleEx;

		cs.lpszName = lpszCaption;
		cs.hMenu = hMenu;

		if (rt)
		{
			const LONG w = rt->right - rt->left;
			const LONG h = rt->bottom - rt->top;
			cs.x = rt->left;
			cs.y = rt->top;
			if (w)
				cs.cx = w;
			if (h)
				cs.cy = h;
		}
		else
		{
			cs.x = CW_USEDEFAULT;
			cs.y = CW_USEDEFAULT;
		}

		return create(cs);
	}

	// --------------------------------------------------------------------

	HWND wndbase::create(const CREATESTRUCT &cs)
	{
		assert(m_hWnd == 0);
		return CreateWindowEx(
			cs.dwExStyle,
			cs.lpszClass,
			cs.lpszName,
			cs.style,
			cs.x,
			cs.y,
			cs.cx,
			cs.cy,
			cs.hwndParent,
			cs.hMenu,
			cs.hInstance,
			this
			);
	}
	
	// --------------------------------------------------------------------

	void wndbase::before_create(CREATESTRUCT &cs)
	{
		cs.lpszClass = MAKEINTATOM(g_app->defclass);
	}

	// --------------------------------------------------------------------

	void wndbase::destroy()
	{
		if (m_hWnd)
		{
			::DestroyWindow(m_hWnd);
			m_hWnd = 0;

			free_thunk();
		}
	}

	// --------------------------------------------------------------------

	bool wndbase::paint(dcclass &dc, const RECT &rtClip, size_t context)
	{
		_msg_struct msg;
		msg.hWnd = m_hWnd;
		msg.message = WM_PAINT;
		msg.wParam = reinterpret_cast<WPARAM>(&dc);
		msg.lParam = reinterpret_cast<LPARAM>(&rtClip);
		msg.result = context;
		msg.next_slot = m_mapslot_head.next;
		msg.next_msg = 0;
		msg.wnd = this;
		return wndproc::process(msg);
	}

	// --------------------------------------------------------------------

	bool wndbase::paint(const wndbase &child, dcclass &dc, const RECT &rtClip, size_t context)
	{
		// 画child所占据的区域
		rect rt(child.window_rect());
		screen_to_client(rt);

		const POINT ptOrg = dc.window_org.offset(rt.left, rt.top);

		rt = rtClip;

		const DWORD dwExStyle = child.exstyle();
		const DWORD dwStyle = child.style();
		HMENU hMenu;

		if ((dwStyle & WS_CHILD) == 0)
			hMenu = ::GetMenu(child);
		else
			hMenu = 0;

		::AdjustWindowRectEx(&rt, dwStyle, hMenu != 0, dwExStyle);

		::MapWindowPoints(child, *this, LPPOINT(&rt), 2);

		const bool result = paint(dc, rt, context);

		dc.window_org = ptOrg;
		return result;
	}

	// --------------------------------------------------------------------

	int wndbase::run_modal()
	{
		HWND hParent = ::GetParent(m_hWnd);
		::EnableWindow(hParent, false);
		::SetActiveWindow(m_hWnd);
		MSG msg;

		while (TRUE)
		{
			while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (msg.hwnd == hParent && msg.message != WM_PAINT)
				{
					continue;
				}

				const BOOL ret = ::GetMessage(&msg, NULL, 0, 0);
				if (ret == 0)
				{
					::EnableWindow(hParent, TRUE);
					::SetActiveWindow(hParent);
					return int(msg.wParam);
				}

				::TranslateMessage(&msg);
				::DispatchMessage(&msg);

				if (!::IsWindow(m_hWnd))
				{
					::EnableWindow(hParent, TRUE);
					::SetActiveWindow(hParent);
					return 0;
				}
			}
			::WaitMessage();
		}
		return   FALSE;
	}

	// --------------------------------------------------------------------

	void wndbase::end_modal(int exit_code)
	{
		::PostQuitMessage(exit_code);
	}

	// --------------------------------------------------------------------
	// class scwnd::supperclass
	// --------------------------------------------------------------------

	scwnd::scinfo_t scwnd::super_class(string::const_pointer lpszOldClassName,
		string::const_pointer lpszNewClassName)
	{
		scinfo_t sc;
		WNDCLASSEX wc;
		wc.cbSize = sizeof(wc);
		BOOL result = ::GetClassInfoEx(*g_app, lpszOldClassName, &wc);

		if (result == FALSE)
			throw_win32_exception(_T("GetClassInfoEx()"));

		sc.old_proc = wc.lpfnWndProc;
		wc.lpfnWndProc = wndproc::wnd_init;
		wc.lpszClassName = lpszNewClassName;

		sc.class_name = ::RegisterClassEx(&wc);
		if (!sc.class_name)
			throw_win32_exception(_T("GetClassInfoEx()"));

		return sc;
	}

	// --------------------------------------------------------------------
	
	void scwnd::before_create(CREATESTRUCT &cs)
	{
		assert(m_sc.class_name);
		cs.lpszClass = MAKEINTATOM(m_sc.class_name);
	}

	// --------------------------------------------------------------------

	WNDPROC scwnd::attach(HWND hWnd)
	{
		assert(m_hWnd == 0);
		assert(m_sc.old_proc == 0 && m_sc.class_name == 0);

		m_hWnd = hWnd;
#ifdef _DEBUG
		m_debug_hWnd = hWnd;
#endif
		WNDPROC proc = create_thunk(wndproc::scwnd_main);
		m_sc.old_proc = (WNDPROC)SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG)proc);
		return m_sc.old_proc;
	}

	// --------------------------------------------------------------------

	void scwnd::detach()
	{
		assert(m_hWnd);
		assert(m_sc.old_proc && m_sc.class_name == 0);

		if (m_hWnd)
		{
			SetWindowLongPtr(m_hWnd, GWL_WNDPROC, (LONG)m_sc.old_proc);
			free_thunk();
			m_hWnd = 0;
			m_sc.old_proc = 0;
#ifdef _DEBUG
			m_debug_hWnd = 0;
#endif
		}
	}

	// https://blog.csdn.net/mdj280759843/article/details/83846574
	// Thunk技术
	//
	//LRESULT CALLBACK ThunkTemplate(DWORD& addr1, DWORD& addr2)
	//{
	//	int flag = 0;
	//	DWORD x1, x2;
	//	if (flag)
	//	{
	//		_asm
	//		{
	//		stdcall_1:
	//			mov ECX, -1;
	//			mov EAX, dword ptr[ESP + 4];
	//			mov dword ptr[ECX], EAX;
	//			mov dword ptr[ESP + 4], ECX;
	//		stdcall_2:
	//		}
	//	}

	//	_asm
	//	{
	//		mov x1, offset stdcall_1;
	//		mov x2, offset stdcall_2;
	//	}
	//	addr1 = x1;
	//	addr2 = x2;
	//}

	// --------------------------------------------------------------------
	// hooker
	// --------------------------------------------------------------------	

	hooker::hooker()
	{
		::memset(this, 0, sizeof(*this));
	}

	// --------------------------------------------------------------------

	hooker::~hooker()
	{
		if (m_hook)
			UnhookWindowsHookEx(m_hook);
		if (m_thunk)
			thunk_free(m_thunk);
	}

	// --------------------------------------------------------------------

	HHOOK hooker::hook(int idHook, fun_t lpfn, HINSTANCE hMod, DWORD dwThreadId)
	{
		if (m_hook)
		{
			UnhookWindowsHookEx(m_hook);
			m_hook = 0;
		}

		if (!m_thunk)
		{
			m_thunk = (thunk_struct *)thunk_alloc(sizeof(thunk_struct));

			// mov ECX, -1;		// b9 ff ff ff ff
			// mov EAX, dword ptr[ESP + 4]; // 8b 44 24 04
			// mov dword ptr[ECX], EAX;	// 89 01
			// mov dword ptr[ESP + 4], ECX;	// 89 4c 24 04
			// jmp _1;	// e9
			const BYTE thunk_code[] = { 0xb9, 0xff, 0xff, 0xff, 0xff, 0x8b, 0x44, 0x24, 0x04, 0x89, 0x01, 0x89, 0x4c, 0x24, 0x04, 0xe9 };
			__wabc_static_assert(sizeof(thunk_struct) == (sizeof(thunk_code)+sizeof(DWORD)));
			::memcpy(m_thunk, thunk_code, sizeof(thunk_code));
		}
		m_thunk->m_relproc = ((int)lpfn) - ((int)(m_thunk)+sizeof(thunk_struct));
		HOOKPROC proc = reinterpret_cast<HOOKPROC>(m_thunk);
		m_hook = ::SetWindowsHookEx(idHook, proc, hMod, dwThreadId);
		return m_hook;
	}

	// --------------------------------------------------------------------
	// getmsg_hooker
	// --------------------------------------------------------------------	

	HHOOK getmsg_hooker::hook(mapslot *slot)
	{
		m_slot = slot;

		if (!m_hook)
			hooker::hook(WH_GETMESSAGE, &hookproc);
		return m_hook;
	}

	// --------------------------------------------------------------------	

	void getmsg_hooker::unhook()
	{
		m_slot = 0;
		hooker::unhook();
	}
	
	// --------------------------------------------------------------------
	// application
	// --------------------------------------------------------------------	

	static ATOM register_class(HINSTANCE hInstance,const wchar_t *lpClassName)
	{
		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
			&wndproc::wnd_init, 0, 0, hInstance, NULL, ::LoadCursor(0, IDC_ARROW), // NULL
			(HBRUSH)(COLOR_WINDOW + 1), NULL, lpClassName, NULL };
		const ATOM atom = RegisterClassEx(&wc);
		return atom;
	}
	
	// --------------------------------------------------------------------	

	application::application(HINSTANCE hInstance, const wchar_t *lpClassName): m_hInstance(hInstance)
		, defclass(register_class(hInstance, lpClassName))
	{
		g_app = this;
	}

	// --------------------------------------------------------------------	

	static bool get_wndbase(_msg_struct &msg)
	{
		assert(msg.wnd);
		msg.result= LRESULT(msg.wnd);
		return true;
	}

	wndbase * from_handle(HWND hWnd)
	{
		msg_struct msg;
		::memset(&msg, 0, sizeof(msg));

		const LRESULT ret = ::SendMessage(hWnd, WM_WTL, WPARAM(&msg), LPARAM(&get_wndbase));
		return reinterpret_cast<wndbase *>(msg.result);
	}
}