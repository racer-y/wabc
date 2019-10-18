#include "w_proc.h"
#include "w_wndbase.h"
#include "w_dialog.h"

#include <algorithm>

namespace wabc
{
	typedef mapslot_ptr<mapslot_node> head_guard;

	// --------------------------------------------------------------------

	static inline bool compare_by_notify(const msgmap_t &lhs, const msgmap_t &rhs)
	{
		const uint64 &v1 = *reinterpret_cast<const uint64 *>(&lhs);
		const uint64 &v2 = *reinterpret_cast<const uint64 *>(&rhs);
		if (v1 == v2)
			return lhs.ctrlid < rhs.ctrlid;
		return v1 < v2;
	}

	// --------------------------------------------------------------------
	// wndproc
	// --------------------------------------------------------------------

	bool wndproc::process_WM(_msg_struct &msg, WPARAM wParam, size_t no_flag)
	{
		typedef std::pair<const msgmap_t *, const msgmap_t *> pair_type;
		msgmap_t v;
		v.message = msg.message;
		v.wParam = wParam;

		head_guard head(msg.wnd->m_mapslot_head);
		while (msg.cur_slot != head.m_p)
		{
			mapslot &slot = static_cast<mapslot &>(*msg.cur_slot);
			pair_type pr = std::equal_range(slot.entries, slot.entries + slot.entries_count, v);
			if (pr.first == pr.second)
			{
				// FIMXE -注意'mapslot::count'的内存布局，见"struct mapslot"的定义
				if (wParam == 0 || (slot.count & no_flag))
				{
					msg.cur_slot = slot.next;
					continue;
				}

				// 给 message range 一个处理的机会
				v.wParam = 0;
				pr = std::equal_range(slot.entries, pr.first, v);
				v.wParam = wParam;
				if (pr.first == pr.second)
				{
					// 标记位在'count'的高16位字节
					assert(no_flag >= 64 * 1024);
					slot.count |= no_flag;
					msg.cur_slot = slot.next;
					continue;
				}
			}

			mapslot_ptr<mapslot> g(&slot);

			const msgmap_t &v2 = *pr.first;
			if (v2.invoke(v2, slot.wnd, msg))
				return true;

			assert(msg.cur_slot == &slot);
			msg.cur_slot = slot.next;
			assert(msg.cur_slot->next != msg.cur_slot);
		}
		return false;
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_CREATE(_msg_struct &msg)
	{
		typedef std::pair<const msgmap_t *, const msgmap_t *> pair_type;
		msgmap_t v;
		v.message = msg.message;

		head_guard head(msg.wnd->m_mapslot_head);
		while (msg.cur_slot != head.m_p)
		{
			mapslot &slot = static_cast<mapslot &>(*msg.cur_slot);
			pair_type pr = std::equal_range(slot.entries, slot.entries + slot.entries_count, v);
			if (pr.first != pr.second)
			{
				mapslot_ptr<mapslot> g(&slot);

				const msgmap_t &v2 = *pr.first;
				if (v2.invoke(v2, slot.wnd, msg))
					return true;

				assert(msg.cur_slot == &slot);
				msg.cur_slot = slot.prior;
				assert(msg.cur_slot->prior != msg.cur_slot);
			}
			else
				msg.cur_slot = slot.prior;
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_NCDESTROY(_msg_struct &msg)
	{
		msg.wnd->free_thunk();
		msg.wnd->m_hWnd = 0;
		const bool ret = process_WM(msg);

		// 永远返回false，对于超类化的窗口来说，其被替换的窗口过程或许重载了这个消息
		return false;
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_COMMAND(_msg_struct &msg)
	{
		if (HWND(msg.lParam) == msg.hWnd)
		{
			// 消息反射，不需要验证control id
			return process_WM(msg, MAKEWPARAM(0, msg.wParamHi), mapslot::flag_no_WM_COMMAND);
		}
		
		return process_WM(msg, msg.wParam, mapslot::flag_no_WM_COMMAND);
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_SYSCOMMAND(_msg_struct &msg)
	{
		return process_WM(msg, msg.wParam, mapslot::flag_no_WM_SYSCOMMAND);
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_TIMER(_msg_struct &msg)
	{
		return process_WM(msg, msg.wParam, mapslot::flag_no_WM_TIMER);
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_NOTIFY(_msg_struct &msg)
	{
		NMHDR &nh = *reinterpret_cast<NMHDR *>(msg.lParam);

		if (nh.hwndFrom == msg.hWnd)
		{
			// 这个消息传递给自身，不需要验证control id
			return process_WM(msg, nh.code, mapslot::flag_no_WM_NOTIFY);
		}
		else
		{
			typedef std::pair<const msgmap_t *, const msgmap_t *> pair_type;
			msgmap_t v;
			v.message = msg.message;
			v.wParam = nh.code;

			// 一般情况下，nh.idFrom不应该为0，若为0，若多个不同类型的控件都
			// 定义了同一个nh.code的值，这时找到的映射函数可能不是所期望的
			assert(nh.idFrom);
			v.ctrlid = nh.idFrom;
			
			head_guard head(msg.wnd->m_mapslot_head);
			while (msg.cur_slot != head.m_p)
			{
				mapslot &slot = static_cast<mapslot &>(*msg.cur_slot);
				pair_type pr = std::equal_range(slot.entries, slot.entries + slot.entries_count, v, &compare_by_notify);

				if (pr.first == pr.second)
				{
					// 也许映射了整个WM_NOTIFY消息
					if (slot.no_WM_NOTIFY != 0)
					{
						msg.cur_slot = slot.next;
						continue;
					}

					v.wParam = 0;
					v.ctrlid = 0;

					pr = std::equal_range(slot.entries, pr.first, v);

					v.wParam = nh.code;
					v.ctrlid = nh.idFrom;
					if (pr.first == pr.second)
					{
						slot.no_WM_NOTIFY = 1;
						msg.cur_slot = slot.next;
						continue;
					}
				}

				mapslot_ptr<mapslot> g(&slot);

				const msgmap_t &v2 = *pr.first;
				if (v2.invoke(v2, slot.wnd, msg))
					return true;

				assert(msg.cur_slot == &slot);
				msg.cur_slot = slot.next;
				assert(msg.cur_slot->next != msg.cur_slot);
			}
		}
		return false;
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_DRAWITEM(_msg_struct &msg)
	{
		DRAWITEMSTRUCT &di = *(reinterpret_cast<DRAWITEMSTRUCT*>(msg.lParam));

		if (di.hwndItem == msg.hWnd)
		{
			// 消息发往自身
			return process_WM(msg);
		}
		else if (process_WM(msg, msg.wParam))
			return true;
		return false;
	}

	// --------------------------------------------------------------------

	bool wndproc::process_WM_WTL(_msg_struct &msg)
	{
		typedef bool(*fun_t)(_msg_struct &);
		fun_t fun = reinterpret_cast<fun_t>(msg.lParam);

		if (msg.wParam >= 64 * 1024)
		{
			_msg_struct &other = *reinterpret_cast<_msg_struct *>(msg.wParam);
			msg.message = other.message;
			msg.wParam = other.wParam;
			msg.lParam = other.lParam;
			if (fun(msg))
			{
				other.result = msg.result;
				msg.result = 1;
			}

			// 由于是自定义内部消息，所以这消息已经被处理了
			return true;
		}
		else
			return process_WM(msg, msg.wParam, mapslot::flag_no_WM_WTL);
	}


	// --------------------------------------------------------------------

	LRESULT CALLBACK wndproc::wnd_init(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			CREATESTRUCT& cs = *reinterpret_cast<CREATESTRUCT*>(lParam);
			wndbase * pWnd = reinterpret_cast< wndbase *>(cs.lpCreateParams);

			pWnd->m_hWnd = hWnd;

			WNDPROC proc = pWnd->create_thunk(cs.lpszClass == MAKEINTATOM(g_app->defclass) ? &wndproc::wnd_main : &wndproc::scwnd_main);
			SetWindowLongPtr(hWnd, GWL_WNDPROC, (LONG)proc);

			return proc(HWND(pWnd), message, wParam, lParam);
		}
		else
			return ::DefWindowProc(hWnd, message, wParam, lParam);
	}

	// --------------------------------------------------------------------

	LRESULT CALLBACK wndproc::wnd_main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		wndbase &wnd = *reinterpret_cast<wndbase *>(hWnd);
		_msg_struct msg;
		msg.hWnd = wnd.m_hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.result = 0;

		msg.cur_slot = wnd.m_mapslot_head->next;
		msg.wnd = &wnd;

		if (wnd.m_hook_procmsg_count > 0)
		{
			if (!process_procmsg(msg))
				goto __default;
		}

		if (!process(msg))
		{
		__default:
			msg.result = ::DefWindowProc(msg.hWnd, msg.message, msg.wParam, msg.lParam);
		}

		return msg.result;
	}

	// --------------------------------------------------------------------

	LRESULT CALLBACK wndproc::scwnd_main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		scwnd &wnd = *reinterpret_cast<scwnd *>(hWnd);
		WNDPROC old_proc = wnd.m_sc.old_proc;

		_msg_struct msg;
		msg.hWnd = wnd.m_hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.result = 0;

		msg.cur_slot = wnd.m_mapslot_head->next;
		msg.wnd = &wnd;

		if (wnd.m_hook_procmsg_count > 0)
		{
			if (!process_procmsg(msg))
				goto __default;
		}

		if (msg.message == WM_CREATE)
			msg.result = ::CallWindowProc(old_proc, msg.hWnd, msg.message, msg.wParam, msg.lParam);

		if (!process(msg))
		{
		__default:
			msg.result = ::CallWindowProc(old_proc, msg.hWnd, msg.message, msg.wParam, msg.lParam);
		}

		return msg.result;
	}

	// --------------------------------------------------------------------

	INT_PTR CALLBACK wndproc::dlg_init(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message != WM_INITDIALOG)
			//		::DefDlgProc ( hwnd,message,wParam,lParam );
			return false;

		dialog::create_param &param = *reinterpret_cast<dialog::create_param *>(lParam);

		param.dlg->m_hWnd = hWnd;

		DLGPROC proc = (DLGPROC)param.dlg->create_thunk(reinterpret_cast<WNDPROC>(&wndproc::dlg_main));
		SetWindowLongPtr(hWnd, DWL_DLGPROC, (LONG)proc);

		param.dlg->dialog_create(param);
		if (param.dlg->m_tab_ctrls.size() > 0)
			::SetFocus(param.dlg->m_tab_ctrls[0]);
		
		return proc(HWND(param.dlg), message, wParam, lParam);
	}

	// --------------------------------------------------------------------

	INT_PTR CALLBACK wndproc::dlg_main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		dialog &wnd = *reinterpret_cast<dialog *>(hWnd);

		_msg_struct msg;		

		msg.hWnd = wnd.m_hWnd;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.result = 0;

		msg.wnd = &wnd;
		msg.cur_slot = wnd.m_mapslot_head->next;

		if (wnd.m_hook_procmsg_count > 0)
		{
			if (!process_procmsg(msg))
				return FALSE;
		}

		if (process(msg))
		{
			::SetWindowLongPtr(wnd.m_hWnd, DWL_MSGRESULT, msg.result);
			return TRUE;
		}
		return FALSE;
	}

	// --------------------------------------------------------------------

	bool wndproc::process(_msg_struct &msg)
	{
		// --------------------------------------------------------------------

		struct special_msgs
		{
			typedef bool(*fun_t)(_msg_struct &);
			UINT	message;
			fun_t	fun;
		};

		static special_msgs items[] = {
			WM_CREATE, wndproc::process_WM_CREATE,
			WM_DRAWITEM, wndproc::process_WM_DRAWITEM,
			WM_NOTIFY, wndproc::process_WM_NOTIFY,
			WM_NCDESTROY, wndproc::process_WM_NCDESTROY,
			WM_INITDIALOG, wndproc::process_WM_CREATE,
			WM_COMMAND, wndproc::process_WM_COMMAND,
			WM_SYSCOMMAND, wndproc::process_WM_SYSCOMMAND,
			WM_TIMER, wndproc::process_WM_TIMER,
			WM_WTL, wndproc::process_WM_WTL,
		};

		const size_t nSize = countof(items);
		size_t first = 0, last = nSize, n = nSize, m, tmp;

#ifdef _DEBUG
		for (m = 1; m < countof(items); ++m)
		{
			assert(items[m - 1].message < items[m].message);
		}
#endif

		while (0 < n)
		{
			m = n / 2;
			tmp = first + m;

			if (items[tmp].message < msg.message)
			{
				first = tmp + 1;
				n -= m + 1;
			}
			else if (msg.message < items[tmp].message)
				n = m;
			else
			{
				return items[tmp].fun(msg);
			}
		}

		return wndproc::process_WM(msg);
	}

	// --------------------------------------------------------------------

	bool wndproc::process_procmsg(const _msg_struct &msg)
	{
		typedef std::pair<const msgmap_t *, const msgmap_t *> pair_type;
		msgmap_t v;
		v.message = WM_WTL;
		v.wParam = SM_HOOK_PROCMSG;
		v.ctrlid = msg.message;

		MSG m = { 0 };
		m.hwnd = msg.hWnd;
		m.message = msg.message;
		m.wParam = msg.wParam;
		m.lParam = msg.lParam;

		_msg_struct msg1= msg;
		msg_hook &mh = reinterpret_cast<msg_hook &>(msg1);
		mh.code = HC_ACTION;
		mh.wParam = PM_REMOVE;
		mh.lParam = LPARAM(&m);

		head_guard head(msg1.wnd->m_mapslot_head);
		while (msg1.cur_slot != head.m_p)
		{
			mapslot &slot = static_cast<mapslot &>(*msg.cur_slot);
			if (slot.no_PROCMSG != 0)
				continue;

			pair_type pr = std::equal_range(slot.entries, slot.entries + slot.entries_count, v);
			if (pr.first != pr.second)
			{
				// 每一个slot都要检查
				mapslot_ptr<mapslot> g(&slot);
				const msgmap_t &v2 = *pr.first;
				v2.invoke(v2, slot.wnd, reinterpret_cast<msg_struct &>(msg1));

				assert(msg.cur_slot == &slot);
				msg1.cur_slot = slot.next;
				assert(msg.cur_slot->next != msg.cur_slot);

			}
			else
			{
				msg1.cur_slot = slot.next;
				slot.no_PROCMSG = 1;
			}
		}
		return msg1.wnd ? true : false;
	}

	// --------------------------------------------------------------------
	// _msg_node
	// --------------------------------------------------------------------

	bool _msg_struct::inherited()
	{
		mapslot *p = static_cast<mapslot*>(cur_slot);
		if (p->next != p)
		{
			_msg_struct tmp = *this;
			tmp.cur_slot = p->next;
			return wndproc::process(tmp);
		}
		return false;
	}

	// --------------------------------------------------------------------
	// getmsg_hooker
	// --------------------------------------------------------------------

	LRESULT CALLBACK getmsg_hooker::hookproc(hooker *p, WPARAM wParam, LPARAM lParam)
	{
		typedef std::pair<const msgmap_t *, const msgmap_t *> pair_type;

		getmsg_hooker &h = *static_cast<getmsg_hooker *>(p);
		HHOOK hHook = h.m_hook;
		int code = h.code;

		if (code >= 0 && h.m_slot)
		{
			msgmap_t v;
			v.message = WM_WTL;
			v.wParam = SM_HOOK_GETMSG;
			v.ctrlid = reinterpret_cast<MSG *>(lParam)->message;

			pair_type pr = std::equal_range(h.m_slot->entries, h.m_slot->entries + h.m_slot->entries_count, v, &compare_by_notify);
			if (pr.first != pr.second)
			{
				const msgmap_t &v = *pr.first;
				msg_hook msg = { 0 };
				msg.code = code;
				msg.wParam = wParam;
				msg.lParam = lParam;
				v.invoke(v, h.m_slot->wnd, reinterpret_cast<msg_struct &>(msg));
			}
		}

		return ::CallNextHookEx(hHook, code, wParam, lParam);
	}
}