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

#include "w_msg.h"
#include <vector>

namespace wabc
{
	bool is_my_child(HWND hOwner, HWND hChild);

	// --------------------------------------------------------------------

	class dialog;
	struct dlgctrl_property
	{
//		enum{ anchor_left = 1, anchor_right = 2, anchor_top = 4, anchor_bottom = 8 };

		size_t	resource_id;

		dlgctrl_property(size_t offset_of_dialog, size_t resc_id);

		wndbase * control()
		{
//			__wabc_static_assert((sizeof(dlgctrl_property) % 8) == 0);
			uint8 *p = (uint8*)this;
			p += sizeof(*this);
			return reinterpret_cast<wndbase *>(p);
		}

		const wndbase * control()const
		{
			const uint8 *p = (uint8*)this;
			p += sizeof(*this);
			return reinterpret_cast<const wndbase *>(p);
		}
	};

	// --------------------------------------------------------------------

#define DLGCTRL(dialog_type, resc_id, class_type)	\
	struct _struct_##resc_id : public wabc::dlgctrl_property{ \
	_struct_##resc_id() : dlgctrl_property(offsetof(dialog_type, __##resc_id), resc_id){}	\
	} __##resc_id;	class_type

	// --------------------------------------------------------------------
	
	class dialog : public wndbase
	{
		WABC_DECLARE_MSG_MAP()

		WABC_DECLARE_HOOK_GETMSG()

		friend class wndproc;
		friend dlgctrl_property;

		bool process_tab_keydown(HWND hCtrl, bool bShift);
		void tab_next();
				
		// --------------------------------------------------------------------

		typedef union_attr<dialog> union_property;

		struct items_type : union_property
		{
			_basewnd operator[](size_t res_id)const
			{
				_basewnd a;
				a.m_hWnd = ::GetDlgItem(_this->m_hWnd, res_id);
				return a;
			}
		};

		struct tab_items_type : union_property
		{
			void add(HWND hWnd)
			{
				_this->m_tab_ctrls.push_back(hWnd);

			}
		};

	public:
		struct create_param
		{
			HINSTANCE hInstance;
			char *dlgtmpl, *itemtmpl;
			dialog *dlg;
		};

		typedef dialog self;
		typedef wndbase inherited;

		union
		{
			items_type items;
			tab_items_type	tab_items;
		};

		dialog();
		explicit dialog(size_t nResourceId);
		virtual ~dialog(){ dialog_destroy(); }

		HWND get_item(int nDlgItem)
		{
			HWND hWnd = ::GetDlgItem(m_hWnd, nDlgItem);
			return hWnd;
		}
		
		bool on_size(msg_size &);
		bool on_sc_close(msg_syscommand &);
		bool on_msg_intercept(msg_hook &);

		bool on_tab_next(msg_struct &);
		bool on_tab_prior(msg_struct &);
	public:

		// 非模式对话框
		bool create(HINSTANCE hInstance, size_t resid, HWND hParent = 0);
		bool create(size_t resid, HWND hParent)
		{
			return create(*g_app, resid, hParent);
		}
		bool create(size_t resid)
		{
			return create(*g_app, resid, 0);
		}

		// 模式对话框
		int show_modal(HINSTANCE hInstance, size_t resid, HWND hParent = 0);
		int show_modal(size_t resid, HWND hWnd)
		{
			return show_modal(*g_app, resid, hWnd);
		}
		int show_modal(size_t resid)
		{
			return show_modal(*g_app, resid, 0);
		}
		void end_dialog(int nModalResult)
		{
			bool result = ::EndDialog(m_hWnd, nModalResult) != 0;
		}

		//dlgctr_t & attr(wndbase &wnd)
		//{
		//	uint8 *p = (uint8*)&wnd;
		//	p -= sizeof(dlgctr_t);
		//	return reinterpret_cast<dlgctr_t &>(*p);
		//}

		//const dlgctr_t & attr(wndbase &wnd)const
		//{
		//	const uint8 *p = (const uint8*)&wnd;
		//	p -= sizeof(dlgctr_t);
		//	return reinterpret_cast<const dlgctr_t &>(*p);
		//}

	private:
		std::vector<HWND> m_tab_ctrls;
		std::vector< dlgctrl_property * >	m_dlgctrls;

		void dialog_create(const create_param &param);
		void dialog_destroy(){}
	};
}