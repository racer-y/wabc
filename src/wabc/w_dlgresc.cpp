#include "w_dlgresc.h"

namespace wabc
{
	// --------------------------------------------------------------------
	// struct dlgitem_template
	// --------------------------------------------------------------------

	dlgitem_template::dlgitem_template(const char * dlgtmpl, const char * itemtmpl)
	{
		m_template = (DLGITEMTEMPLATEEX *)(itemtmpl);

		DLGTEMPLATEEX *p = (DLGTEMPLATEEX*)dlgtmpl;
		if(p->signature == 0xFFFF)
		{
			m_item_count = p->cDlgItems;
			m_is_standare_template= false;
		}
		else
		{
			m_item_count = ((DLGTEMPLATE*)p)->cdit;
			m_is_standare_template= true;
		}

	}

	// --------------------------------------------------------------------

	bool dlgitem_template::first()
	{
		m_cur_item = 0;
		m_next = m_template;
		return next();
	}

	// --------------------------------------------------------------------

	bool dlgitem_template::next()
	{
		if (m_cur_item < m_item_count)
		{
			m_cur = m_next;

			read();

			++m_cur_item;
			return true;
		}
		return false;
	}

	// --------------------------------------------------------------------

	void dlgitem_template::read()
	{
		if(m_is_standare_template)
		{
			m_wnd_class = (wchar_t*)(m_cur_std + 1);
		}
		else
		{
			m_wnd_class = (wchar_t*)(m_cur + 1);
		}

		wchar_t *pwchar = m_wnd_class;
		if (*pwchar == 0xFFFF)
			++pwchar;
		else
			for (; *pwchar != 0; ++pwchar);

		++pwchar;
		m_title = pwchar;
		assert(*m_title != 0xFFFF);
		if(*pwchar == 0xFFFF)
			++pwchar;
		else
			for (; *pwchar != 0; ++pwchar);

		++pwchar;
		if (*pwchar == 0)
		{
			++pwchar;
			m_extra = 0;
		}
		else
		{
			const size_t n = *pwchar;
			m_extra = pwchar + 1;
			pwchar += n + 1;
		}

		// DWORD boundary
		size_t n = size_t(pwchar);
		n += 3;
		n >>= 2;
		n <<= 2;

		m_next= reinterpret_cast< DLGITEMTEMPLATEEX *>(n);
	}
}