#include "w_dialog.h"
#include "w_dlgresc.h"
#include "w_proc.h"

namespace wabc
{
	bool is_my_child(HWND hOwner, HWND hChild)
	{
		if (hOwner == hChild)
			return true;

		HWND hParent;
		while (hParent = ::GetParent(hChild))
		{
			if (hParent == hOwner)
				return true;

			const DWORD nStyle = ::GetWindowLong(hParent, GWL_STYLE);
			if (nStyle & (WS_POPUP | WS_OVERLAPPED))
				return false;
			hChild = hParent;
		}

		return false;
	}

	// --------------------------------------------------------------------

	static void load_template(dialog::create_param &param, size_t resid)
	{
		HRSRC hResource = ::FindResource(param.hInstance, MAKEINTRESOURCE(resid), RT_DIALOG);
		assert(hResource);
		HGLOBAL hDialogTemplate = ::LoadResource(param.hInstance, hResource);
		assert(hDialogTemplate);

		param.dlgtmpl = (char*)::LockResource(hDialogTemplate);

		DLGTEMPLATEEX *lpDialogTemplate = (DLGTEMPLATEEX*)param.dlgtmpl;
		DWORD style;
		uint8 *pb1 = (uint8*)lpDialogTemplate;
		if (lpDialogTemplate->signature != 0xFFFF)
		{
			style = reinterpret_cast<DLGTEMPLATE *>(lpDialogTemplate)->style;
			pb1 += sizeof(DLGTEMPLATE);
		}
		else
		{
			style = lpDialogTemplate->style;
			pb1 += sizeof(*lpDialogTemplate);
		}

		//menu
		WORD *pw = reinterpret_cast< WORD *>(pb1);
		assert(*pw == 0);

		// window class
		++pw;
		assert(*pw == 0);
		if (*pw == 0xFFFF)
			++pw;
		else
		for (; *pw != 0; ++pw);

		// title
		++pw;
		wchar_t *pwchar = reinterpret_cast< wchar_t *>(pw);
		for (; *pwchar != 0; ++pwchar);

		// font
		++pwchar;
		if ((style & (DS_SETFONT | 0x0008L)) != 0)
		{
			pwchar += 3; // pointsize - WORD, weight-WORD,italic-BYTE, charset-BYTE;
			for (; *pwchar != 0; ++pwchar);
			++pwchar;
		}

		// DWORD boundary
		size_t n = size_t(pwchar);
		n += 3;
		n >>= 2;
		n <<= 2;

		param.itemtmpl = reinterpret_cast< char *>(n);
	}

	// --------------------------------------------------------------------
	// dialog
	// --------------------------------------------------------------------

	dialog::dialog()
	{
		WABC_BEGIN_MSG_MAP(self)
			//WABC_ON_SIZE(&self::on_size)
			WABC_ON_SYSCOMMAND(SC_CLOSE, &self::on_sc_close)
			WABC_ON_HOOK_GETMSG(WM_KEYDOWN, &self::on_msg_intercept)
			WABC_ON_SYSMSG(SM_TAB_NEXT, &self::on_tab_next)
			WABC_ON_SYSMSG(SM_TAB_PRIOR, &self::on_tab_prior)
		WABC_END_MSG_MAP()
		items._this = this;
	}
		
	// --------------------------------------------------------------------

	bool dialog::create(HINSTANCE hInstance, size_t resid, HWND hParent)
	{
		create_param param;
		param.hInstance = hInstance;
		load_template(param, resid);

		const size_t n = param.itemtmpl - param.dlgtmpl;
		DLGTEMPLATEEX *p= (DLGTEMPLATEEX*)alloca(n);
		::memcpy(p, param.dlgtmpl, n);
		if(p->signature == 0xFFFF)
			p->cDlgItems= 0;
		else
			reinterpret_cast<DLGTEMPLATE*>(p)->cdit= 0;

		param.dlg = this;
		::CreateDialogIndirectParam(hInstance, LPCDLGTEMPLATE(p),
			hParent, wndproc::dlg_init, LPARAM(&param));
		return m_hWnd != 0;
	}

	// --------------------------------------------------------------------

	int dialog::show_modal(HINSTANCE hInstance, size_t resid, HWND hParent)
	{
		create_param param;
		param.hInstance = hInstance;
		load_template(param, resid);

		const size_t n = param.itemtmpl - param.dlgtmpl;
		DLGTEMPLATEEX *p= (DLGTEMPLATEEX*)alloca(n);
		::memcpy(p, param.dlgtmpl, n);
		if(p->signature == 0xFFFF)
			p->cDlgItems= 0;
		else
			reinterpret_cast<DLGTEMPLATE*>(p)->cdit= 0;

		param.dlg = this;
		const int result = pfnDialogBoxIndirectParam(hInstance, LPCDLGTEMPLATE(p),
			hParent, wndproc::dlg_init, LPARAM(&param));

		return result;
	}

