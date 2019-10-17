#pragma once

#include "w_wndbase.h"
#include <assert.h>

#define RGB32(r,g,b) (DWORD(b)|((DWORD(g) << 8))|((DWORD(r) << 16)))

// --------------------------------------------------------------------

struct color32
{
	typedef unsigned char uint8;
	typedef DWORD uint32;

	union
	{
		uint32	m_bgra;
		struct
		{
			uint8 b, g, r, a;
		};
	};

	color32(){}

	color32(DWORD dwBGRA) :m_bgra(dwBGRA){}

	color32(DWORD r1, DWORD g1, DWORD b1, DWORD a1 = 0)
		: r(uint8(r1)), g(uint8(g1)), b(uint8(b1)), a(uint8(a1))
	{}

	bool operator==(const color32 &rhs)const
	{
		return m_bgra == rhs.m_bgra;
	}

	bool operator!=(const color32 &rhs)const
	{
		return m_bgra != rhs.m_bgra;
	}
};

inline DWORD RGBTORGB32(DWORD v)
{
	const unsigned char r = GetRValue(v);
	const unsigned char g = GetGValue(v);
	const unsigned char b = GetBValue(v);
	return RGB32(r,g,b);
}

namespace wabc
{
	// --------------------------------------------------------------------

	template<unsigned char r, unsigned char g, unsigned char b>
	struct make_color
	{
		enum{ value = RGB(r, g, b) };
		enum{ value32 = RGB32(r, g, b) };

		enum{ red = r, green = g, blue = b };
	};

	// --------------------------------------------------------------------

	class pen
	{
		pen(const pen &);
		void operator=(const pen &);
	public:

		HPEN	m_hPen;

		enum{ style_solid = PS_SOLID, style_dash = PS_DASH, style_dot = PS_DOT };

		pen() : m_hPen(0)
		{}

		explicit pen(DWORD clr, int nStyle = style_solid,
			int nWidth = 1) : m_hPen(0)
		{
			create(clr, nStyle, nWidth);
		}

		explicit pen(const LOGPEN &lp)
		{
			create(lp);
		}

		~pen() { destroy(); }

		HPEN create(DWORD clr, int nStyle = style_solid,
			int nWidth = 1);

		HPEN create(const LOGPEN &lp)
		{
			assert(m_hPen == 0);
			return m_hPen = ::CreatePenIndirect(&lp);
		}

		void destroy()
		{
			if (m_hPen != 0)
			{
				::DeleteObject(m_hPen);
				m_hPen = 0;
			}
		}

		operator HPEN()const
		{
			return m_hPen;
		}

		void swap(pen &rhs)
		{
			std::swap(m_hPen, rhs.m_hPen);
		}
	};

	// --------------------------------------------------------------------

	class brush
	{
		brush(const brush &);
		void operator=(const brush &);
	public:

		enum{ style_solid = BS_SOLID };

		HBRUSH	m_hBrush;

		brush() : m_hBrush(0)
		{}

		explicit brush(DWORD clr, int nStyle = style_solid,
			int nHatch = 0) : m_hBrush(0)
		{
			create(clr, nStyle, nHatch);
		}

		explicit brush(const LOGBRUSH &lb) : m_hBrush(0)
		{
			create(lb);
		}

		explicit brush(HBITMAP hBitmap) : m_hBrush(0)
		{
			create(hBitmap);
		}

		~brush()
		{
			destroy();
		}

		HBRUSH create(DWORD clr, int nStyle = style_solid, int nHatch = 0)
		{
			LOGBRUSH lb;
			lb.lbColor = clr;
			lb.lbStyle = nStyle;
			lb.lbHatch = nHatch;

			return create(lb);
		}

		HBRUSH create(const LOGBRUSH &lb)
		{
			assert(m_hBrush == 0);
			return m_hBrush = ::CreateBrushIndirect(&lb);
		}

		HBRUSH create(HBITMAP hBitmap)
		{
			assert(m_hBrush == 0);
			return m_hBrush = ::CreatePatternBrush(hBitmap);
		}

		void destroy()
		{
			if (m_hBrush != 0)
			{
				::DeleteObject(m_hBrush);
				m_hBrush = 0;
			}
		}

		operator HBRUSH()const
		{
			return m_hBrush;
		}
	};

	// --------------------------------------------------------------------

	class bitmap
	{
		bitmap(const bitmap &);
		void operator=(const bitmap &);
	public:
		HBITMAP	m_hBitmap;

		bitmap() :m_hBitmap(0){}

		explicit bitmap(size_t nResourceId) { load(nResourceId); }

