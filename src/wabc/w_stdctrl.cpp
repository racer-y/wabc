#include "w_stdctrl.h"
#include "w_dc.h"

namespace wabc
{
	// --------------------------------------------------------------------
	// radiobox
	// --------------------------------------------------------------------

	void radiobox::before_create(CREATESTRUCT &cs)
	{
		inherited::before_create(cs);
		cs.style |= BS_RADIOBUTTON | WS_CHILD;
	}

	// --------------------------------------------------------------------
	// checkbox
	// --------------------------------------------------------------------

	void checkbox::before_create(CREATESTRUCT &cs)
	{
		inherited::before_create(cs);
		cs.style |= BS_CHECKBOX | WS_CHILD;
	}

	// --------------------------------------------------------------------
	// editbox::selected_t
	// --------------------------------------------------------------------

	void editbox::selected_t::get(size_t &nStart, size_t &nEnd)const
	{
		__wabc_static_assert(sizeof(size_t) == sizeof(DWORD));

		::SendMessage(_this->m_hWnd, EM_GETSEL, WPARAM(&nStart), LPARAM(&nEnd));
	}

	// --------------------------------------------------------------------
	// href_text
	// --------------------------------------------------------------------

	href_text::href_text()
	{
		WABC_BEGIN_MSG_MAP(self)
			WABC_ON_CREATE(&self::on_create)
			WABC_ON_SETFOCUS(&self::on_set_or_kill_focus)
			WABC_ON_KILLFOCUS(&self::on_set_or_kill_focus)
			WABC_ON_SETTEXT(&self::on_set_text)
			WABC_ON_GETTEXT(&self::on_get_text)
			WABC_ON_GETTEXTLENGTH(&self::on_get_text_length)
			WABC_ON_PAINT32(&self::on_paint)
			WABC_NOT_ERASEBKGND()
			WABC_ON_SETCURSOR(&self::on_set_cursor)
			WABC_ON_SETFONT(&self::on_set_font)
			WABC_ON_GETFONT(&self::on_get_font)

			WABC_ON_KEYDOWN(&self::on_key_down)

			WABC_ON_MOUSEMOVE(&self::on_mouse_move)
			WABC_ON_LBUTTONDOWN(&self::on_lbutton_down)
			WABC_ON_LBUTTONUP(&self::on_lbutton_up)

			WABC_ON_MOUSELEAVE(&self::on_mouse_leave)
		WABC_END_MSG_MAP()

		m_hFont = 0;
		m_flag = 0;
	}

	// --------------------------------------------------------------------

	bool href_text::on_create(msg_create &msg)
	{
		if (msg.lpcs->lpszName)
			m_text = msg.lpcs->lpszName;
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_set_text(msg_settext&msg)
	{
		m_text.assign(msg.text);
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_get_text(msg_gettext&msg)
	{
		if (msg.text_size > 0)
		{
			const size_t n = size_t(msg.text_size - 1) > m_text.size()
				? m_text.size() : size_t(msg.text_size - 1);
			::memcpy(msg.text, m_text.c_str(), (n + 1) * sizeof(m_text[0]));
			msg.result = n;
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_get_text_length(msg_gettextlength &msg)
	{
		msg.result = LRESULT(m_text.size());
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_set_cursor(msg_setcursor &)
	{
		HCURSOR hCursor = ::LoadCursor(0, IDC_HAND);
		::SetCursor(hCursor);
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_lbutton_down(msg_mouse &msg)
	{
		m_flag |= flag_lbutton_down;
		::SetCapture(m_hWnd);
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_lbutton_up(msg_mouse &msg)
	{
		m_flag &= ~flag_lbutton_down;
		::ReleaseCapture();

		const rect rt = client_rect();
		if (rt.intersect(msg.x, msg.y))
		{
			clicked();
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_mouse_move(msg_mouse &msg)
	{
		if ((m_flag & flag_track_leave) == 0)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = m_hWnd;
			tme.dwFlags = TME_LEAVE;
			if (::_TrackMouseEvent(&tme) != 0)
				m_flag |= flag_track_leave;
			invalidate();
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_mouse_leave(msg_struct &msg)
	{
		m_flag &= ~flag_track_leave;
		invalidate();
		return true;
	}

	bool href_text::on_key_down(msg_keydown &msg)
	{
		if (msg.key_code == VK_SPACE)
			clicked();
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_set_font(msg_setfont &msg)
	{
		m_hFont = msg.hFont;
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_get_font(msg_getfont &msg)
	{
		msg.result = m_hFont;
		return true;
	}

	// --------------------------------------------------------------------

	bool href_text::on_paint(dcclass &dc, const rect &rtClip, wabc::msg_paint &)
	{
		const DWORD clr= RGB(0, 0, 255);
		dc.text_color = clr;
		dc.bkmode = TRANSPARENT;

		wabc::drawtxt a(dc);
		const rect rtClient = client_rect();

		rect rt(rtClient);

		rt.deflate(3);
		dc.use_font(m_hFont);
		a.put(m_text, rt, a.align_vcenter).draw();

		dc.discard();

		if (m_flag & flag_track_leave)
		{
			wabc::pen pe(clr);
			dc.use_pen(pe);

			const rect &rt1 = a.bound();

			dc.line(rt1.left, rt1.bottom, rt1.right, rt1.bottom);

			dc.discard();
		}

		HWND hFocus = ::GetFocus();
		if (hFocus == m_hWnd)
		{
			rt = rtClient;
			rt.deflate(1);
			DrawFocusRect(dc, &rt);
		}

		return true;
	}

	// --------------------------------------------------------------------

	void href_text::auto_resize()
	{
		cdc dc(m_hWnd);
		dc.use_font(m_hFont);

		wabc::drawtxt a(dc);
		RECT rt = { 0, 0, 1920, 30 };
		a.put(m_text, rt);

		const LONG h = a.bound().height();
		const LONG w = a.bound().width();
		resize(w + 3*2, h + 1 + 2*3);
	}

	// --------------------------------------------------------------------

	bool href_text::on_set_or_kill_focus(msg_setfocus &)
	{
		this->invalidate();
		return true;
	}

	// --------------------------------------------------------------------

	void href_text::clicked()
	{
		// FIXME - ?
		HWND hParent = ::GetParent(m_hWnd);
		const size_t nId = ::GetWindowLongPtr(m_hWnd, GWLP_ID);
		::SendMessage(hParent, WM_COMMAND, MAKEWPARAM(nId, BN_CLICKED), LPARAM(m_hWnd));
	}
}
