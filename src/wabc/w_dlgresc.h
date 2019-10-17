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

#include "w_std.h"
#include <vector>

namespace wabc
{
#pragma pack(push, 1)

		typedef struct
		{
			WORD dlgVer;
			WORD signature;
			DWORD helpID;
			DWORD exStyle;
			DWORD style;
			WORD cDlgItems;
			short x;
			short y;
			short cx;
			short cy;
		} DLGTEMPLATEEX;

#pragma pack(pop)
	// --------------------------------------------------------------------

	class dlgitem_template
	{
	public:
#pragma pack(push, 1)
		typedef struct
		{
			DWORD helpID;
			DWORD exStyle;
			DWORD style;
			short x;
			short y;
			short cx;
			short cy;
			DWORD id;
		} DLGITEMTEMPLATEEX;
#pragma pack(pop)

	public:

		dlgitem_template(const char * dlgtmpl, const char * itemtmpl);

		bool first();
		bool next();

		rect bound()const
		{
			if(m_is_standare_template)
				return rect(m_cur_std->x, 
				m_cur_std->y, 
				m_cur_std->x + m_cur_std->cx,
				m_cur_std->y + m_cur_std->cy);

			return rect(m_cur->x, m_cur->y, m_cur->x + m_cur->cx, m_cur->y + m_cur->cy);
		}

		DWORD style()const { return m_is_standare_template ? m_cur_std->style : m_cur->style; }
		DWORD exstyle()const { return m_is_standare_template ? m_cur_std->dwExtendedStyle : m_cur->exStyle; }
		DWORD id()const { return m_is_standare_template ? m_cur_std->id : m_cur->id; }

		wchar_t * title(){ return m_title; }
		wchar_t * window_class(){ return m_wnd_class; }
		void *extract(){ return m_extra; }

	private:

		bool	m_is_standare_template;
		size_t	m_item_count;
		size_t	m_cur_item;

		union
		{
			DLGITEMTEMPLATEEX	*m_template;
			DLGTEMPLATE			*m_template_std;
		};

		union
		{
			DLGITEMTEMPLATEEX	*m_cur;
			DLGITEMTEMPLATE		*m_cur_std;
		};

		DLGITEMTEMPLATEEX	*m_next;

		wchar_t	*m_wnd_class;
		wchar_t *m_title;
		void	*m_extra;

		void read();
	};
}