		bitmap(size_t nResourceId, HINSTANCE hInstance)
		{
			load(nResourceId, hInstance);
		}

		explicit bitmap(const BITMAP &bm)
		{
			create(bm);
		}

		bitmap(LONG nWidth, LONG nHeight)
		{
			create(nWidth, nHeight);
		}

		bitmap(LONG nWidth, LONG nHeight, const void *lpBits)
		{
			create(nWidth, nHeight, lpBits);
		}

		bitmap(HDC hdc, LONG nWidth, LONG nHeight)
		{
			create(hdc, nWidth, nHeight);
		}

		~bitmap(){ destroy(); }

		bool create(HDC hdc, LONG nWidth, LONG nHeight);

		bool create(LONG nWidth, LONG nHeight, const void *lpBits = 0);

		bool create(const BITMAP &bm);

		bool load(size_t nResourceId, HINSTANCE hInstance);
		bool load(size_t nResourceId)
		{
			return load(nResourceId, *g_app);
		}

		void load_from_file(const string &strFile);
		bool save_to_bmp(const string &strFile);

		bool load_from_resource(size_t nRescId, LPCTSTR lpType)
		{
			return load_from_resource(nRescId, lpType, *g_app);
		}
		bool load_from_resource(size_t nRescId, LPCTSTR lpType, HINSTANCE hInstance);

		void destroy() ;

		LONG width()const 
		{
			BITMAP bm;
			if (::GetObject(m_hBitmap, sizeof(bm), &bm) == 0)
				return 0;
			return bm.bmWidth;
		}

		LONG height()const
		{
			BITMAP bm;
			if (::GetObject(m_hBitmap, sizeof(bm), &bm) == 0)
				return 0;
			return bm.bmHeight;
		}

		size_t size()const;
		bool get_bits(void *bits, size_t uStartScan, size_t cScanLines)const;

		operator HBITMAP()const{ return m_hBitmap; }
	};

	// --------------------------------------------------------------------

	class bitmap32
	{
		bitmap32(const bitmap32 &);
		void operator=(const bitmap32 &);
	public:
		typedef color32 color_type;

		HBITMAP	m_hBitmap;

		bitmap32() { ::memset(this, 0, sizeof(*this)); }
		bitmap32(LONG w, LONG h) :m_hBitmap(0){ ::memset(this, 0, sizeof(*this)); create(w, h); }

		virtual ~bitmap32(){ destroy(); }

		bool create(LONG w, LONG h);
		void destroy();

		color_type & operator()(LONG x, LONG y)
		{
			assert(x >= 0 && x < LONG(m_width));
			assert(y >= 0 && y < LONG(m_height));
			const size_t n= x + y * pitch();
			return m_pBits[n];
		}

		const color_type & operator()(LONG x, LONG y)const
		{
			assert(x >= 0 && x < LONG(m_width));
			assert(y >= 0 && y < LONG(m_height));
			const size_t n = x + y * pitch();
			return m_pBits[n];
		}

		color_type & operator[](size_t n)
		{
			assert(n < m_count_of_pixels);
			return m_pBits[n];
		}

		const color_type & operator[](size_t n)const
		{
			assert(n < m_count_of_pixels);
			return m_pBits[n];
		}

		size_t size()const { return m_count_of_pixels; }

		bool load_from_file(const wchar_t * lpszFile);

		template<typename T>
		bool load_from_file(const T &strFile)
		{
			return load_from_file(&strFile[0]);
		}

		bool load_from_resource(size_t nRescId, LPCTSTR lpType, HINSTANCE hInstance);
		bool load_from_resource(size_t nRescId, LPCTSTR lpType)
		{
			return load_from_resource(nRescId, lpType, *g_app);
		}

		bool save_to_bmp(LPCTSTR lpszFile);
		bool save_to_bmp(const string &strFile)
		{
			return save_to_bmp(strFile.c_str());
		}
		
		LONG width()const 
		{ return LONG(m_width); }

		LONG height()const 
		{ return LONG(m_height); }

		void copy(LONG x, LONG y, LONG w, LONG h,
			const bitmap32 &src, LONG src_x, LONG src_y)
		{
			const RECT rt = { x, y, x + w, y + h };
			copy(rt, src, src_x, src_y);
		}

		void copy(const RECT &rt, const bitmap32 &src,
			LONG src_x, LONG src_y);

		void transcopy(LONG x, LONG y, LONG w, LONG h,
			const bitmap32 &src, LONG src_x, LONG src_y, color_type clrTrans)
		{
			const RECT rt = { x, y, x + w, y + h };
			transcopy(rt, src, src_x, src_y, clrTrans);
		}

