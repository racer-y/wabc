#include "w_comctrl.h"

namespace wabc
{
	// --------------------------------------------------------------------
	// image_list
	// --------------------------------------------------------------------

	void image_list::destroy()
	{
		if (m_hImageList)
		{
			ImageList_Destroy(m_hImageList);
			m_hImageList = 0;
		}
	}

	// --------------------------------------------------------------------

	bool image_list::load_from_resource(size_t resc, int width, DWORD crMask)
	{
		assert(!m_hImageList);
		const int nGrow = 4;

		m_hImageList = ::ImageList_LoadBitmap(*g_app,
			MAKEINTRESOURCE(resc), width, nGrow, crMask);

		return m_hImageList != 0;
	}

	// --------------------------------------------------------------------

	bool image_list::create(int nWidth, int nHeight, DWORD dwFlags)
	{
		assert(m_hImageList == 0);
		const int nSize = 0;
		const int nGrow = 4;

		m_hImageList = ::ImageList_Create(nWidth, nHeight, dwFlags,nSize, nGrow);
		return m_hImageList != 0;
	}

	// --------------------------------------------------------------------
	// treeview
	// --------------------------------------------------------------------

	size_t treeview::get_item_data(HTREEITEM ti)const
	{
		TVITEMEX tvi;

		tvi.mask = TVIF_PARAM;
		tvi.hItem = ti;
		const BOOL b = TreeView_GetItem(m_hWnd, &tvi);
		return b ? tvi.lParam : 0;
	}

	// --------------------------------------------------------------------

	void treeview::set_item_data(HTREEITEM ti, size_t nData)
	{
		TVITEMEX tvi;

		tvi.mask = TVIF_PARAM;
		tvi.hItem = ti;
		tvi.lParam = nData;
		BOOL b = TreeView_SetItem(m_hWnd, &tvi);
		assert(b != FALSE);
	}

	// --------------------------------------------------------------------

	bool treeview::set_item_children(HTREEITEM ti, int cChildren)
	{
		TVITEMEX tvi;

		tvi.mask = TVIF_CHILDREN;
		tvi.hItem = ti;
		tvi.cChildren = cChildren;
		const BOOL b = TreeView_SetItem(m_hWnd, &tvi);
		assert(b != FALSE);
		return b != FALSE;
	}

	// --------------------------------------------------------------------

	bool treeview::get_item_text(HTREEITEM ti, wchar_t *text, size_t n)const
	{
		TVITEMEX tvi;

		tvi.mask = TVIF_TEXT;
		tvi.hItem = ti;
		tvi.pszText = text;
		tvi.cchTextMax = n;

		const BOOL b = TreeView_GetItem(m_hWnd, &tvi);
		return b != FALSE;
	}

	// --------------------------------------------------------------------

	bool treeview::get_item_text(HTREEITEM ti, string &s)const
	{
		TVITEMEX tvi;
		s.resize(255);
		tvi.mask = TVIF_TEXT;
		tvi.hItem = ti;
		tvi.pszText = &s[0];
		tvi.cchTextMax = s.size();

		const BOOL b = TreeView_GetItem(m_hWnd, &tvi);
		return b != FALSE;
	}

	// --------------------------------------------------------------------

	bool treeview::set_item_text(HTREEITEM ti, const wchar_t *text, size_t n)
	{
		TVITEMEX tvi;

		tvi.mask = TVIF_TEXT;
		tvi.hItem = ti;
		tvi.pszText = (LPTSTR)text;
		tvi.cchTextMax = n;
		const BOOL b = TreeView_SetItem(m_hWnd, &tvi);
		return b != FALSE;
	}

	// --------------------------------------------------------------------
	// tabctrl
	// --------------------------------------------------------------------

	size_t tabctrl::add_tab(string::const_pointer lpszText, size_t n, int nImageIndex)
	{
		TCITEM item = { 0 };

		item.mask = nImageIndex >0 ? TCIF_IMAGE | TCIF_TEXT : TCIF_TEXT;
		item.pszText = const_cast< string::pointer >(lpszText);
		item.cchTextMax = int(n);
		item.iImage = nImageIndex;

		const size_t result = TabCtrl_InsertItem(m_hWnd, TabCtrl_GetItemCount(m_hWnd), &item);
		return result;
	}

	// --------------------------------------------------------------------
	// tooltips
	// --------------------------------------------------------------------

	void tooltips::before_create(CREATESTRUCT &cs)
	{
		inherited::before_create(cs);
		// ToolTips必须是个Popup窗口
		cs.style |= WS_POPUP;
		cs.dwExStyle |= WS_EX_TOOLWINDOW;
	}

	// --------------------------------------------------------------------

	bool tooltips::add(HWND hWnd, string::const_pointer lpszTips)
	{
		TOOLINFO toolInfo = { 0 };
		toolInfo.cbSize = 44;// sizeof(TOOLINFO);
		toolInfo.hwnd = ::GetParent(m_hWnd);
		toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		toolInfo.uId = (UINT_PTR)hWnd;
		toolInfo.lpszText = LPWSTR(lpszTips);
		const BOOL b= SendMessage(m_hWnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
		return b == TRUE;
	}

	// --------------------------------------------------------------------
	// dptctrl::as_time_t
	// --------------------------------------------------------------------

	bool dptctrl::as_c_time_t::get(time_t &t)const
	{
		SYSTEMTIME st;
		FILETIME ft;
		LRESULT b = DateTime_GetSystemtime(_this->m_hWnd, &st);
		if (b == GDT_VALID)
		{
			::SystemTimeToFileTime(&st, &ft);
			
			ULARGE_INTEGER ui;

			ui.LowPart = ft.dwLowDateTime;

			ui.HighPart = ft.dwHighDateTime;

			t = (time_t)((LONGLONG)(ui.QuadPart - 116444736000000000) / 10000000);

			return true;
		}
		return false;
	}

	// --------------------------------------------------------------------

	void dptctrl::as_c_time_t::operator = (const time_t &t)
	{
		SYSTEMTIME st;
		FILETIME ft;
		LONGLONG nLL = Int32x32To64(t, 10000000) + 116444736000000000;

		ft.dwLowDateTime = (DWORD)nLL;
		ft.dwHighDateTime = (DWORD)(nLL >> 32);

		FileTimeToSystemTime(&ft, &st);
		const LRESULT b = DateTime_SetSystemtime(_this->m_hWnd, GDT_VALID, &st);
		assert(b != FALSE);
	}
}