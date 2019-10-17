#pragma once

#include "w_msg.h"

#pragma comment(lib,"Comctl32.lib")

namespace wabc
{
	// --------------------------------------------------------------------

	class image_list
	{
		image_list(const image_list &);
		void operator=(const image_list &);
	public:
		HIMAGELIST	m_hImageList;

		image_list() :m_hImageList(0){}
		explicit image_list(HIMAGELIST hImageList) : m_hImageList(hImageList){}
		~image_list()
		{
			if (m_hImageList)
				ImageList_Destroy(m_hImageList);
		}

		void swap(image_list &rhs)
		{
			std::swap(m_hImageList, rhs.m_hImageList);
		}

		operator HIMAGELIST()const { return m_hImageList; }

		bool operator !()const { return m_hImageList == 0; }


		bool create(int nWidth, int nHeight,DWORD dwFlags = ILC_COLOR);

		void destroy();

		bool load_from_resource(size_t resc, int width, DWORD crMask = ILC_COLOR);

		size_t add(HBITMAP hbmImage)
		{
			return ::ImageList_Add(m_hImageList, hbmImage, 0);
		}

		size_t add(HBITMAP hbmImage, HBITMAP hbmMask)
		{
			return ::ImageList_Add(m_hImageList, hbmImage, hbmMask);
		}

		size_t add(HBITMAP hbmImage, COLORREF crMask)
		{
			return ImageList_AddMasked(m_hImageList, hbmImage, crMask);
		}

		void resize(size_t newsize)
		{
			::ImageList_SetImageCount(m_hImageList, (UINT)newsize);
		}

		bool drag_begin(int iTrack, LONG x, LONG y)
		{
			const BOOL b = ImageList_BeginDrag(m_hImageList, iTrack, x, y);
			return b != FALSE;
		}

		void drag_end()
		{
			ImageList_EndDrag();
		}

		bool drag_enter(HWND hwndLock, LONG x, LONG y)
		{
			const BOOL b= ImageList_DragEnter(hwndLock, x, y);
			return b != FALSE;
		}

		bool drag_leave(HWND hwndLock)
		{
			const BOOL b = ImageList_DragLeave(hwndLock);
			return b != FALSE;
		}

		bool drag_move(LONG x, LONG y)
		{
			const BOOL b = ImageList_DragMove(x, y);
			return b != FALSE;
		}

		bool drag_show(BOOL bShow = TRUE)
		{
			const BOOL b= ImageList_DragShowNolock(BOOL(bShow));
			return b != FALSE;
		}

		bool drag_hide(){ return drag_show(FALSE); }
	};

	// --------------------------------------------------------------------