		void transcopy(const RECT &rt, const bitmap32 &src,
			LONG src_x, LONG src_y, color_type clrTrans);

		void alpha(const RECT &rt, const bitmap32 &src,
			LONG src_x, LONG src_y, uint8 nAlphaValue);

		void alpha(LONG x, LONG y, LONG w, LONG h,
			const bitmap32 &src, LONG src_x, LONG src_y, uint8 nAlphaValue)
		{
			const RECT rt = { x, y, x + w, y + h };
			alpha(rt, src, src_x, src_y, nAlphaValue);
		}

		void alpha(const RECT &rt, const bitmap32 &src,LONG src_x, LONG src_y);
		void alpha(LONG x, LONG y, LONG w, LONG h,
			const bitmap32 &src, LONG src_x, LONG src_y)
		{
			const RECT rt = { x, y, x + w, y + h };
			alpha(rt, src, src_x, src_y);
		}

		void transalpha(const RECT &rt, const bitmap32 &src,
			LONG src_x, LONG src_y, uint8 nAlphaValue, color_type clrTrans);

		void transalpha(LONG x, LONG y, LONG w, LONG h, const bitmap32 &src,
			LONG src_x, LONG src_y, uint8 nAlphaValue, color_type clrTrans)
		{
			const RECT rt = { x, y, x + w, y + h };
			transalpha(rt, src, src_x, src_y, nAlphaValue, clrTrans);
		}

		void fill(size_t pos, size_t count, color_type clr);
		void fill(color_type clr) { fill(0, m_count_of_pixels, clr); }
		void fill(const RECT &rt, color_type clr);
		void fill(LONG x, LONG y, LONG w, LONG h, color_type clr)
		{
			fill(rect(x,y,x+w,y+h),clr);
		}

		void zero()
		{
			::memset( m_pBits, 0, m_count_of_pixels * sizeof(color_type) );
		}

		void alpha(uint8 nAlphaValue, color_type clr);
		void alpha(uint8 nAlphaValue, color_type clr, const RECT &rt);
		void alpha(uint8 nAlphaValue, color_type clr, LONG x,LONG y,LONG w,LONG h)
		{
			alpha(nAlphaValue, clr, rect(x,y,x+w,y+h));
		}

		void transalpha(uint8 nAlphaValue, color_type clr, color_type clrTrans);
		void transalpha(uint8 nAlphaValue, color_type clr, const rect &rt, color_type clrTrans);
		void transalpha(uint8 nAlphaValue, color_type clr, 
			LONG x,LONG y,LONG w,LONG h, color_type clrTrans)
		{
			transalpha(nAlphaValue, clr, rect(x,y,x+w,y+h),clrTrans);
		}

		size_t address(const POINT &pt)const
		{ 
			assert(pt.x < LONG(m_width) && pt.y < LONG(m_height));
			return pt.x + pt.y * pitch(); 
		}

		size_t address(LONG x, LONG y)const
		{ 
			assert(x < LONG(m_width) && y < LONG(m_height));
			return x + y * pitch(); 
		}

		inline LONG pitch()const
		{
			return m_width;
			//			const size_t n= m_width + 1; 
			//			return (n >> 1) << 1;
		}
	private:

		color32	*m_pBits;
		size_t	m_count_of_pixels;

		size_t	m_width, m_height;
	};

	// --------------------------------------------------------------------

	class bitmap1
	{
		bitmap1(const bitmap1 &);
		void operator=(const bitmap1 &);
	public:
		HBITMAP	m_hBitmap;

		bitmap1()
		{
			::memset(this, 0, sizeof(*this));
		}

		~bitmap1(){ destroy();}

		bool create(LONG w, LONG h);
		void destroy();

		unsigned char & operator[](size_t n)
		{
			assert(n < m_count_of_pixels);
			return m_pBits[n];
		}

		const unsigned char & operator[](size_t n)const
		{
			assert(n < m_count_of_pixels);
			return m_pBits[n];
		}

		size_t size()const { return m_count_of_pixels; }

		LONG pitch()const
		{
			// pitch必须是4的倍数
			const size_t n = ((m_width + 31) >> 5) << 2;
			return LONG(n);
		}

	private:
		unsigned char	*m_pBits;
		size_t	m_count_of_pixels;
		size_t	m_width, m_height;
	};

	// --------------------------------------------------------------------

	class dcclass
	{
		struct union_property
		{
			HDC	m_hdc;
		};

		// --------------------------------------------------------------------

