#pragma once

#include "w_wndbase.h"

namespace wabc
{
	// --------------------------------------------------------------------

	class menu_item;

	class menu_item_base
	{
	protected:

		explicit menu_item_base(HMENU hMenu) : m_hMenu(hMenu)
		{}

		virtual HMENU create_submenu()const
		{
			return 0;
		}

	public:

		HMENU	m_hMenu;

		virtual ~menu_item_base(){}

		size_t size()const
		{
			const size_t nSize = ::GetMenuItemCount(m_hMenu);
			return nSize == size_t(-1) ? 0 : nSize;
		}

		void insert_separator(size_t nPos);
		void insert(size_t nPos, size_t nId, const wchar_t * szText, size_t n);
		void insert(size_t nPos, size_t nId, const wchar_t * szText)
		{
			insert(nPos, nId, szText, ::wcslen(szText));
		}

		void append_separator()
		{
			insert_separator(size());
		}

		void append(size_t nId, const string & strText)
		{
			insert(size(), nId, strText.c_str(), strText.size());
		}

		void append(size_t nId, const wchar_t *szText)
		{
			insert(size(), nId, szText, ::wcslen(szText));
		}

		void append(size_t nId, const wchar_t *szText, size_t n)
		{
			insert(size(), nId, szText, n);
		}

		void erase(size_t n)
		{
			assert(n < UINT(-1));
			bool result = ::DeleteMenu(m_hMenu, (UINT)n, MF_BYPOSITION) != 0;
		}

		void erase(size_t nFirst, size_t nLast)
		{
			size_t n = nLast - nFirst;
			for (; n > 0; --n)
				erase(nFirst);
		}

		void clear()
		{
			for (size_t i = size(); i > 0;)
				erase(--i);
		}

		enum{ BYID = 0X80000000 };
		menu_item operator[](size_t n)const;

		friend class menu;

	public:

		size_t popup(HWND hWnd, LONG x, LONG y,
			size_t nFlags = TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN)const;
	};

	// --------------------------------------------------------------------

	class menu_item : public menu_item_base
	{
		struct union_property
		{
			HMENU	m_hParentMenu;
			size_t	m_index;
		};

	public: // menu item's properties

		// --------------------------------------------------------------------

		struct index_t : union_property
		{
			operator size_t()const { return m_index; }
			size_t operator()()const { return *this; }
		};

		// --------------------------------------------------------------------

		struct text_t : union_property
		{
			operator string()const
			{
				string s(255, 0);
				return get(s);
			}
			string operator()()const { return *this; }

			string & get(string &s)const;
			void operator=(const string &s)
			{
				assign(s.c_str(), s.size());
			}
			void operator=(string::const_pointer lpsz)
			{
				assign(lpsz);
			}

			size_t get(string::value_type * szBuf, size_t n)const;

			template<size_t N>
			size_t get(wchar_t(&text)[N])
			{
				return get(text, N);
			}

			void assign(string::const_pointer lpsz, size_t n);
			void assign(string::const_pointer lpsz)
			{
				assign(lpsz, ::wcslen(lpsz));
			}
		};

		// --------------------------------------------------------------------

		struct id_t : union_property
		{
			operator size_t()const;
			size_t operator()()const { return *this; }

			//			id_t & operator=( size_t n );
		};

		// --------------------------------------------------------------------

		struct  data_t : union_property
		{
			size_t get()const;
			void set(size_t n);

			template< typename T >
			data_t & operator=(T &t)
			{
				__abc_static_assert(abc::is_pod< typename abc::remove_cv<T>::type >::value);
				set(size_t(t));
				return *this;
			}

			template< typename T >
			operator T()const
			{
				__abc_static_assert(abc::is_pod< typename abc::remove_cv<T>::type >::value);
				size_t n = get();
				return T(n);
			}
		};

		// --------------------------------------------------------------------

		struct enabled_t : union_property
		{
			operator bool()const;
			bool operator()()const { return *this; }

			enabled_t & operator=(bool n);
		};

		// --------------------------------------------------------------------

		struct checked_t : union_property
		{
			operator bool()const;
			bool operator()()const { return *this; }

			checked_t & operator=(bool n);
		};

		// --------------------------------------------------------------------

		struct radio_checked_t : union_property
		{
			operator bool()const;
			bool operator()()const { return *this; }

			radio_checked_t & operator=(bool n);
		};

		// --------------------------------------------------------------------

		struct owner_draw_t : union_property
		{
			operator bool()const;
			bool operator()()const { return *this; }

			owner_draw_t & operator=(bool b);
		};

		// --------------------------------------------------------------------

		struct separator_t : union_property
		{
			operator bool()const;
			bool operator()()const { return *this; }

			separator_t & operator=(bool b);
		};

	protected:

		virtual HMENU create_submenu()const;

	public:

		union
		{
			id_t			id;
			text_t			text;
			index_t			index;
			data_t			data;
			checked_t		checked;
			enabled_t		enabled;
			separator_t		separator;
			owner_draw_t	owner_draw;
			radio_checked_t	radio_checked;
		};

		menu_item(HMENU hMenu, HMENU hParentMenu, size_t nIndex): menu_item_base(hMenu)
		{
			id.m_hParentMenu = hParentMenu;
			id.m_index = nIndex;
		}

		virtual ~menu_item() {}
	};

	// --------------------------------------------------------------------
	// class menu
	// --------------------------------------------------------------------

	class menu : public menu_item_base
	{
		menu(const menu &);
		menu & operator=(const menu &);

	public:

		menu() : menu_item_base(0){}

		virtual ~menu()
		{
			destroy();
		}

	public:

		bool create()
		{
			assert(m_hMenu == 0);
			m_hMenu = ::CreateMenu();
			return m_hMenu != 0;
		}

		bool load_from_resource(size_t nResourceId)
		{
			assert(m_hMenu == 0);
			m_hMenu = ::LoadMenu(*g_app, MAKEINTRESOURCE(nResourceId));
			return m_hMenu != 0;
		}

		bool load_from_resource(size_t nResourceId, HINSTANCE hInstance)
		{
			assert(m_hMenu == 0);
			m_hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(nResourceId));
			return m_hMenu != 0;
		}

		void destroy()
		{
			if (m_hMenu != 0)
			{
				if (::DestroyMenu(m_hMenu))
					m_hMenu = 0;
			}
		}

		void attach(HWND hWnd)
		{
			::SetMenu(hWnd, this->m_hMenu);
		}
	};

	// --------------------------------------------------------------------
	// class popup_menu
	// --------------------------------------------------------------------

	class popup_menu : public menu
	{
		HWND		m_hWnd;
		DWORD	m_flags;
	public:
		enum{ default_flags = TPM_RIGHTBUTTON | TPM_TOPALIGN | TPM_LEFTALIGN };

		explicit popup_menu(HWND hWnd, DWORD f = default_flags) 
			: m_hWnd(hWnd), m_flags(f) {}

		virtual ~popup_menu(){}

		// logical coordinate
		BOOL show(HMENU hMenu, LONG x, LONG y);
		BOOL show(const menu_item_base &mib, LONG x, LONG y)
		{
			return show(mib.m_hMenu, x, y);
		}

		BOOL show(LONG x, LONG y)
		{
			return show((*this)[0], x, y);
		}
	};
}