	class treeview : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return WC_TREEVIEW; }
			string::const_pointer new_name()const { return _T("wabctree_view"); }
		};
	protected:
		typedef union_attr<treeview> union_property;

		// --------------------------------------------------------------------

		struct imagelist_normal_t : union_property
		{
			operator HIMAGELIST()
			{
				return (HIMAGELIST)::SendMessage(_this->m_hWnd, TVM_GETIMAGELIST, TVSIL_NORMAL, 0);
			}
			HIMAGELIST operator()(){ return *this; }

			void operator=(HIMAGELIST hImageList)
			{
				HIMAGELIST h = (HIMAGELIST)::SendMessage(_this->m_hWnd, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)hImageList);
			}
		};

		// --------------------------------------------------------------------

		struct imagelist_state_t : union_property
		{
			operator HIMAGELIST()
			{
				return (HIMAGELIST)::SendMessage(_this->m_hWnd, TVM_GETIMAGELIST, TVSIL_STATE, 0);
			}
			HIMAGELIST operator()(){ return *this; }

			void operator=(HIMAGELIST hImageList)
			{
				HIMAGELIST h = (HIMAGELIST)::SendMessage(_this->m_hWnd, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)hImageList);
			}
		};

		// --------------------------------------------------------------------

		struct	bkcolor_t : union_property
		{
			void operator=(COLORREF clr)
			{
				const COLORREF ret = (COLORREF)::SendMessage(_this->m_hWnd, TVM_GETBKCOLOR, 0, 0);
			}

			COLORREF operator()()const
			{
				const COLORREF ret = (COLORREF)::SendMessage(_this->m_hWnd, TVM_GETBKCOLOR, 0, 0);
				return ret;
			}

			COLORREF set(COLORREF clr)
			{
				const COLORREF ret = (COLORREF)::SendMessage(_this->m_hWnd, TVM_SETBKCOLOR, 0, LPARAM(clr));
				return ret;
			}

			COLORREF get()const
			{
				const COLORREF ret = (COLORREF)::SendMessage(_this->m_hWnd, TVM_GETBKCOLOR, 0, 0);
				return ret;
			}
		};

	public:

		union
		{
			imagelist_normal_t normal_image;
			imagelist_state_t	state_image;
			bkcolor_t	bkcolor;
		};
		
		typedef scwnd inherited;
		typedef treeview self;

		treeview() :inherited(superclass()){ bkcolor._this = this; }
		virtual ~treeview(){}

	public:

		HTREEITEM root()const
		{
			return TreeView_GetRoot(m_hWnd);
		}

		HTREEITEM selection()const
		{
			return TreeView_GetSelection(m_hWnd);
		}

		void select(HTREEITEM hItem)
		{
			TreeView_SelectItem(m_hWnd, hItem);
		}

		void drop_hilite(HTREEITEM hItem)
		{
			TreeView_SelectDropTarget(m_hWnd, hItem);
		}
		HTREEITEM drop_hilite()const
		{
			return TreeView_GetDropHilight(m_hWnd);
		}

		void expand(HTREEITEM ti)
		{
			BOOL b = TreeView_Expand(m_hWnd, ti, TVE_EXPAND);
		}

		void collapse(HTREEITEM ti)
		{
			BOOL b = TreeView_Expand(m_hWnd, ti, TVE_COLLAPSE);
		}

		void toggle(HTREEITEM ti)
		{
			BOOL b = TreeView_Expand(m_hWnd, ti, TVE_TOGGLE);
		}

		HWND edit(HTREEITEM ti)
		{
			LRESULT b = ::SendMessage(m_hWnd, TVM_EDITLABEL, 0, LPARAM(ti));
			return reinterpret_cast<HWND>(b);
		}

		HTREEITEM parent(HTREEITEM ti)const
		{
			return TreeView_GetParent(m_hWnd, ti);
		}

		HTREEITEM first_child(HTREEITEM ti)const
		{
			return TreeView_GetChild(m_hWnd, ti);
		}

		HTREEITEM next_sibling(HTREEITEM ti)const
		{
			return TreeView_GetNextSibling(m_hWnd, ti);
		}

		HTREEITEM previous_sibling(HTREEITEM ti)const
		{
			return TreeView_GetPrevSibling(m_hWnd, ti);
		}

		void set_item(TVITEMEX &tvi)
		{
			BOOL b = TreeView_SetItem(m_hWnd, &tvi);
			assert(b != FALSE);
		}

		bool get_item(TVITEMEX &tvi)
		{
			const BOOL b = TreeView_GetItem(m_hWnd, &tvi);
			return(b != FALSE);
		}

		size_t get_item_data(HTREEITEM ti)const;
		void set_item_data(HTREEITEM ti, size_t nData);

		bool set_item_children(HTREEITEM ti, int cChild);

		bool get_item_text(HTREEITEM ti, wchar_t *text, size_t n)const;
		template<size_t N>
		bool get_item_text(HTREEITEM ti, wchar_t(&text)[N])const
		{
			return get_item_text(ti, text, N);
		}

		bool get_item_text(HTREEITEM ti, string &s)const;
		bool set_item_text(HTREEITEM ti, const wchar_t *text, size_t n);
		bool set_item_text(HTREEITEM ti, const wchar_t *text)
		{
			return set_item_text(ti, text, ::wcslen(text));
		}

		bool set_item_text(HTREEITEM ti, const string &s)
		{
			return set_item_text(ti, s.c_str(), s.size());
		}

		HTREEITEM add_last_child(HTREEITEM hParent, TVINSERTSTRUCT &tss)
		{
			tss.hInsertAfter = TVI_LAST;
			tss.hParent = hParent;

			HTREEITEM hItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM,
				0, (LPARAM)&tss);
			return hItem;
		}

		HTREEITEM add_first_child(HTREEITEM hParent, TVINSERTSTRUCT &tss)
		{
			tss.hInsertAfter = TVI_FIRST;
			tss.hParent = hParent;

			HTREEITEM hItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM,
				0, (LPARAM)&tss);
			return hItem;
		}

		HTREEITEM add_next_sibling(HTREEITEM hItem, TVINSERTSTRUCT &tss)
		{
			tss.hInsertAfter = hItem;
			tss.hParent = TreeView_GetParent(m_hWnd, hItem);

			hItem = (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEM,
				0, (LPARAM)&tss);
			return hItem;
		}

		bool erase_item(HTREEITEM hItem)
		{
			const BOOL b= TreeView_DeleteItem(m_hWnd, hItem);
			return b != FALSE;
		}

		HTREEITEM hit_test(TVHITTESTINFO &tvht)
		{
			return (HTREEITEM)SendMessage(m_hWnd, TVM_HITTEST, NULL, (LPARAM)&tvht);
		}

		HIMAGELIST create_drag_image(HTREEITEM hItem)
		{
			return TreeView_CreateDragImage(m_hWnd, hItem);
		}
	};

	// --------------------------------------------------------------------

	struct msg_tvn_selchanged : stdmsg_template<msg_tvn_selchanged>
	{
		HWND	hWnd;
		UINT	message;

		WPARAM 	ctrlid;
		LPNMTREEVIEW	pnmhtv;

		LRESULT result;
	};

	struct map_tvn_selchanged : msgmap_t
	{
		typedef msg_tvn_selchanged msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_type) == sizeof(msg_struct));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	typedef msg_tvn_selchanged msg_tvn_selchanging;

	typedef msg_tvn_selchanged msg_tvn_itemexpanded;
	typedef msg_tvn_selchanged msg_tvn_itemexpanding;

	typedef msg_tvn_selchanged msg_tvn_begindrag;

	// --------------------------------------------------------------------

	struct msg_tvn_beginedit : stdmsg_template<msg_tvn_beginedit>
	{
		HWND	hWnd;
		UINT	message;

		WPARAM 	ctrlid;
		LPNMTVDISPINFO	pnmhtv;

		LRESULT result;
	};

	struct map_tvn_edit_begin : msgmap_t
	{
		typedef msg_tvn_beginedit msg_type;

		template<typename T>
		static inline size_t map_fun_addr(bool (T::*f)(msg_type &))
		{
			__wabc_static_assert(sizeof(msg_type) == sizeof(msg_struct));
			return *reinterpret_cast<size_t *>(&f);
		}
	};

	typedef msg_tvn_beginedit msg_tvn_endedit;
	typedef map_tvn_edit_begin map_tvn_end_edit;

	// --------------------------------------------------------------------

	class tabctrl : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return WC_TABCONTROL; }
			string::const_pointer new_name()const { return _T("wabctabctrl"); }
		};

		typedef union_attr<tabctrl> union_property;

		// --------------------------------------------------------------------

		struct selection_t : union_property
		{
			operator size_t()const
			{
				const size_t n = TabCtrl_GetCurSel(_this->m_hWnd);
				return n;
			}

			size_t operator()()const { return *this; }

			void operator=(size_t n)
			{
				const BOOL result = TabCtrl_SetCurSel(_this->m_hWnd, n);
				assert(result != - 1);
			}
		};

		// --------------------------------------------------------------------

		struct display_rect_t : union_property
		{
			bool operator()(RECT &rt)const
			{
				if (::GetClientRect(_this->m_hWnd, &rt))
				{
					TabCtrl_AdjustRect(_this->m_hWnd, false, &rt);
					return true;
				}
				return false;
			}

			operator rect()const
			{
				RECT rt;
				const bool b= operator()(rt);
				assert(b);
				return static_cast<rect &>(rt);
			}

			rect operator()()const { return *this; }

			LONG width()const { return operator()().width(); }
			LONG height()const { return operator()().height(); }
		};

	public:
		union
		{
			display_rect_t	display_rect;
			selection_t		selection;
		};


		enum{
			tabs_style = TCS_TABS, buttons_style = TCS_BUTTONS,
			flat_buttons_style = TCS_FLATBUTTONS | TCS_BUTTONS
		};

		enum{
			top_tab = 0, bottom_tab = TCS_BOTTOM, left_tab = TCS_VERTICAL,
			right_tab = TCS_VERTICAL | TCS_RIGHT
		};

		typedef tabctrl self;
		typedef scwnd inherited;

		tabctrl() :inherited(superclass()){ selection._this = this; }
		virtual ~tabctrl(){}

		size_t tab_count()const
		{
			const size_t n = TabCtrl_GetItemCount(m_hWnd);
			return n;
		}

		void remove_tab(size_t nTabIndex)
		{
			const bool result = TabCtrl_DeleteItem(m_hWnd, nTabIndex) != 0;
		}

		size_t add_tab(const TCITEM &item)
		{
			const size_t n = tab_count();
			const size_t result = TabCtrl_InsertItem(m_hWnd, n, &item);
			return result;
		}

		size_t add_tab(string::const_pointer lpszText, size_t n, int nImageIndex = -1);
		size_t add_tab(string::const_pointer lpszText, int nImageIndex = -1)
		{
			return add_tab(lpszText, ::wcslen(lpszText), nImageIndex);
		}

		size_t add_tab(const string &s, int nImageIndex = -1)
		{
			return add_tab(s.c_str(), s.size(), nImageIndex);
		}

	};

	// --------------------------------------------------------------------

	class listview : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return WC_LISTVIEW; }
			string::const_pointer new_name()const { return _T("wabclist_view"); }
		};
	public:
		typedef tabctrl self;
		typedef scwnd inherited;

		listview() :inherited(superclass()){}
		virtual ~listview(){}
	};

	// --------------------------------------------------------------------

	class ipctrl : public scwnd
	{
		typedef union_attr<ipctrl> union_property;

		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return WC_IPADDRESS; }
			string::const_pointer new_name()const { return _T("wabcIPADDRESS"); }
		};

		// --------------------------------------------------------------------

		struct address_t : union_property
		{
			void operator=(const DWORD addr)
			{
				::SendMessage(_this->m_hWnd, IPM_SETADDRESS, 0, LPARAM(addr));
			}

			DWORD operator()()const
			{
				DWORD addr = 0;
				const LRESULT n = ::SendMessage(_this->m_hWnd, IPM_GETADDRESS, 0, LPARAM(&addr));
				return addr;
			}
		};

	public:

		union
		{
			address_t address;
		};
		
		typedef ipctrl self;
		typedef scwnd inherited;

		// 一开始要设好这个控件的高度和宽度，不然会有显示的麻烦，这个控件没有处理WM_SIZE消息
		ipctrl() :inherited(superclass()){ address._this = this; }
		virtual ~ipctrl(){}
	};

	// --------------------------------------------------------------------

	class progress_bar : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return PROGRESS_CLASS; }
			string::const_pointer new_name()const { return _T("wabcprogress_bar"); }
		};

		typedef union_attr< progress_bar > union_property;

		// --------------------------------------------------------------------
		
		struct range_t : union_property
		{
			LONG operator()()
			{
				PBRANGE rg;
				HWND hWnd = _this->m_hWnd;
				::SendMessage(hWnd, PBM_GETRANGE, (WPARAM)FALSE, (LPARAM)&rg);
				return rg.iHigh - rg.iLow;
			}

			void set(LONG nMin, LONG nMax)
			{
				HWND hWnd = _this->m_hWnd;
				::SendMessage(hWnd, PBM_SETRANGE32, (WPARAM)nMin, (LPARAM)nMax);
			}

			LONG low()const
			{
				PBRANGE rg;
				HWND hWnd = _this->m_hWnd;
				::SendMessage(hWnd, PBM_GETRANGE, (WPARAM)FALSE, (LPARAM)&rg);
				return rg.iLow;
			}

			LONG high()const
			{
				PBRANGE rg;
				HWND hWnd = _this->m_hWnd;
				::SendMessage(hWnd, PBM_GETRANGE, (WPARAM)TRUE, (LPARAM)&rg);
				return rg.iHigh;
			}
		};

		// --------------------------------------------------------------------

		struct pos_t : union_property
		{
			operator LONG()const
			{
				HWND hWnd = _this->m_hWnd;
				const LONG n = (LONG)::SendMessage(hWnd, PBM_GETPOS, 0, 0);
				return n;
			}

			LONG operator()()const { return *this; }

			LONG operator=(LONG n)
			{
				HWND hWnd = _this->m_hWnd;
				::SendMessage(hWnd, PBM_SETPOS, (WPARAM)n, 0);
				return n;
			}

			LONG operator++()
			{
				HWND hWnd = _this->m_hWnd;
				LONG n = (LONG)::SendMessage(hWnd, PBM_DELTAPOS, (WPARAM)1, 0);
				return ++n;
			}

			LONG operator+=(LONG n)
			{
				HWND hWnd = _this->m_hWnd;
				n += (LONG)::SendMessage(hWnd, PBM_DELTAPOS, (WPARAM)n, 0);
				return n;
			}
		};

		// --------------------------------------------------------------------

		struct step_t : union_property
		{
			LONG operator=(LONG n)
			{
				HWND hWnd = _this->m_hWnd;
				::SendMessage(hWnd, PBM_SETSTEP, (WPARAM)n, 0);
				return n;
			}

			LONG operator++(int)
			{
				HWND hWnd = _this->m_hWnd;
				LONG n = (LONG)::SendMessage(hWnd, PBM_STEPIT, 0, 0);
				return n;
			}
		};

	public:
		union
		{
			range_t	range; 
			pos_t	pos; 
			step_t	step;
		};
		
		typedef progress_bar self;
		typedef scwnd inherited;

		progress_bar() :inherited(superclass()){ range._this = this; }
		virtual ~progress_bar(){}
	};

	// --------------------------------------------------------------------

	class tooltips : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return TOOLTIPS_CLASS; }
			string::const_pointer new_name()const { return _T("wabctooltips"); }
		};

		typedef union_attr< progress_bar > union_property;
	public:
		typedef tooltips self;
		typedef scwnd inherited;

		tooltips() :inherited(superclass()){}
		virtual ~tooltips(){}

		virtual void before_create(CREATESTRUCT &cs);

		bool add(HWND hWnd, string::const_pointer lpszTips);
	};

	// --------------------------------------------------------------------

	class dptctrl : public scwnd
	{
		// --------------------------------------------------------------------

		struct superclass
		{
			string::const_pointer old_name()const{ return DATETIMEPICK_CLASS; }
			string::const_pointer new_name()const { return _T("wabcDateTime"); }
		};

		// --------------------------------------------------------------------

		typedef union_attr< dptctrl > union_property;

		struct as_c_time_t : union_property
		{
			bool get(time_t &t)const;

			time_t operator()()const
			{
				time_t t;
				return get(t) ? t : -1;
			}

			operator time_t()const{return *this;}

			void operator=(const time_t &t);
		};

	public:
		union
		{
			as_c_time_t	as_c_time;
		};
		
		typedef dptctrl self;
		typedef scwnd inherited;

		dptctrl() :inherited(superclass()) { as_c_time._this = this; }
		virtual ~dptctrl(){}		
	};
}