		template< size_t nType >
		struct stock_pen_template : union_property
		{
			enum{ ID = nType };

			operator HPEN()const
			{
				return HPEN(::GetStockObject(int(nType)));
			}

			HPEN operator()()const { return *this; }
		};

		// --------------------------------------------------------------------

		template< size_t nType >
		struct stock_brush_template : union_property
		{
			enum{ ID = nType };

			operator HBRUSH()const
			{
				return HBRUSH(::GetStockObject(int(ID)));
			}

			HBRUSH operator()()const { return *this; }
		};

		// --------------------------------------------------------------------

		template< size_t nType >
		struct stock_font_template : union_property
		{
			enum{ ID = nType };

			operator HFONT()const
			{
				return HFONT(::GetStockObject(int(nType)));
			}

			HFONT operator()()const { return *this; }

			int height()const
			{
				HFONT hFont = *this;
				LOGFONT lf;
				::GetObject(hFont, sizeof(lf), &lf);
				return (lf.lfHeight > 0) ? lf.lfHeight : -lf.lfHeight;
			}
		};

		// --------------------------------------------------------------------

		struct text_color_t : union_property
		{
			operator DWORD()const
			{
				return ::GetTextColor(m_hdc);
			}

			DWORD operator()()const { return *this; }

			text_color_t & operator=(DWORD clr)
			{
				::SetTextColor(m_hdc, clr);  return *this;
			}
		};

		// --------------------------------------------------------------------

		struct bkcolor_t : union_property
		{
			operator DWORD()const
			{
				return ::GetBkColor(m_hdc);
			}

			DWORD operator()()const { return *this; }

			DWORD operator=(DWORD clr)
			{
				::SetBkColor(m_hdc, clr);  return clr;
			}
		};

		// --------------------------------------------------------------------

		struct bkmode_t : union_property
		{
			operator int()const { return ::GetBkMode(m_hdc); }
			int operator()()const { return *this; }

			int operator=(int nMode)
			{
				::SetBkMode(m_hdc, nMode); return nMode;
			}
		};

		// --------------------------------------------------------------------

		struct window_org_t : union_property
		{
			operator POINT()const
			{
				POINT pt;
				BOOL b = GetWindowOrgEx(m_hdc, &pt);
				assert(b != FALSE);
				return pt;
			}

			POINT offset(LONG x, LONG y)
			{
				const POINT pt = *this;
				BOOL b = ::SetWindowOrgEx(m_hdc, pt.x + x, pt.y + y, 0);
				return pt;
			}

			POINT offset(const POINT &pt) { return offset(pt.x, pt.y); }

			POINT operator()()const { return *this; }

			const POINT & operator=(const POINT &pt)
			{
				set(pt.x, pt.y);
				return pt;
			}

			POINT set(LONG x, LONG y)
			{
				POINT pt;
				BOOL b = ::SetWindowOrgEx(m_hdc, x, y, &pt);
				return pt;
			}
		};

		// --------------------------------------------------------------------

		struct window_ext_t : union_property
		{
			operator POINT()const
			{
				SIZE sz;
				BOOL b = ::GetWindowExtEx(m_hdc, &sz);
				assert(b != FALSE);
				assert(sizeof(SIZE) == sizeof(POINT));
				return reinterpret_cast<POINT &>(sz);
			}

			POINT operator()()const { return *this; }

			const POINT & operator=(const POINT &pt)
			{
				set(pt.x, pt.y);
				return pt;
			}

			POINT set(LONG x, LONG y)
			{
				SIZE sz;
				BOOL b = ::SetWindowExtEx(m_hdc, x, y, &sz);
				return reinterpret_cast<POINT &>(sz);
			}
		};

		// --------------------------------------------------------------------

		struct viewport_org_t : union_property
		{
			operator POINT()const
			{
				POINT pt;
				BOOL b = GetViewportOrgEx(m_hdc, &pt);
				assert(b != FALSE);
				return pt;
			}

			POINT operator()()const { return *this; }

			POINT offset(LONG x, LONG y)
			{
				const POINT pt = *this;
				BOOL b = ::SetViewportOrgEx(m_hdc, pt.x + x, pt.y + y, 0);
				return pt;
			}

			POINT offset(const POINT &pt) { return offset(pt.x, pt.y); }

			const POINT & operator=(const POINT &pt)
			{
				set(pt.x, pt.y);
				return pt;
			}

			POINT set(LONG x, LONG y)
			{
				POINT pt;
				BOOL b = ::SetViewportOrgEx(m_hdc, x, y, &pt);
				return pt;
			}
		};

