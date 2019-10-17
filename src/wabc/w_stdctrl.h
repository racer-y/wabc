#pragma once

#include "w_msg.h"

namespace wabc
{
	// --------------------------------------------------------------------

	class static_text : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return _T("STATIC"); }
			string::const_pointer new_name()const { return _T("wabcstatic"); }
		};

	public:
		
		typedef static_text self;
		typedef scwnd inherited;

		static_text() : inherited(superclass()){}
		virtual ~static_text(){}
	};

	// --------------------------------------------------------------------

	class href_text : public wndbase
	{
		WABC_DECLARE_MSG_MAP()

		enum{ flag_track_leave= 1, flag_lbutton_down= 2,};

		string	m_text;
		HFONT	m_hFont;
		size_t	m_flag;

		void clicked();
	public:
		typedef href_text self;
		typedef wndbase inherited;

		href_text();
		virtual ~href_text(){}

		void auto_resize();
	public:
		bool on_create(msg_create &);

		bool on_paint(dcclass &dc, const rect &rt, wabc::msg_paint &);

		bool on_set_font(msg_setfont &);
		bool on_get_font(msg_getfont &);

		bool on_set_text(msg_settext&);
		bool on_get_text(msg_gettext&);
		bool on_get_text_length(msg_gettextlength &);

		bool on_lbutton_down(msg_mouse &);
		bool on_lbutton_up(msg_mouse &);
		bool on_mouse_move(msg_mouse &);
		bool on_ldbclicked(msg_mouse &);
		bool on_mouse_leave(msg_struct &msg);

		bool on_set_cursor(msg_setcursor &);
		bool on_key_down(msg_keydown &);

		bool on_set_or_kill_focus(msg_setfocus &);
	};

	// --------------------------------------------------------------------

	class button : public scwnd
	{
	public:

		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return _T("BUTTON"); }
			string::const_pointer new_name()const { return _T("wabcbutton"); }
		};

	public:

		typedef button self;
		typedef scwnd inherited;

		button() : inherited(superclass()){}
		virtual ~button(){}
	};

	// --------------------------------------------------------------------

	class radiobox : public scwnd
	{
	public:

		typedef button::superclass superclass;

		typedef radiobox self;
		typedef scwnd inherited;

		radiobox() : inherited(superclass()){}
		virtual ~radiobox(){}

		virtual void before_create(CREATESTRUCT &cs);
	public:

		bool checked()const
		{
			return ::SendMessage(m_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
		}

		void check(bool b)
		{
			::SendMessage(m_hWnd, BM_SETCHECK, b ? BST_CHECKED : BST_UNCHECKED, 0);
		}
	};

	// --------------------------------------------------------------------

	class checkbox : public scwnd
	{
	public:

		typedef button::superclass superclass;

		typedef checkbox self;
		typedef scwnd inherited;

		checkbox() : inherited(superclass()){}
		virtual ~checkbox(){}

		virtual void before_create(CREATESTRUCT &cs);
	public:

		bool checked()const
		{
			return ::SendMessage(m_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
		}

		void check(bool b)
		{
			::SendMessage(m_hWnd, BM_SETCHECK, b ? BST_CHECKED : BST_UNCHECKED, 0);
		}
	};

	// --------------------------------------------------------------------

	class editbox : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return _T("EDIT");; }
			string::const_pointer new_name()const { return _T("wabceditbox"); }
		};

		typedef union_attr<editbox> union_property;

		// --------------------------------------------------------------------

		struct read_only_t : union_property
		{
			operator bool()const
			{
				return (_this->style() & ES_READONLY) != 0;
			}

			bool operator()()const { return *this; }

			bool operator=(bool b)
			{
				::SendMessage(_this->m_hWnd, EM_SETREADONLY, b ? TRUE : FALSE, 0);
				return b;
			}
		};

		// --------------------------------------------------------------------

		struct selected_t : union_property
		{
			int pos()const;

			void get(size_t &nStart, size_t &nEnd)const;

			void set(size_t nStart, size_t nEnd)
			{
				::SendMessage(_this->m_hWnd, EM_SETSEL, WPARAM(nStart), WPARAM(nEnd));
			}


			void operator=(string::const_pointer lpsz)
			{
				::SendMessage(_this->m_hWnd, EM_REPLACESEL, WPARAM(TRUE), LPARAM(lpsz));
			}

			void operator=(const string &s)
			{
				::SendMessage(_this->m_hWnd, EM_GETSEL, WPARAM(TRUE), LPARAM(s.c_str()));
			}
		};
	public:
		union
		{
			const style_template< ES_PASSWORD >	password; \
			style_template< ES_NUMBER >		only_number; \
			style_template< ES_UPPERCASE >	uppercase; \
			style_template< ES_LOWERCASE >	lowercase; \
			style_template< ES_WANTRETURN >	want_return; \
			style_template< ES_MULTILINE >	multiline; \
			style_template< ES_AUTOVSCROLL >	auto_vscroll; \
			read_only_t	read_only; \
			selected_t selected;
		};
		
		typedef editbox self;
		typedef scwnd inherited;

		editbox() : inherited(superclass()){ only_number._this = this; }
		virtual ~editbox(){}
	};
	// --------------------------------------------------------------------

	class combo_box : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return _T("COMBOBOX"); }
			string::const_pointer new_name()const { return _T("wabccombo_box"); }
		};

		// --------------------------------------------------------------------

		typedef wabc::union_attr<combo_box> union_property;

	public:	// combo box's properties

		enum {
			simple_style = CBS_SIMPLE, dropdown_style = CBS_DROPDOWN,
			dropdown_list_style = CBS_DROPDOWNLIST
		};

		// --------------------------------------------------------------------

		struct selected_t : union_property
		{
			operator size_t()const
			{
				const LRESULT n = ::SendMessage(_this->m_hWnd, CB_GETCURSEL, 0, 0);
				return size_t(n);
			}

			size_t operator()()const { return *this; }

			size_t operator= (size_t n)
			{
				::SendMessage(_this->m_hWnd, CB_SETCURSEL, n, 0);
				return n;
			}
		};

		// --------------------------------------------------------------------

		struct top_index_t : union_property
		{
			operator size_t()const
			{
				const LRESULT n = ::SendMessage(_this->m_hWnd, CB_GETTOPINDEX, 0, 0);
				return size_t(n);
			}

			size_t operator()()const { return *this; }

			size_t operator= (size_t n)
			{
				const LRESULT ret = ::SendMessage(_this->m_hWnd, CB_SETTOPINDEX, n, 0);
				return n;
			}
		};

		// --------------------------------------------------------------------
		// 下拉框的高度是combox的高度

		struct drop_down_width_t : union_property
		{
			operator size_t()const
			{
				const LRESULT n = ::SendMessage(_this->m_hWnd, CB_GETDROPPEDWIDTH, 0, 0);
				return size_t(n);
			}

			size_t operator()()const { return *this; }

			void operator= (size_t n)
			{
				const LRESULT ret = ::SendMessage(_this->m_hWnd, CB_SETDROPPEDWIDTH, WPARAM(n), 0);
			}
		};

		// --------------------------------------------------------------------

		union drop_down_t
		{
			drop_down_width_t	width;

			void show()
			{
				::SendMessage(width._this->m_hWnd, CB_SHOWDROPDOWN, WPARAM(TRUE), 0);
			}

			void hide()
			{
				::SendMessage(width._this->m_hWnd, CB_SHOWDROPDOWN, WPARAM(FALSE), 0);
			}

			bool is_show()const
			{
				return ::SendMessage(width._this->m_hWnd, CB_GETDROPPEDSTATE, 0, 0) == TRUE;
			}
		};

		// --------------------------------------------------------------------

		union owner_draw_t
		{
			style_template< CBS_OWNERDRAWFIXED,
			CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE >	fixed;

			style_template< CBS_OWNERDRAWVARIABLE,
				CBS_OWNERDRAWFIXED | CBS_OWNERDRAWVARIABLE >	variable;
		};

		// --------------------------------------------------------------------

		union view_style_t
		{
			enum{ mask = simple_style | dropdown_style | dropdown_list_style };

			style_template< simple_style, mask >	simple;
			style_template< dropdown_style, mask >	dropdown;
			style_template< dropdown_list_style, mask >	dropdown_list;

			operator size_t()const
			{
				const size_t n = simple._this->style();
				return (n & mask);
			}

			size_t operator()()const { return *this; }

			size_t operator=(size_t nStyle)
			{
				size_t n = simple._this->style();
				n &= ~mask;
				n |= nStyle;
				simple._this->style = n;
				return nStyle;
			}
		};

	public:
		union {
			selected_t		selected;
			top_index_t		top_index;

			drop_down_t		drop_down;
			view_style_t	view_style;
			owner_draw_t	owner_draw;

			style_template< CBS_SORT >			enable_sort;
			style_template< CBS_UPPERCASE >		uppercase;
			style_template< CBS_LOWERCASE >		lowercase;
			style_template< CBS_AUTOHSCROLL >	auto_hscroll;
			style_template<CBS_DISABLENOSCROLL>	no_scroll;
		};

	public:
		typedef combo_box self;
		typedef scwnd inherited;

		combo_box() : inherited(superclass())
		{
			selected._this = this;
		}
		virtual ~combo_box(){}

	public:

		size_t find(string::const_pointer lpsz, size_t start_after = size_t(-1))const
		{
			return (size_t)::SendMessage(m_hWnd, CB_FINDSTRING, WPARAM(start_after), LPARAM(lpsz));
		}

		size_t find(const string &s, size_t start_after = size_t(-1))const
		{
			return (size_t)::SendMessage(m_hWnd, CB_FINDSTRING, WPARAM(start_after), LPARAM(s.c_str()));
		}

		bool get_item_text(size_t index, string &s)const
		{
			const size_t n = ::SendMessage(m_hWnd, CB_GETLBTEXTLEN, index, 0);
			s.resize(n);
			const LRESULT ret= ::SendMessage(m_hWnd, CB_GETLBTEXT, index, LPARAM(&s[0]));
			return ret != CB_ERR;
		}

		bool get_item_text(size_t index, string::pointer lpszText)const
		{
			const LRESULT ret = ::SendMessage(m_hWnd, CB_GETLBTEXT, index, LPARAM(lpszText));
			return ret != CB_ERR;
		}

		size_t get_item_data(size_t index)const
		{
			LRESULT n = ::SendMessage(m_hWnd, CB_GETITEMDATA, index, 0);
			return n;
		}

		bool set_item_data(size_t index, size_t nData)
		{
			const LRESULT ret = ::SendMessage(m_hWnd, CB_SETITEMDATA, index, LPARAM(nData));
			return ret != CB_ERR;
		}

		size_t size()const
		{
			return ::SendMessage(m_hWnd, CB_GETCOUNT, 0, 0);
		}

		size_t append(const void *p)
		{
			// for owner-draw
			LRESULT n = ::SendMessage(m_hWnd, CB_ADDSTRING, 0, LPARAM(p));
//			api_check(n != CB_ERR && n != CB_ERRSPACE, "CB_ADDSTRING");
			return size_t(n);
		}

		size_t append(string::const_pointer s)
		{
			return append((void *)s);
		}

		size_t append(const string & s)
		{
			return append(s.c_str());
		}

		size_t append(string::const_pointer s, size_t nData)
		{
			const size_t n = append(s);
			::SendMessage(m_hWnd, CB_SETITEMDATA, n, LPARAM(nData));
			return n;
		}

		size_t append(const string & s, size_t nData)
		{
			return append(s.c_str(), nData);
		}

		size_t insert(size_t n, string::const_pointer s)
		{
			n = ::SendMessage(m_hWnd, CB_INSERTSTRING, n, LPARAM(s));
//			api_check(n != CB_ERR, "CB_INSERTSTRING");
			return n;
		}

		size_t insert(size_t n, const string & s)
		{
			return insert(n, s.c_str());
		}

		size_t insert(size_t n, string::const_pointer s, size_t nData)
		{
			n = insert(n, s);
			::SendMessage(m_hWnd, CB_SETITEMDATA, n, LPARAM(nData));
			return n;
		}

		size_t insert(size_t n, const string & s, size_t nData)
		{
			return insert(n, s.c_str(), nData);
		}

		void erase(size_t n)
		{
			assert(n < size());
			::SendMessage(m_hWnd, CB_DELETESTRING, WPARAM(n), 0);
		}

		void clear()
		{
			::SendMessage(m_hWnd, CB_RESETCONTENT, 0, 0);
		}
	};
}

#define BN_ON_CLICKED(ctrlid, f) WABC_ON_COMMAND(ctrlid, BN_CLICKED, f)
#define EN_ON_CHANGE(ctrlid, f) WABC_ON_COMMAND(ctrlid, EN_CHANGE, f)
#define EN_ON_UPDATE(ctrlid, f) WABC_ON_COMMAND(ctrlid, EN_UPDATE, f)
#define CBN_ON_SELCHANGE(ctrlid, f) WABC_ON_COMMAND(ctrlid, CBN_SELCHANGE, f)
#define CBN_ON_DROPDOWN(ctrlid, f) WABC_ON_COMMAND(ctrlid, CBN_DROPDOWN, f)