#define TREEVIEW_ON_ENDEDIT(ctrlid, f) \
{ TVN_ENDLABELEDIT, WM_NOTIFY, ctrlid, wabc::map_tvn_end_edit::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TREEVIEW_ON_BEGINEDIT(ctrlid, f) \
{ TVN_BEGINLABELEDIT, WM_NOTIFY, ctrlid, wabc::map_tvn_edit_begin::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TREEVIEW_ON_BEGINDRAG(ctrlid, f) \
{ TVN_BEGINDRAG, WM_NOTIFY, ctrlid, wabc::map_tvn_selchanged::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TREEVIEW_ON_ITEMEXPANDING(ctrlid, f) \
{ TVN_ITEMEXPANDING, WM_NOTIFY, ctrlid, wabc::map_tvn_selchanged::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TREEVIEW_ON_ITEMEXPANDED(ctrlid, f) \
{ TVN_ITEMEXPANDED, WM_NOTIFY, ctrlid, wabc::map_tvn_selchanged::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TREEVIEW_ON_SELCHANGING(ctrlid, f) \
{ TVN_SELCHANGING, WM_NOTIFY, ctrlid, wabc::map_tvn_selchanged::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TREEVIEW_ON_SELCHANGED(ctrlid, f) \
{ TVN_SELCHANGED, WM_NOTIFY, ctrlid, wabc::map_tvn_selchanged::map_fun_addr<map_class>(f), &wabc::map_create::on_message },

#define TABCTRL_ON_SELCHANGING(ctrlid, f) WABC_ON_NOTIFY(ctrlid, TCN_SELCHANGING, f)
#define TABCTRL_ON_SELCHANGE(ctrlid, f) WABC_ON_NOTIFY(ctrlid, TCN_SELCHANGE, f)

#define WABC_ON_RDBCLICK(ctrlid, f) WABC_ON_NOTIFY(ctrlid, NM_RDBLCLK, f)
#define WABC_ON_RCLICK(ctrlid, f) WABC_ON_NOTIFY(ctrlid, NM_RCLICK, f)
#define WABC_ON_DBCLICK(ctrlid, f) WABC_ON_NOTIFY(ctrlid, NM_DBLCLK, f)