		// --------------------------------------------------------------------

		struct viewport_ext_t : union_property
		{
			operator POINT()const
			{
				SIZE sz;
				BOOL b = ::GetViewportExtEx(m_hdc, &sz);
				assert(b != FALSE);
				assert(sizeof(SIZE) == sizeof(POINT));
				return reinterpret_cast<POINT &>(sz);
			}

			POINT operator()()const { return *this; }

			const POINT & operator=(const POINT &pt)
			{
				set(pt.x, pt.y);
				return pt;
			}

			POINT set(LONG x, LONG y)
			{
				SIZE sz;
				BOOL b = ::SetViewportExtEx(m_hdc, x, y, &sz);
				return reinterpret_cast<POINT &>(sz);
			}
		};

		// --------------------------------------------------------------------

		struct clip_box_t : union_property
		{
			operator RECT()const
			{
				RECT rt;
				BOOL n = ::GetClipBox(m_hdc, &rt);
				return rt;
			}

			RECT operator()()const { return *this; }
		};

		// --------------------------------------------------------------------

		struct ROP2_t : union_property
		{
			operator int()const { return ::GetROP2(m_hdc); }

			int operator()()const { return *this; }

			int operator=(int nROP)
			{
				::SetROP2(m_hdc, nROP);
				return nROP;
			}
		};

		// --------------------------------------------------------------------

		struct text_metrics_t : union_property
		{
			TEXTMETRIC & operator()(TEXTMETRIC &tm)const
			{
				::GetTextMetrics(m_hdc, &tm);
				return tm;
			}

			TEXTMETRIC  operator()()const 
			{ 
				TEXTMETRIC tm;
				return operator()(tm);
			}
		};

	public:

		// --------------------------------------------------------------------

		class sentry
		{
			dcclass			&m_gc;
			const size_t	m_index;
		public:

			explicit sentry(dcclass &gc) : m_gc(gc),
				m_index(m_gc.m_object_count) {}

			~sentry(){
				if (m_gc.m_object_count > m_index)
					m_gc.discard(m_gc.m_object_count - m_index);
			}
		};

		union
		{
			HDC m_hdc;

			stock_pen_template< NULL_PEN >		null_pen;
			stock_pen_template< WHITE_PEN >		white_pen;
			stock_pen_template< BLACK_PEN >		black_pen;

			stock_brush_template< NULL_BRUSH >	null_brush;
			stock_brush_template< WHITE_BRUSH >	white_brush;
			stock_brush_template< BLACK_BRUSH >	black_brush;
			stock_brush_template< GRAY_BRUSH >	gray_brush;
			stock_brush_template< LTGRAY_BRUSH>	light_gray_brush;
			stock_brush_template< DKGRAY_BRUSH>	dark_gray_brush;

			stock_font_template< SYSTEM_FONT > system_font;
			stock_font_template< DEFAULT_GUI_FONT > default_font;

			text_color_t	text_color;
			bkcolor_t		bkcolor;
			bkmode_t		bkmode;

			window_org_t	window_org;
			window_ext_t	window_ext;

			viewport_org_t	viewport_org;
			viewport_ext_t	viewport_ext;

			clip_box_t		clip_box;
			//		clip_region_t	clip_region;

			ROP2_t	ROP2;

			text_metrics_t	text_metrics;
		};

		bitmap32 *bmp32;

		explicit dcclass(bool is_create_mem_dc = false);
		dcclass(bool is_create_mem_dc, HDC h);
		dcclass(HWND hWnd, bool is_create_window_dc);

		virtual ~dcclass();

		operator HDC()const { return m_hdc; }

		void discard_all();
		void discard();
		void discard(size_t n);

		void use(HANDLE h);

		void use(const bitmap32 &bmp) { use(bmp.m_hBitmap); }

		void use_pen(HPEN hPen) { use(hPen); }
		void use_brush(HBRUSH hBrush){ use(hBrush); }
		void use_font(HFONT hFont) { use(hFont); }
		void use_bitmap(HBITMAP hBitmap) { use(hBitmap); }

		// viewport to window
		void v2w(POINT *p, size_t n, bool bNoZoon = false)const;

		void v2w(LONG &x, LONG &y, bool bNoZoon = true)const;

		POINT & v2w(POINT &pt, bool bNoZoon = false)const
		{
			v2w(&pt, 1, bNoZoon); return pt;
		}

		RECT & v2w(RECT &rt, bool bNoZoon = false)const
		{
			v2w(reinterpret_cast<POINT *>(&rt), bNoZoon); return rt;
		}

