#include "w_menu.h"

namespace wabc
{
	// --------------------------------------------------------------------
	// class menu_item_base
	// --------------------------------------------------------------------

	void menu_item_base::insert(size_t nPos, size_t nId, const wchar_t * szText, size_t n)
	{
		if (m_hMenu == 0)
		{
			m_hMenu = create_submenu();
			assert(m_hMenu != 0);
		}

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));

		miif.cbSize = sizeof(miif);
		miif.fMask = MIIM_ID | MIIM_STRING;
		miif.wID = nId;
		miif.dwTypeData = const_cast<string::pointer>(szText);
		miif.cch = n;

		bool result = ::InsertMenuItem(m_hMenu, nPos, true, &miif) != 0;

		//api_check(result, "InsertMenuItem");
	}

	// --------------------------------------------------------------------

	void menu_item_base::insert_separator(size_t nPos)
	{
		if (m_hMenu == 0)
		{
			m_hMenu = create_submenu();
			assert(m_hMenu != 0);
		}

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));

		miif.cbSize = sizeof(miif);
		miif.fType |= MFT_SEPARATOR;
		miif.fMask |= MIIM_FTYPE;

		bool result = ::InsertMenuItem(m_hMenu, nPos, true, &miif) != 0;
		//api_check(result, "InsertMenuItem");
	}

	// --------------------------------------------------------------------

	menu_item menu_item_base::operator[](size_t n)const
	{
		assert(m_hMenu != 0);

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));

		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_SUBMENU | MIIM_ID;

		bool result = ::GetMenuItemInfo(m_hMenu, n&~BYID, n & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");

		return menu_item(miif.hSubMenu, m_hMenu, n);
	}

	// --------------------------------------------------------------------

	size_t menu_item_base::popup(HWND hWnd, LONG x, LONG y,
		size_t nFlags)const
	{
		assert(m_hMenu != 0);
		::SetForegroundWindow(hWnd);

		//		wabc::point pt( x, y );
		//		::ClientToScreen(hWnd, &pt);

		// Display the menu
		size_t result = ::TrackPopupMenu(m_hMenu, nFlags, x, y, 0, hWnd, 0);

		::PostMessage(hWnd, WM_NULL, 0, 0);

		return result;
	}

	// --------------------------------------------------------------------
	// menu_item::text_t
	// --------------------------------------------------------------------

	size_t menu_item::text_t::get(string::value_type * szBuf, size_t n)const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;
		
		string s(n, 0);

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask = MIIM_STRING;

