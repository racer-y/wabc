#include "w_msg.h"
#include "w_proc.h"
#include "w_dc.h"

namespace wabc
{
	// --------------------------------------------------------------------

	bool map_create::on_map(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		msgmap_t &o = *reinterpret_cast<msgmap_t *>(_this);

		typedef bool (msgmap_t::*fun_t)(msg_type &);
		const fun_t &f = *reinterpret_cast<const fun_t *>(&a.on_message);

		//		bool (msgmap_t::*f)(msg_type &);
		//		*reinterpret_cast<size_t *>(&f)= a.on_message;

		(o.*f)(reinterpret_cast<msg_type &>(msg));

		// 这是初始化的消息，映射的函数都需要遍历一次
		return false;
	}

	// --------------------------------------------------------------------

	bool map_size::on_map(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		msgmap_t &o= *reinterpret_cast<msgmap_t *>(_this);

		typedef bool (msgmap_t::*fun_t)(msg_type &);
		const fun_t &f = *reinterpret_cast<const fun_t *>(&a.on_message);

//		bool (msgmap_t::*f)(msg_type &);
//		*reinterpret_cast<size_t *>(&f)= a.on_message;

		const bool b= (o.*f)(reinterpret_cast<msg_type &>(msg));
		return b;
	}

	// --------------------------------------------------------------------

	bool map_erasebkgnd::not_erase_background(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		// 不画背景
		msg.result = TRUE;
		return true;
	}

	// --------------------------------------------------------------------

	bool map_command::reflect(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		if (msg.lParam)
		{
			const LRESULT ret = ::SendMessage(HWND(msg.lParam), WM_WTL, WPARAM(&msg), LPARAM(&wndproc::process_WM_COMMAND));
			if (ret == 1)
				return true;
		}
		return false;
	}

	// --------------------------------------------------------------------

	bool map_notify::reflect(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		NMHDR &nh = *reinterpret_cast<NMHDR *>(msg.lParam);
		const LRESULT ret = ::SendMessage(nh.hwndFrom, WM_WTL, WPARAM(&msg), LPARAM(&wndproc::process_WM_NOTIFY));
		return (ret == 1) ? true : false;
	}

	// --------------------------------------------------------------------
	
	struct paint_sentry : PAINTSTRUCT
	{
		HWND	m_hWnd;
		HDC		hdc;

		paint_sentry(HWND hWnd) : m_hWnd(hWnd)
		{
			hdc = ::BeginPaint(m_hWnd, this);
		}

		~paint_sentry()
		{
			::EndPaint(m_hWnd, this);
		}
	};

	bool map_paint::on_map(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		msgmap_t &o = *reinterpret_cast<msgmap_t *>(_this);

		bool (msgmap_t::*f)(dcclass &, const rect &, msg_type &);
		*reinterpret_cast<size_t *>(&f) = a.on_message;

		if (msg.wParam == 0 && msg.lParam == 0)
		{
			paint_sentry ps(msg.hWnd);
			const rect &rt = static_cast<rect &>(ps.rcPaint);
			const LONG w = rt.width();
			const LONG h = rt.height();

			if (w > 0 && h > 0)
			{
				dcclass dc(false, ps.hdc);

				msg.wParam = reinterpret_cast<WPARAM>(&dc);
				msg.lParam = reinterpret_cast<LPARAM>(&rt);

				(o.*f)(dc, rt, reinterpret_cast<msg_type &>(msg));
			}
		}
		else
		{
			assert(msg.wParam && msg.lParam);
			dcclass &dc = *reinterpret_cast<dcclass *>(msg.wParam);
			const rect &rt = *reinterpret_cast<rect *>(msg.lParam);
			(o.*f)(dc, rt, reinterpret_cast<msg_type &>(msg));
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool map_paint32::on_map(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		msgmap_t &o = *reinterpret_cast<msgmap_t *>(_this);

		bool (msgmap_t::*f)(dcclass &, const rect &, msg_type &);
		*reinterpret_cast<size_t *>(&f) = a.on_message;

		if (msg.wParam == 0 && msg.lParam == 0)
		{
			paint_sentry ps(msg.hWnd);

			const rect &rt = static_cast<rect &>(ps.rcPaint);
			const LONG w = rt.width();
			const LONG h = rt.height();

			if (w > 0 && h > 0)
			{
				bitmap32	bmp(w, h);
				mdc dc(ps.hdc, bmp);
				
				msg.wParam = reinterpret_cast<WPARAM>(&dc);
				msg.lParam = reinterpret_cast<LPARAM>(&rt);

				dc.window_org.set(rt.left, rt.top);
				(o.*f)(dc, rt, reinterpret_cast<msg_type &>(msg));

				msg.wParam = 0;
				msg.lParam = 0;

				::BitBlt(ps.hdc, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top,
					dc, rt.left, rt.top, SRCCOPY);
			}
		}
		else
		{
			assert(msg.wParam && msg.lParam);
			dcclass &dc = *reinterpret_cast<dcclass*>(msg.wParam);
			const rect &rt = *reinterpret_cast<rect*>(msg.lParam);

			if (dc.bmp32)
			{
				(o.*f)(dc, rt, reinterpret_cast<msg_type &>(msg));
			}
			else
			{
				const LONG w = rt.right - rt.left;
				const LONG h = rt.bottom - rt.top;

				if (w > 0 && h > 0)
				{
					bitmap32	bmp(w, h);
					mdc bdc(dc, bmp);

					bdc.window_org.set(rt.left, rt.top);

					msg.wParam = reinterpret_cast<WPARAM>(&bdc);

					(o.*f)(dc, rt, reinterpret_cast<msg_type &>(msg));

					msg.wParam = WPARAM(&dc);

					dc.bitblt(rt, bdc);
				}
			}
		}
		return true;
	}

	// --------------------------------------------------------------------

	bool map_draw::on_map(const msgmap_t &a, void * _this, msg_struct &msg)
	{
		msgmap_t &o = *reinterpret_cast<msgmap_t *>(_this);
		bool (msgmap_t::*f)(dcclass &, const rect &, msg_type &);
		*reinterpret_cast<size_t *>(&f) = a.on_message;

		msg_struct &from = *reinterpret_cast<msg_struct *>(msg.lParam);
		dcclass &dc = *reinterpret_cast<dcclass*>(from.wParam);
		const rect &rt = *reinterpret_cast<rect*>(from.lParam);

		const bool b = (o.*f)(dc, rt, reinterpret_cast<msg_type &>(msg));
		return b;
	}
}