		// window to viewport
		void w2v(POINT *p, size_t n, bool bNoZoon = false)const;

		void w2v(LONG &x, LONG &y, bool bNoZoon = false)const;

		POINT & w2v(POINT &pt, bool bNoZoon = false)const
		{
			w2v(&pt, 1, bNoZoon); return pt;
		}

		RECT & w2v(RECT &rt, bool bNoZoon = false)const
		{
			w2v(reinterpret_cast<POINT *>(&rt), 2, bNoZoon); return rt;
		}

		void text(LONG x, LONG y, const wchar_t * first, size_t n)
		{
			::TextOut(m_hdc, x, y, first, int(n));
		}

		void text(LONG x, LONG y, const wchar_t * lpszText)
		{
			::TextOut(m_hdc, x, y, lpszText, int(::wcslen(lpszText)));
		}

		void text(LONG x, LONG y, const string &s)
		{
			::TextOut(m_hdc, x, y, s.c_str(), int(s.size()));
		}

		void center_text(const RECT &rt, const TCHAR * first, size_t n)
		{
			RECT tmp=rt;
			text_rect(first, n, tmp);
			text((rt.left + rt.right - (tmp.right - tmp.left)) / 2, (rt.top + rt.bottom - (tmp.bottom - tmp.top)) / 2, first, n);
		}

		void center_text(const RECT &rt, const TCHAR *lpszText)
		{
			center_text(rt, lpszText, ::wcslen(lpszText));
		}

		void center_text(const RECT &rt, const string &s)
		{
			center_text(rt, s.c_str(), s.size());
		}

		rect & text_rect(const wchar_t *first, size_t n, RECT &rt)
		{
			::DrawText(m_hdc, first, n, &rt, DT_SINGLELINE | DT_CALCRECT);
			return static_cast<rect &>(rt);
		}
		rect & text_rect(const wchar_t *first, RECT &rt)
		{
			::DrawText(m_hdc, first, ::wcslen(first), &rt, DT_SINGLELINE | DT_CALCRECT);
			return static_cast<rect &>(rt);
		}
		rect & text_rect(const string &s, RECT &rt)
		{
			::DrawText(m_hdc, s.c_str(), s.size(), &rt, DT_SINGLELINE | DT_CALCRECT);
			return static_cast<rect &>(rt);
		}

		rect text_rect(const wchar_t *first, size_t n)
		{
			RECT rt = { 0, 0, 1920, 1080 };
			return text_rect(first, n, rt);
		}
		rect text_rect(const wchar_t *first)
		{
			return text_rect(first, ::wcslen(first));
		}
		rect text_rect(const string &s)
		{
			return text_rect(s.c_str(), s.size());
		}

		void fill(const RECT &rt, HBRUSH br)
		{
			::FillRect(m_hdc, &rt, br);
		}

		void fill(const RECT &rt, DWORD clr)
		{
			brush br(clr);
			::FillRect(m_hdc, &rt, br);
		}

		void bitblt(LONG x, LONG y, LONG w, LONG h,
			HDC hSrc, LONG src_x, LONG src_y, DWORD dwRop = SRCCOPY)
		{
			::BitBlt(*this, x, y, w, h, hSrc, src_x, src_y, dwRop);
		}

		void bitblt(LONG x, LONG y, LONG w, LONG h,
			HDC hSrc, DWORD dwRop = SRCCOPY)
		{
			const BOOL b = ::BitBlt(*this, x, y, w, h, hSrc, x, y, dwRop);
		}

		void bitblt(const RECT &rt, HDC hSrc, LONG src_x,
			LONG src_y, DWORD dwRop = SRCCOPY)
		{
			::BitBlt(*this, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top,
				hSrc, src_x, src_y, dwRop);
		}

		void bitblt(const RECT &rt, HDC hSrc, DWORD dwRop = SRCCOPY)
		{
			::BitBlt(*this, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top,
				hSrc, rt.left, rt.top, dwRop);
		}

		void patblt(LONG x, LONG y, LONG w, LONG h,
			DWORD dwRop = PATCOPY)
		{
			::PatBlt(m_hdc, x, y, w, h, dwRop);
		}

		void patblt(const RECT &rt, DWORD dwRop = PATCOPY)
		{
			::PatBlt(m_hdc, rt.left, rt.top, rt.right - rt.left, rt.bottom - rt.top, dwRop);
		}

		void line( LONG x1,LONG y1,LONG x2,LONG y2)
		{
			::MoveToEx( m_hdc, x1,y1,0);
			::LineTo( m_hdc, x2,y2);
		}