//		string::char_type szBuf[128] = { 0 };

		miif.dwTypeData = szBuf;
		miif.cch = n + 1;

		const bool b = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		return b ? miif.cch : 0;
		//api_check(b, "GetMenuItemInfo");
	}

	// --------------------------------------------------------------------

	string & menu_item::text_t::get(string &s)const
	{
		s.resize(255);
		const size_t n = get(&s[0], 255 + 1);
		s.resize(n);
		return s;
	}

	// --------------------------------------------------------------------

	void menu_item::text_t::assign(string::const_pointer lpsz, size_t n)
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t & nIndex = m_index;
		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));

		miif.cbSize = sizeof(miif);
		miif.fMask = MIIM_STRING;
		miif.dwTypeData = const_cast<string::pointer>(lpsz);
		miif.cch = n;
		bool result = ::SetMenuItemInfo(hParentMenu, nIndex, true, &miif) != 0;

		//api_check(result, "SetMenuItemInfo");
	}

	// --------------------------------------------------------------------
	// menu_item::id_t
	// --------------------------------------------------------------------

	menu_item::id_t::operator size_t()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_ID;
		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");
		return miif.wID;
	}

	// --------------------------------------------------------------------
	// menu_item::data_t
	// --------------------------------------------------------------------

	size_t menu_item::data_t::get()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_DATA;
		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");
		return miif.dwItemData;
	}

	// --------------------------------------------------------------------

	void menu_item::data_t::set(size_t n)
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_DATA;
		miif.dwItemData = n;
		bool result = ::SetMenuItemInfo(hParentMenu, nIndex, true, &miif) != 0;
		//api_check(result, "SetMenuItemInfo");
	}

	// --------------------------------------------------------------------
	// menu_item::enabled_t
	// --------------------------------------------------------------------

	menu_item::enabled_t::operator bool()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_STATE;

		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");
		return (miif.fState & MFS_DISABLED) != MFS_DISABLED;
	}

	// --------------------------------------------------------------------

	menu_item::enabled_t & menu_item::enabled_t::operator=(bool bEnable)
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fState |= bEnable ? MFS_ENABLED : MFS_DISABLED;
		miif.fMask |= MIIM_STATE;

		bool result = ::SetMenuItemInfo(hParentMenu, nIndex, true, &miif) != 0;
		//api_check(result, "SetMenuItemInfo");
		return *this;
	}

	// --------------------------------------------------------------------
	// menu_item::checked_t
	// --------------------------------------------------------------------

	menu_item::checked_t::operator bool()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_STATE;

		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");
		return (miif.fState & MFS_CHECKED) != 0;
	}

	// --------------------------------------------------------------------

	menu_item::checked_t & menu_item::checked_t::operator=(bool bCheck)
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_STATE;

		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");

		if (bCheck)
			miif.fState |= MFS_CHECKED;
		else
			miif.fState &= ~MFS_CHECKED;

		miif.fMask |= MIIM_STATE;
		result = ::SetMenuItemInfo(hParentMenu, nIndex, true, &miif) != 0;
		//api_check(result, "SetMenuItemInfo");

		return *this;
	}

	// --------------------------------------------------------------------
	// menu_item::radio_checked_t
	// --------------------------------------------------------------------

	menu_item::radio_checked_t::operator bool()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_FTYPE;

		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
		//api_check(result, "GetMenuItemInfo");
		return (miif.fType & MFT_RADIOCHECK) != 0;
	}

	// --------------------------------------------------------------------

	menu_item::radio_checked_t &
		menu_item::radio_checked_t::operator=(bool bCheck)
	{
			const HMENU &hParentMenu = m_hParentMenu;
			const size_t	& nIndex = m_index;

			MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
			miif.cbSize = sizeof(miif);
			miif.fMask |= MIIM_FTYPE | MIIM_CHECKMARKS | MIIM_STATE;

			bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;
			//api_check(result, "GetMenuItemInfo");

			if (bCheck)
			{
				miif.fType |= MFT_RADIOCHECK;
				miif.fState |= MFS_CHECKED;
			}
			else
			{
				miif.fType &= ~MFT_RADIOCHECK;
				//			miif.fState&= ~MFS_CHECKED;
			}

			result = ::SetMenuItemInfo(hParentMenu, nIndex, true, &miif) != 0;
			//api_check(result, "SetMenuItemInfo");

			return *this;
		}

	// --------------------------------------------------------------------
	// menu_item::owner_draw_t
	// --------------------------------------------------------------------

	menu_item::owner_draw_t::operator bool()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_FTYPE;

		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;

		//api_check(result, "GetMenuItemInfo");
		return (miif.fType & MFT_OWNERDRAW) != 0;
	}

	// --------------------------------------------------------------------

	menu_item::owner_draw_t & menu_item::owner_draw_t::operator=(bool b)
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));
		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_FTYPE;

		if (b)
			miif.fType |= MFT_OWNERDRAW;
		else
			miif.fType &= ~MFT_OWNERDRAW;

		bool result = ::SetMenuItemInfo(hParentMenu, nIndex, true, &miif) != 0;
		//api_check(result, "SetMenuItemInfo");
		return *this;
	}

	// --------------------------------------------------------------------
	// menu_item::separator_t
	// --------------------------------------------------------------------

	menu_item::separator_t::operator bool()const
	{
		const HMENU &hParentMenu = m_hParentMenu;
		const size_t	& nIndex = m_index;

		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));

		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_FTYPE;
		bool result = ::GetMenuItemInfo(hParentMenu, nIndex & ~BYID, nIndex & BYID ? FALSE : TRUE, &miif) != 0;

		//api_check(result, "GetMenuItemInfo");
		return (miif.fType & MFT_SEPARATOR) != 0;
	}

	// --------------------------------------------------------------------
	// class menu_item
	// --------------------------------------------------------------------

	HMENU menu_item::create_submenu()const
	{
		MENUITEMINFO miif; ::memset(&miif, 0, sizeof(miif));

		miif.cbSize = sizeof(miif);
		miif.fMask |= MIIM_SUBMENU;
		miif.hSubMenu = ::CreateMenu();

		//api_check(miif.hSubMenu != 0, "CreateMenu");

		bool b = ::SetMenuItemInfo(id.m_hParentMenu, id.m_index, true, &miif) != 0;
		return miif.hSubMenu;
	}

	// --------------------------------------------------------------------
	// class popup_menu
	// --------------------------------------------------------------------

	BOOL popup_menu::show(HMENU hMenu, LONG x, LONG y)
	{
		::SetForegroundWindow(m_hWnd);

//		POINT pt = { x, y };
//		::ClientToScreen(m_hWnd, &pt);

		// Display the menu
		const BOOL bResult = ::TrackPopupMenu(hMenu, m_flags,
			x, y, 0, m_hWnd, 0);

		::PostMessage(m_hWnd, WM_NULL, 0, 0);
		return bResult;
	}
}