	// --------------------------------------------------------------------

	bool dialog::on_size(msg_size &msg)
	{
		if (msg.is_changed())
		{
			//const LONG w = msg.width;
			//const LONG h = msg.height;

			//const rect rtClient = client_rect();
			//const size_t n = m_dlgctrls.size();
			//rect rt;
			//for (size_t i = 0; i < n; ++i)
			//{
			//	dlgctrl_property &prop = *m_dlgctrls[i];
			//	wndbase *p = m_dlgctrls[i]->control();

			//	rect rt1 = p->window_rect();
			//	screen_to_client(rt1);

			//	rt = prop.m_rect;
			//	if (rt.left < 0)
			//		rt.left += rtClient.right;
			//	if (rt.top < 0)
			//		rt.top += rtClient.bottom;

			//	if (rt.right < 0)
			//		rt.right += rtClient.right;

			//	if (rt.bottom < 0)
			//		rt.bottom += rtClient.bottom;

			//	if (rt != rt1)
			//		p->move(rt);
			//}
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool dialog::on_sc_close(msg_syscommand &msg)
	{
		::PostMessage(m_hWnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
		return true;
	}

	// --------------------------------------------------------------------

	bool dialog::on_msg_intercept(msg_hook &msg)
	{
		assert(msg.pmsg->message == WM_KEYDOWN);
		if (msg.pmsg->wParam == VK_TAB)// || msg.pmsg->wParam == VK_RETURN || msg.pmsg->wParam == VK_ESCAPE)
		{
			keyboard kb;
			const bool bCtrl = kb[VK_CONTROL];
			const bool bMenu = kb[VK_MENU];
			if (bCtrl || bMenu)
				return false;

			if (!is_my_child(m_hWnd, msg.pmsg->hwnd))
				return false;

			HWND hCtrl, hParent = msg.pmsg->hwnd;

			size_t nFlags = DLGC_WANTALLKEYS | DLGC_WANTMESSAGE;

			if (msg.pmsg->wParam == VK_TAB)
				nFlags |= DLGC_WANTTAB;

			while (true)
			{
				hCtrl = hParent;
				hParent = ::GetParent(hCtrl);
				if (!hParent)
					break;

				const size_t n = ::SendMessage(hCtrl, WM_GETDLGCODE, 0, LPARAM(&msg));
				if ((n & nFlags) != 0)
					return true;	// 避免下一次的验证

//				if (hParent == m_hWnd)
				{
					const bool bShift = kb[VK_SHIFT];
//					if (msg.pmsg->wParam == VK_TAB)
					{
						// TAB键的处理
						if (process_tab_keydown(hCtrl, bShift))
						{
							msg.pmsg->message = 0;
							return true;
						}
					}
					/*					else if (!bShift)
					{
						// 回车键和ESC键的处理
						const WPARAM w = msg.pmsg->wParam == VK_RETURN ? MAKEWPARAM(IDOK, BN_CLICKED) : MAKEWPARAM(IDCANCEL, BN_CLICKED);
						const LRESULT ret = ::SendMessage(m_hWnd, WM_COMMAND, w, 0);
						if (ret == 1)
						{
							msg.pmsg->message = 0;
							return true;
						}
					}
					*/					break;
				}

				//const DWORD nStyle = ::GetWindowLong(hParent, GWL_STYLE);
				//if (nStyle & (WS_POPUP | WS_OVERLAPPED))
				//	break;
			}
		}
		return false;
	}

	// --------------------------------------------------------------------

	void dialog::dialog_create(const create_param &param)
	{
//		m_flags = 0;

		static string::const_pointer szSystemClass[] = {
			_T("BUTTON"), _T("EDIT"), _T("STATIC"), _T("LISTBOX"), _T("SCROLLBAR"), _T("COMBOBOX"),
		};

		// create sub controls

		const size_t n = m_dlgctrls.size();
		HFONT hFont = this->font;
		HWND hCtrl= 0;

		dlgitem_template tmpl(param.dlgtmpl, param.itemtmpl);
		size_t i;

		m_tab_ctrls.clear();
		for (bool b = tmpl.first(); b; b = tmpl.next())
		{
			const size_t resc_id = tmpl.id();
			rect rt = tmpl.bound();
			::MapDialogRect(m_hWnd, &rt);

			for (i = 0; i < n; ++i)
			{
				dlgctrl_property &prop = *m_dlgctrls[i];
				if (prop.resource_id == resc_id)
				{
					wndbase &ctrl = *prop.control();
					CREATESTRUCT cs = { 0 };

					cs.hwndParent = m_hWnd;

					cs.x = rt.left; cs.y = rt.top;
					cs.cx = rt.width();
					cs.cy = rt.height();
					cs.lpszName = tmpl.title();
					cs.hMenu = HMENU(tmpl.id());
					cs.style |= tmpl.style();
					cs.dwExStyle |= tmpl.exstyle();
					cs.hInstance = param.hInstance;

					ctrl.before_create(cs);
					if (cs.lpszClass == 0)
					{
						const wchar_t *pWndClass = tmpl.window_class();

						if (*pWndClass == 0xFFFF)
						{
							++pWndClass;
							const size_t n = WORD(*pWndClass) - 0x0080;
							if (n < sizeof(szSystemClass) / sizeof(szSystemClass[0]))
								pWndClass = szSystemClass[n];
						}
						cs.lpszClass = pWndClass;
					}

					ctrl.create(cs);
					hCtrl = ctrl.m_hWnd;
					assert(hCtrl);
					//{
					//	wndbase *p = &ctrl;
					//	prop.m_rect = ctrl.window_rect();

					//	screen_to_client(prop.m_rect);
					//}
					break;
				}
			}

			if (i == n)
			{
				const wchar_t *pWndClass = tmpl.window_class();

				if (*pWndClass == 0xFFFF)
				{
					++pWndClass;
					const size_t n = WORD(*pWndClass) - 0x0080;
					if (n < sizeof(szSystemClass) / sizeof(szSystemClass[0]))
						pWndClass = szSystemClass[n];
				}

				hCtrl = ::CreateWindowEx(tmpl.exstyle(), pWndClass, tmpl.title(),
					tmpl.style(), rt.left, rt.top, rt.width(), rt.height(), m_hWnd,
					(HMENU)tmpl.id(), param.hInstance, tmpl.extract());
			}
			::SendMessage(hCtrl, WM_SETFONT, WPARAM(hFont), true);

			if (tmpl.style() & WS_TABSTOP)
			{
				m_tab_ctrls.push_back(hCtrl);
			}
		}

		WABC_HOOK_GETMSG();
	}

	// --------------------------------------------------------------------

	static void set_focus(HWND hWnd, WPARAM sm)
	{
		::SetFocus(hWnd);
		const LRESULT ret = ::PostMessage(hWnd, WM_WTL, sm, 0);
	}

	// --------------------------------------------------------------------

	bool dialog::process_tab_keydown(HWND hParent, bool bShift)
	{
		const WPARAM wParam = bShift ? SM_TAB_PRIOR : SM_TAB_NEXT;
		HWND hCtrl = 0;

		while (hParent)
		{
			const LRESULT ret = ::SendMessage(hParent, WM_WTL, wParam, LPARAM(hCtrl));
			if (ret == 1)
				return true;
			const DWORD nStyle = ::GetWindowLong(hParent, GWL_STYLE);
			if (nStyle & (WS_POPUP | WS_OVERLAPPED))
				break;
			hCtrl = hParent;
			hParent = ::GetParent(hParent);
		}

		//assert(::GetParent(hCtrl) == m_hWnd);
		//HWND hParent = m_hWnd, hGrandParent;

		//const size_t n = m_tab_ctrls.size();
		//for (i = 0; i < n; ++i)
		//{
		//	if (m_tab_ctrls[i] == hCtrl)
		//	{
		//		if (bShift)
		//		{
		//			// 前一个
		//			if (i == 0)
		//			{
		//				while(true)
		//				{
		//					const DWORD nStyle= ::GetWindowLong(hParent, GWL_STYLE);
		//					if (nStyle & (WS_POPUP | WS_OVERLAPPED))
		//						break;

		//					hGrandParent= ::GetParent(hParent);
		//					if(!hGrandParent)
		//						break;

		//					const LRESULT ret= ::SendMessage(hGrandParent, WM_WTL, SM_TAB_PRIOR, LPARAM(hParent));
		//					if(ret == 1)
		//						return true;

		//					hParent= hGrandParent;
		//				}
		//			}

		//			for(j= i; j > 0;)
		//			{
		//				const DWORD nStyle= ::GetWindowLong(m_tab_ctrls[--j],GWL_STYLE);
		//				if((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
		//				{
		//					if ((nStyle & WS_DISABLED) == 0)
		//					{
		//						wabc::set_focus(m_tab_ctrls[j], SM_TAB_PRIOR);
		//						return true;
		//					}
		//				}
		//			}

		//			for(j= n; j > i;)
		//			{
		//				const DWORD nStyle= ::GetWindowLong(m_tab_ctrls[--j],GWL_STYLE);
		//				if((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
		//				{
		//					wabc::set_focus(m_tab_ctrls[j], SM_TAB_PRIOR);
		//					return true;
		//				}
		//			}
		//		}
		//		else
		//		{
		//			// 下一个
		//			if ((i+1) >= n)
		//			{
		//				while(true)
		//				{
		//					const DWORD nStyle= ::GetWindowLong(hParent,GWL_STYLE);
		//					if (nStyle & (WS_POPUP | WS_OVERLAPPED))
		//						break;

		//					hGrandParent= ::GetParent(hParent);
		//					if(!hGrandParent)
		//						break;

		//					const LRESULT ret= ::SendMessage(hGrandParent, WM_WTL, SM_TAB_NEXT, LPARAM(hParent));
		//					if(ret == 1)
		//						return true;

		//					hParent= hGrandParent;
		//				}
		//			}

		//			for(j= i; ++j < n; )
		//			{
		//				const DWORD nStyle= ::GetWindowLong(m_tab_ctrls[j],GWL_STYLE);
		//				if((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
		//				{
		//					if ((nStyle & WS_DISABLED) == 0)
		//					{
		//						wabc::set_focus(m_tab_ctrls[j], SM_TAB_NEXT);
		//						return true;
		//					}
		//				}
		//			}

		//			for(j= 0; j < i; ++j)
		//			{
		//				const DWORD nStyle= ::GetWindowLong(m_tab_ctrls[j],GWL_STYLE);
		//				if((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
		//				{
		//					if ((nStyle & WS_DISABLED) == 0)
		//					{
		//						wabc::set_focus(m_tab_ctrls[j], SM_TAB_NEXT);
		//						return true;
		//					}
		//				}
		//			}
		//		}
		//	}
		//}
		return false;
	}

	// --------------------------------------------------------------------

	bool dialog::on_tab_next(msg_struct &msg)
	{
		HWND hCtrl = HWND(msg.lParam);
		const size_t n = m_tab_ctrls.size();
		size_t i, j;

		for (i = 0; i < n; ++i)
		{
			if (m_tab_ctrls[i] == hCtrl)
			{
				for (j = i; ++j < n;)
				{
					const DWORD nStyle = ::GetWindowLong(m_tab_ctrls[j], GWL_STYLE);
					if ((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
					{
						wabc::set_focus(m_tab_ctrls[j], SM_TAB_NEXT);
						msg.result = 1;
						return true;
					}
				}

				HWND hParent = ::GetParent(m_hWnd);
				if (hParent)
				{
					msg.result = ::SendMessage(hParent, WM_WTL, SM_TAB_NEXT, LPARAM(m_hWnd));
					if (msg.result == 1)
						return true;
				}

				// 将焦点设置到第一个可见的控件上	
				for (j = 0; j < i; ++j)
				{
					const DWORD nStyle = ::GetWindowLong(m_tab_ctrls[j], GWL_STYLE);
					if ((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
					{
						wabc::set_focus(m_tab_ctrls[j], SM_TAB_NEXT);
						msg.result = 1;
						return true;
					}
				}
				break;
			}
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool dialog::on_tab_prior(msg_struct &msg)
	{
		HWND hCtrl = HWND(msg.lParam);
		const size_t n = m_tab_ctrls.size();
		size_t i, j;
		for (i = 0; i < n; ++i)
		{
			if (m_tab_ctrls[i] == hCtrl)
			{
				for (j = i; j-- > 0;)
				{
					const DWORD nStyle = ::GetWindowLong(m_tab_ctrls[j], GWL_STYLE);
					if ((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
					{
						wabc::set_focus(m_tab_ctrls[j], SM_TAB_PRIOR);
						msg.result = 1;
						return true;
					}
				}

				HWND hParent = ::GetParent(m_hWnd);
				if (hParent)
				{
					msg.result = ::SendMessage(hParent, WM_WTL, SM_TAB_PRIOR, LPARAM(m_hWnd));
					if (msg.result == 1)
						return true;
				}
				// 将焦点设置到倒数第一个可见的控件上
				for (j = n; j-- > i;)
				{
					const DWORD nStyle = ::GetWindowLong(m_tab_ctrls[j], GWL_STYLE);
					if ((nStyle & (WS_VISIBLE | WS_TABSTOP)) == (WS_VISIBLE | WS_TABSTOP))
					{
						wabc::set_focus(m_tab_ctrls[j], SM_TAB_PRIOR);
						msg.result = 1;
						return true;
					}
				}
				break;
			}
		}
		return true;
	}

	// --------------------------------------------------------------------
	// dlgctrl_property
	// --------------------------------------------------------------------

	dlgctrl_property::dlgctrl_property(size_t offset_of_dialog, size_t resc_id)
	{
		uint8 *p = (uint8*)this;
		//::memset(this, 0, sizeof(*this));

		resource_id = resc_id;
		dialog * dlg = (dialog *)(p - offset_of_dialog);
		dlg->m_dlgctrls.push_back(this);
	}
}