		void line(const RECT &rt);
		void hline(LONG first_x, LONG second_x, LONG y)
		{
			::MoveToEx(m_hdc, first_x, y, 0);
			::LineTo(m_hdc, second_x, y);
		}

		void vline(LONG first_y, LONG second_y, LONG x)
		{
			::MoveToEx(m_hdc, x, first_y, 0);
			::LineTo(m_hdc, x, second_y);
		}

		void line(const RECT &rt, HPEN hPen)
		{
			use_pen(hPen);
			line(rt);
			discard();
		}

		void hline(LONG first_x, LONG second_x, LONG y, HPEN hPen)
		{
			use_pen(hPen);
			hline(first_x, second_x, y);
			discard();
		}

		void vline(LONG first_y, LONG second_y, LONG x, HPEN hPen)
		{
			use_pen(hPen);
			vline(first_y, second_y, x);
			discard();
		}

		void line(const RECT &rt, DWORD clr)
		{
			pen pe(clr);
			use_pen(pe);
			line(rt);
			discard();
		}

		void hline(LONG first_x, LONG second_x, LONG y, DWORD clr)
		{
			pen pe(clr);
			use_pen(pe);
			hline(first_x, second_x, y);
			discard();
		}

		void vline(LONG first_y, LONG second_y, LONG x, DWORD clr)
		{
			pen pe(clr);
			use_pen(pe);
			vline(first_y, second_y, x);
			discard();
		}

		int char_width(UINT ch)
		{
			int result= 0;
			::GetCharWidth(m_hdc, ch, ch, &result);
			return result;
		}

		void move_to(LONG x, LONG y)
		{
			::MoveToEx(m_hdc, x, y, 0);
		}
		void line_to(LONG x, LONG y)
		{
			::LineTo(m_hdc, x, y);
		}
	private:
		union
		{
			HWND	m_hWnd;
			size_t	m_dc_type;
		};
		HGDIOBJ	*m_objects;
		uint16 m_object_count;
		uint16 m_object_capacity;	// 最后一个memset变量
	};

	// --------------------------------------------------------------------

	class wdc : public dcclass
	{
	public:
		explicit wdc(HWND hWnd) : dcclass(hWnd, true){}
	};

	// --------------------------------------------------------------------
	// class cdc
	// --------------------------------------------------------------------

	class cdc : public dcclass
	{
	public:

		cdc() :dcclass(HWND(0), false) {}
		explicit cdc(HWND hWnd) :dcclass(hWnd, false){}
	};

	// --------------------------------------------------------------------

	class mdc : public dcclass
	{
	public:
		mdc() : dcclass(true){}
		explicit mdc(const HDC &dc) : dcclass(true, dc){}
		explicit mdc(const dcclass &dc) : dcclass(true, dc){}
		explicit mdc(const mdc &dc) : dcclass(true, dc){}

		explicit mdc(bitmap32 &bmp) : dcclass(true)
		{
			assert(bmp.m_hBitmap);
			bmp32 = &bmp;
			use_bitmap(bmp.m_hBitmap);
		}

		mdc(const HDC &dc, bitmap32 &bmp) : dcclass(true, dc)
		{
			assert(bmp.m_hBitmap);
			bmp32 = &bmp;
			use_bitmap(bmp.m_hBitmap);
		}

		mdc(const dcclass &dc, bitmap32 &bmp) : dcclass(true, dc)
		{
			assert(bmp.m_hBitmap);
			bmp32 = &bmp;
			use_bitmap(bmp.m_hBitmap);
		}

		mdc(const mdc &dc, bitmap32 &bmp) : dcclass(true, dc)
		{
			assert(bmp.m_hBitmap);
			bmp32 = &bmp;
			use_bitmap(bmp.m_hBitmap);
		}
	};

	// --------------------------------------------------------------------

	class font
	{
		font(const font &);
		void operator=(const font &);
	public:
		HFONT	m_hFont;

		font() : m_hFont(0){}
		explicit font(const LOGFONT &lf) : m_hFont(0)
		{
			create(lf);
		}
		font(int nHeight, const wchar_t * lpszFaceName, int nCharset = EASTEUROPE_CHARSET)
			: m_hFont(0) 
		{
			create(nHeight, lpszFaceName, nCharset);
		}
		~font(){ destroy(); }

		operator HFONT()const { return m_hFont;  }

		bool create(int nHeight, const wchar_t * lpszFaceName,
			int nCharset = EASTEUROPE_CHARSET);

		bool create(const LOGFONT &lf)
		{
			assert(m_hFont == 0);
			m_hFont = ::CreateFontIndirect(&lf);
			bool result = m_hFont != 0;
			return result;
		}

		void destroy() 
		{
			if (m_hFont != 0)
			{
				::DeleteObject(m_hFont);
				m_hFont = 0;
			}
		}

		int height()const 
		{
			LOGFONT lf;
			::GetObject(m_hFont, sizeof(lf), &lf);
			return (lf.lfHeight > 0) ? lf.lfHeight : -lf.lfHeight;
		}

		int width()const
		{
			LOGFONT lf;
			::GetObject(m_hFont, sizeof(lf), &lf);
			return lf.lfWidth;
		}

		void swap(font &rhs)
		{
			std::swap(m_hFont, rhs.m_hFont);
		}

		bool choose_dialog(HWND hWnd);
	};

	// --------------------------------------------------------------------

	class drawtxt
	{
		RECT	m_rect;		// 第一个memset的变量
		string::const_pointer	m_text;
		size_t	m_text_size;
		size_t	m_visible_size;
		LONG	m_visible_width;
		LONG	m_width_of_dot3;
		size_t	m_flags;

		int	*m_width_of_chars;// 最后一个memset的变量

	public:

		enum{ align_left= 0, align_right=0x00000001,
			align_center=0x00000002, align_adjust= 0x00000004,

			align_vcenter= 0x00000008,align_bottom=0x00000010,
			align_evenly_spaced = align_bottom *2,	// 等间距，这意味着若一个字符就居中
			flag_3dot= 0x00000100,

			align_hfields= align_left | align_right | align_center |  align_adjust,
			align_vfields= align_bottom | align_vcenter,
		};

	public:
		dcclass	&m_gc;

		explicit drawtxt(dcclass &dc);
		~drawtxt();

		drawtxt & put(string::const_pointer lpszText, size_t n, const RECT &rtBound,
			size_t flags= align_left);
		drawtxt & put(string::const_pointer lpszText, const RECT &rtBound,
			size_t flags= align_left)
		{
			return put(lpszText, ::wcslen(lpszText), rtBound, flags);
		}
		drawtxt & put(const string &s, const RECT &rtBound,
			size_t flags= align_left)
		{
			return put(s.c_str(), s.size(), rtBound, flags);
		}

		void draw(LONG xoffset=0, LONG yoffset=0);

		const rect & bound()const { return static_cast<const rect &>(m_rect); }
		LONG char_width(size_t n)const { return m_width_of_chars[n]; }
		LONG visible_width()const { return m_visible_width; }
		size_t visible_text_count()const { return m_visible_size; }
	};

	// --------------------------------------------------------------------
	// class region
	// --------------------------------------------------------------------

	class region
	{
		region( const region &rgn1, const region &rgn2, int nCombineMode );

		bool _create( LONG left, LONG top, LONG right, LONG bottom )
		{
			m_hRgn= ::CreateRectRgn( left, top, right, bottom );
			bool result= m_hRgn != 0;
//			api_check( result, "CreateRectRgn" );
			return result;
		}

		void _destroy()
		{ 
			if (m_hRgn)
				::DeleteObject( m_hRgn ); 
		}

	public:

		typedef region self;

		HRGN	m_hRgn;

	public:

		region() : m_hRgn(0){}

		region(const POINT *pts, size_t n, int nMode= WINDING);

		region( LONG left, LONG top, LONG right,LONG bottom )
		{ _create( left, top, right, bottom ); }

		explicit region( const RECT & rt )
		{ _create( rt.left, rt.top, rt.right, rt.bottom ); }

		region( const self & rhs );

		~region()
		{ _destroy(); }

		operator HRGN()const 
		{ return m_hRgn; }

		self& operator=( const rect & rt )
		{
			self(rt).swap(*this);
			return *this;
		};

		self& operator=(const self & rhs )
		{
			if ( this != & rhs )
				self(rhs).swap(*this);
			return *this;
		}

		self& operator+=( const self & rhs )
		{
			self( *this, rhs, RGN_OR ).swap(*this);
			return *this;
		}

		self& operator-=( const self & rhs )
		{
			self( *this, rhs, RGN_DIFF ).swap(*this);
			return *this;
		}

		rect bound()const
		{ 
			rect rt;
			::GetRgnBox( m_hRgn, &rt ); 
			return rt;
		}

		void swap( self &rhs )
		{
			std::swap( m_hRgn, rhs.m_hRgn );
		}
	};

	void clip_rect(const bitmap32 &me, RECT &rtMe, RECT &rtYou);
}