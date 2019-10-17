#include "w_dc.h"
#include "w_file.h"

// FIXME - the following function was comment in 'intrin.h'
//__MACHINEI(unsigned char _interlockedbittestandset(long volatile *a, long b))
//__MACHINEI(unsigned char _interlockedbittestandreset(long volatile *a, long b))

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>


#include <ocidl.h>
#include <olectl.h>

// --------------------------------------------------------------------
// class bitmap_loader
// --------------------------------------------------------------------

struct bitmap_loader
{
	void load(HGLOBAL hGlobal, LONG cb)
	{
		const int HIMETRIC_INCH = 2540;

		IStream *pStm;
		HRESULT hr = ::CreateStreamOnHGlobal(hGlobal, TRUE, &pStm);
		if (hr == S_OK)
		{

			IPicture *pPic;
			HRESULT hr = ::OleLoadPicture(pStm, cb, TRUE, IID_IPicture, (LPVOID*)&pPic);
			pStm->Release();
			//		api_check(SUCCEEDED(hr), "OleLoadPicture");

			OLE_XSIZE_HIMETRIC w;
			OLE_YSIZE_HIMETRIC h;
			wabc::mdc dc;

			pPic->get_Width(&w);
			pPic->get_Height(&h);

			const LONG cx = ::MulDiv(w, ::GetDeviceCaps(dc, LOGPIXELSX), HIMETRIC_INCH);
			const LONG cy = ::MulDiv(h, ::GetDeviceCaps(dc, LOGPIXELSY), HIMETRIC_INCH);

			HBITMAP hBitmap = create_bitmap(cx, cy);

			dc.use_bitmap(hBitmap);
			hr = pPic->Render(dc, 0, 0, cx, cy, 0, h, w, -h, 0);
			pPic->Release();
		}
	}

	bool load_from_file(const wchar_t * strFile)
	{
		wabc::file f(strFile, L"r", wabc::file::share_r);
		if (!f)
			return false;

		const LONG cb = (LONG)f.size();

		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, cb);
		if (!hGlobal)
			return false;

		LPVOID pvData = ::GlobalLock(hGlobal);
		if (pvData)
		{
			f.read(pvData, cb);

			::GlobalUnlock(hGlobal);

			load(hGlobal, cb);
		}
		::GlobalFree(hGlobal);
		return pvData != 0;
	}

	bool load_from_resource(HMODULE hModule, int nResc, LPCTSTR lpType)
	{
		HRSRC hRsrc = ::FindResource(hModule, MAKEINTRESOURCE(nResc), lpType);
		if (hRsrc == 0)
			return false;

		const DWORD dwSize = SizeofResource(hModule, hRsrc);
		if (dwSize == 0)
			return false;

		HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
		if (hGlobal == 0)
			return false;

		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
		if (hMem == NULL) return  FALSE;

		LPVOID  pSrc = ::LockResource(hGlobal);
		LPVOID pDes = ::GlobalLock(hMem);

		if (pSrc != 0 && pDes != 0)
		{
			memcpy(pDes, pSrc, dwSize);
			GlobalUnlock(hMem);

			load(hMem, LONG(dwSize));
		}
		else if (pDes)
			GlobalUnlock(hMem);

		::GlobalFree(hMem);
		return true;
	}

	virtual ~bitmap_loader(){}

	virtual HBITMAP create_bitmap(LONG cx, LONG cy) = 0;
};

namespace wabc
{
	// --------------------------------------------------------------------

	void clip_rect(const bitmap32 &me, RECT &rtMe, RECT &rtYou)
	{
		const LONG x = rtMe.left;
		const LONG y = rtMe.top;

		const rect rt(0, 0, me.width(), me.height());
		static_cast<rect &>(rtMe) &= rt;
		if (rtMe.left != x)
			rtYou.left += (rtMe.left - x);
		if (rtMe.top != y)
			rtYou.top += (rtMe.top - y);
	}
	
	// --------------------------------------------------------------------
	// class bitmap
	// --------------------------------------------------------------------

	bool bitmap::create(HDC hdc, LONG nWidth, LONG nHeight)
	{
		assert(m_hBitmap == 0);
		m_hBitmap = ::CreateCompatibleBitmap(hdc, nWidth, nHeight);
		bool result = m_hBitmap != 0;
		//api_check(result, "CreateCompatibleBitmap");
		return result;
	}

	// --------------------------------------------------------------------

	bool bitmap::create(LONG nWidth, LONG nHeight, const void *lpBits)
	{
		assert(m_hBitmap == 0);
		m_hBitmap = ::CreateBitmap(nWidth, nHeight, 1, 1, lpBits);
		bool result = m_hBitmap != 0;
		return result;
	}

	// --------------------------------------------------------------------

	struct load_bitmap_t : bitmap_loader
	{
		bitmap	&m_bitmap;

		load_bitmap_t(bitmap &bmp) : m_bitmap(bmp)
		{
		}

		virtual HBITMAP create_bitmap(LONG cx, LONG cy)
		{
			m_bitmap.create(cx, cy);
			return m_bitmap.m_hBitmap;
		}
	};

	void bitmap::load_from_file(const string &strFile)
	{
		load_bitmap_t a(*this);
		a.load_from_file(strFile.c_str());
	}

	// --------------------------------------------------------------------

	bool bitmap::load_from_resource(size_t nRescId, LPCTSTR lpType, HINSTANCE hInstance)
	{
		load_bitmap_t a(*this);
		if (lpType == RT_BITMAP)
		{
			m_hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(nRescId));
			return m_hBitmap != 0;
		}
		return a.load_from_resource(hInstance, nRescId, lpType);
	}

	// --------------------------------------------------------------------

	bool bitmap::create(const BITMAP &bm)
	{
		assert(m_hBitmap == 0);
		m_hBitmap = ::CreateBitmapIndirect(&bm);
		bool result = m_hBitmap != 0;
		//api_check(result, "CreateBitmapIndirect");
		return result;
	}

	// --------------------------------------------------------------------

	bool bitmap::load(size_t nResourceId, HINSTANCE hInstance)
	{
		assert(m_hBitmap == 0);
		m_hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(nResourceId));
		bool result = m_hBitmap != 0;
		//api_check(result, "LoadBitmap");
		return result;
	}

	// --------------------------------------------------------------------

	void bitmap::destroy()
	{
		if (m_hBitmap != 0)
		{
			::DeleteObject(m_hBitmap);
			m_hBitmap = 0;
		}
	}

	// --------------------------------------------------------------------

	bool bitmap::get_bits(void *bits, size_t uStartScan, size_t cScanLines)const
	{
		// --------------------------------------------------------------------

		struct monochrome_header : BITMAPINFOHEADER
		{
			RGBQUAD	bmiColors[2];

			monochrome_header(LONG cx, LONG cy)
			{
				::memset(this, 0, sizeof(*this));
				this->biSize = sizeof(BITMAPINFOHEADER);
				this->biWidth = cx;
				this->biHeight = -cy;
				this->biPlanes = 1;
				this->biBitCount = 1;	// monochrome bitmap
			}
		};

		BITMAP bm;
		::GetObject(m_hBitmap, sizeof(bm), &bm);

		const LONG w = bm.bmWidth;
		const LONG h = bm.bmHeight;
		monochrome_header header(w, h);

		wabc::cdc dc;
		const int n = ::GetDIBits(dc, m_hBitmap, uStartScan, cScanLines, bits,
			(LPBITMAPINFO)&header, DIB_RGB_COLORS);
		return true;
	}

	// --------------------------------------------------------------------

	size_t bitmap::size()const
	{
		BITMAP bm;
		::GetObject(m_hBitmap, sizeof(bm), &bm);

		const LONG w = bm.bmWidth;
		const LONG h = bm.bmHeight;

		// calculate the 'nRowLength', the 'nRowLength' must
		// be the multiple of 4
		//
		// nRowLength= (w+31) / 32 * 4
		//
		return ((w + 31) >> 5) << 2;
	}

	// --------------------------------------------------------------------
	// bitmap32
	// --------------------------------------------------------------------

	void bitmap32::destroy()
	{
		if (m_hBitmap != 0)
		{
			::DeleteObject(m_hBitmap);
			::memset(this, 0, sizeof(*this));
		}
	}

	// --------------------------------------------------------------------

	struct load_bitmap32_t : bitmap_loader
	{
		bitmap32	&m_bitmap32;

		load_bitmap32_t(bitmap32 &bmp) : m_bitmap32(bmp)
		{
		}

		virtual HBITMAP create_bitmap(LONG cx, LONG cy)
		{
			m_bitmap32.create(cx, cy);
			return m_bitmap32.m_hBitmap;
		}
	};

	bool bitmap32::load_from_file(const wchar_t * lpszFile)
	{
		load_bitmap32_t a(*this);
		return a.load_from_file(lpszFile);
	}

	// --------------------------------------------------------------------

	bool bitmap32::load_from_resource(size_t nRescId, LPCTSTR lpType, HINSTANCE hInstance)
	{
		load_bitmap32_t a(*this);
		if (lpType == RT_BITMAP)
		{
			HBITMAP hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(nRescId));
			if (hBitmap)
			{
				BITMAP bm;
				::GetObject(hBitmap, sizeof(bm), &bm);

				create(bm.bmWidth, bm.bmHeight);
				mdc dcSrc, dcDest;
				dcSrc.use_bitmap(hBitmap);
				dcDest.use_bitmap(m_hBitmap);
				dcDest.bitblt(0, 0, bm.bmWidth, bm.bmHeight, dcSrc, 0, 0);

				dcSrc.discard();
				DeleteObject(hBitmap);
				return true;
			}
			return false;
		}
		return a.load_from_resource(hInstance, nRescId, lpType);
	}

	// --------------------------------------------------------------------

	bool bitmap32::save_to_bmp(LPCTSTR lpszFile)
	{
		BITMAPINFOHEADER bih;

		::memset(&bih, 0, sizeof(bih));
		bih.biSize = sizeof(bih);
		bih.biWidth = LONG(m_width);
		bih.biHeight = -LONG(m_height);
		bih.biPlanes = 1;
		bih.biBitCount = 32;	// 8-8-8-8
		bih.biCompression = BI_RGB;

		BITMAPFILEHEADER bfh;

		bfh.bfType = 0x4D42; //BM
		bfh.bfSize = sizeof(bfh)+sizeof(bih)+m_count_of_pixels * sizeof(color_type);
		bfh.bfReserved1 = bfh.bfReserved2 = 0;
		bfh.bfOffBits = sizeof(bfh)+bih.biSize;//+colors*sizeof(RGBQUAD);

		HANDLE hFile = ::CreateFile(lpszFile, GENERIC_WRITE,
			FILE_SHARE_READ, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		bool result = false;
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD write;
			WriteFile(hFile, &bfh, sizeof(bfh), &write, NULL);
			if (write == sizeof(bfh))
			{
				WriteFile(hFile, &bih, sizeof(bih), &write, NULL);
				if (write == sizeof(bih))
				{
					const DWORD n = m_count_of_pixels * sizeof(color_type);
					WriteFile(hFile, m_pBits, n, &write, NULL);
					result = write == n;
				}
			}
			::CloseHandle(hFile);
		}
		return result;
	}

	// --------------------------------------------------------------------

	bool bitmap32::create(LONG nWidth, long nHeight)
	{
		assert(m_hBitmap == 0);

		struct bmp_header : BITMAPINFOHEADER
		{
			bmp_header(LONG nWidth, LONG nHeight)
			{
				::memset(this, 0, sizeof(BITMAPINFOHEADER));
				this->biSize = sizeof(BITMAPINFOHEADER);
				this->biWidth = nWidth;
				this->biHeight = -nHeight;
				this->biPlanes = 1;
				this->biBitCount = 32;	// 8-8-8-8
				this->biCompression = BI_RGB;
			}
		} bmph(nWidth, nHeight);

		cdc hdc;

		HBITMAP hBitmap = ::CreateDIBSection(hdc, (const BITMAPINFO *)&bmph, DIB_RGB_COLORS,
			(void **)&m_pBits, 0, 0);

		//wabc::api_check( m_hBitmap != 0, "CreateDIBSection" );

		if (hBitmap)
		{
			m_hBitmap = hBitmap;
			m_width = nWidth;
			m_height = nHeight;

			m_count_of_pixels = nWidth * nHeight;
			return true;
		}
		return false;
	}

	// --------------------------------------------------------------------

	void bitmap32::alpha(const RECT &rt, const bitmap32 &src,
		LONG src_x, LONG src_y, uint8 nAlphaValue)
	{
		assert(m_hBitmap && src.m_hBitmap);
		if (nAlphaValue == 0)
			return;

		if (nAlphaValue == 0xFF)
		{
			copy(rt, src, src_x, src_y);
			return;
		}

		RECT rtThis = rt;
		RECT rtSrc = { src_x, src_y, src_x + wabc::width(rt), src_y + wabc::height(rt) };
		clip_rect(*this, rtThis, rtSrc);
		clip_rect(src, rtSrc, rtThis);

		const LONG w = _min(wabc::width(rtSrc), wabc::width(rtThis));
		LONG h = _min(wabc::height(rtSrc), wabc::height(rtThis));

		if (w > 0 && h > 0)
		{
			const color_type *pSrc = src.m_pBits + src.address(rtSrc.left, rtSrc.top);
			color_type *pDest = m_pBits + address(rtThis.left, rtThis.top);

			const uint beta = 255 - nAlphaValue;
			LONG i;

			__declspec(align(16)) __m128i mm0, mm1, mm2, mm3, zero = _mm_setzero_si128();

			mm2 = _mm_set1_epi32(size_t(nAlphaValue));
			mm3 = _mm_set1_epi32(size_t(beta));

			for (; h > 0; --h)
			{
				for (i = 0; i < w; ++i)
				{
					const color_type &src_clr = pSrc[i];
					color_type &dest_clr = pDest[i];

					//const DWORD r = src_clr.r * nAlphaValue + dest_clr.r * beta;
					//const DWORD g = src_clr.g * nAlphaValue + dest_clr.g * beta;
					//const DWORD b = src_clr.b * nAlphaValue + dest_clr.b * beta;

					//dest_clr.r = uint8(r / 256);
					//dest_clr.g = uint8(g / 256);
					//dest_clr.b = uint8(b / 256);

					mm0 = _mm_set1_epi32(src_clr.m_bgra);
					mm1 = _mm_set1_epi32(dest_clr.m_bgra);

					mm0 = _mm_unpackhi_epi8(mm0, zero);
					mm0 = _mm_unpackhi_epi16(mm0, zero);

					mm1 = _mm_unpackhi_epi8(mm1, zero);
					mm1 = _mm_unpackhi_epi16(mm1, zero);

					mm0 = _mm_madd_epi16(mm0, mm2);
					mm1 = _mm_madd_epi16(mm1, mm3);

					mm0 = _mm_add_epi32(mm0, mm1);
					mm0 = _mm_srli_epi32(mm0, 8);
					mm0 = _mm_packs_epi32(mm0, zero);
					mm0 = _mm_packus_epi16(mm0, zero);
					dest_clr.m_bgra = mm0.m128i_i32[0];
				}
				pSrc += src.pitch();
				pDest += pitch();
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::alpha(const RECT &rt, const bitmap32 &src,LONG src_x, LONG src_y)
	{
		assert(m_hBitmap && src.m_hBitmap);

		RECT rtThis = rt;
		RECT rtSrc = { src_x, src_y, src_x + wabc::width(rt), src_y + wabc::height(rt) };
		clip_rect(*this, rtThis, rtSrc);
		clip_rect(src, rtSrc, rtThis);

		const LONG w = _min(wabc::width(rtSrc), wabc::width(rtThis));
		LONG h = _min(wabc::height(rtSrc), wabc::height(rtThis));

		if (w > 0 && h > 0)
		{
			const color_type *pSrc = src.m_pBits + src.address(rtSrc.left, rtSrc.top);
			color_type *pDest = m_pBits + address(rtThis.left, rtThis.top);

			LONG i;

			__declspec(align(16)) __m128i mm0, mm1, mm2, mm3, zero = _mm_setzero_si128();
			for (; h > 0; --h)
			{
				for (i = 0; i < w; ++i)
				{
					const color_type &src_clr = pSrc[i];
					color_type &dest_clr = pDest[i];

					const uint8 &a = src_clr.a;
					if (a)
					{
						if (a == 0xFF)
						{
							dest_clr.m_bgra = src_clr.m_bgra;
						}
						else
						{
							const uint8 beta = 255 - a;

							//const DWORD r = src_clr.r * a + dest_clr.r * beta;
							//const DWORD g = src_clr.g * a + dest_clr.g * beta;
							//const DWORD b = src_clr.b * a + dest_clr.b * beta;

							//dest_clr.r = uint8(r / 256);
							//dest_clr.g = uint8(g / 256);
							//dest_clr.b = uint8(b / 256);

							mm2 = _mm_set1_epi32(size_t(a));
							mm3 = _mm_set1_epi32(size_t(beta));

							mm0 = _mm_set1_epi32(src_clr.m_bgra);
							mm1 = _mm_set1_epi32(dest_clr.m_bgra);

							mm0 = _mm_unpackhi_epi8(mm0, zero);
							mm0 = _mm_unpackhi_epi16(mm0, zero);

							mm1 = _mm_unpackhi_epi8(mm1, zero);
							mm1 = _mm_unpackhi_epi16(mm1, zero);

							mm0 = _mm_madd_epi16(mm0, mm2);
							mm1 = _mm_madd_epi16(mm1, mm3);

							mm0 = _mm_add_epi32(mm0, mm1);
							mm0 = _mm_srli_epi32(mm0, 8);
							mm0 = _mm_packs_epi32(mm0, zero);
							mm0 = _mm_packus_epi16(mm0, zero);
							dest_clr.m_bgra = mm0.m128i_i32[0];
						}
					}
				}
				pSrc += src.pitch();
				pDest += pitch();
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::transalpha(const RECT &rt, const bitmap32 &src,
		LONG src_x, LONG src_y, uint8 nAlphaValue, color_type clrTrans)
	{
		assert(m_hBitmap && src.m_hBitmap);

		if (nAlphaValue == 0)
			return;

		if (nAlphaValue == 0xFF)
		{
			transcopy(rt, src, src_x, src_y, clrTrans);
			return;
		}

		RECT rtThis = rt;
		RECT rtSrc = { src_x, src_y, src_x + wabc::width(rt), src_y + wabc::height(rt) };
		clip_rect(*this, rtThis, rtSrc);
		clip_rect(src, rtSrc, rtThis);

		const LONG w = _min(wabc::width(rtSrc), wabc::width(rtThis));
		LONG h = _min(wabc::height(rtSrc), wabc::height(rtThis));

		if (w > 0 && h > 0)
		{
			const color_type *pSrc = src.m_pBits + src.address(rtSrc.left, rtSrc.top);
			color_type *pDest = m_pBits + address(rtThis.left, rtThis.top);

			const uint beta = 255 - nAlphaValue;
			LONG i;

			__declspec(align(16)) __m128i mm0, mm1, mm2, mm3, zero = _mm_setzero_si128();

			mm2 = _mm_set1_epi32(size_t(nAlphaValue));
			mm3 = _mm_set1_epi32(size_t(beta));

			for (; h > 0; --h)
			{
				for (i = 0; i < w; ++i)
				{
					const color_type &src_clr = pSrc[i];
					color_type &dest_clr = pDest[i];

					if (clrTrans != src_clr)
					{
						//const DWORD r = src_clr.r * nAlphaValue + dest_clr.r * beta;
						//const DWORD g = src_clr.g * nAlphaValue + dest_clr.g * beta;
						//const DWORD b = src_clr.b * nAlphaValue + dest_clr.b * beta;
						//
						//dest_clr.r = uint8(r  / 256);
						//dest_clr.g = uint8(g / 256);
						//dest_clr.b = uint8(b / 256);

						mm0 = _mm_set1_epi32(src_clr.m_bgra);
						mm1 = _mm_set1_epi32(dest_clr.m_bgra);

						mm0 = _mm_unpackhi_epi8(mm0, zero);
						mm0 = _mm_unpackhi_epi16(mm0, zero);

						mm1 = _mm_unpackhi_epi8(mm1, zero);
						mm1 = _mm_unpackhi_epi16(mm1, zero);

						mm0 = _mm_madd_epi16(mm0, mm2);
						mm1 = _mm_madd_epi16(mm1, mm3);

						mm0 = _mm_add_epi32(mm0, mm1);
						mm0 = _mm_srli_epi32(mm0, 8);
						mm0 = _mm_packs_epi32(mm0, zero);
						mm0 = _mm_packus_epi16(mm0, zero);
						dest_clr.m_bgra = mm0.m128i_i32[0];
					}
				}
				pSrc += src.pitch();
				pDest += pitch();
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::copy(const RECT &rt, const bitmap32 &src,
		LONG src_x, LONG src_y)
	{
		if (this != &src)
		{
			assert(m_hBitmap && src.m_hBitmap);
			RECT rtThis = rt;
			RECT rtSrc = { src_x, src_y, src_x + wabc::width(rt), src_y + wabc::height(rt) };
			clip_rect(*this, rtThis, rtSrc);
			clip_rect(src, rtSrc, rtThis);

			const LONG w = _min(wabc::width(rtSrc), wabc::width(rtThis));
			LONG h = _min(wabc::height(rtSrc), wabc::height(rtThis));

			if (w > 0 && h > 0)
			{
				const color_type *pSrc = src.m_pBits + src.address(rtSrc.left, rtSrc.top);
				color_type *pDest = m_pBits + address(rtThis.left, rtThis.top);

				for (; h > 0; --h)
				{
					::memcpy(pDest, pSrc, w * sizeof(color_type));
					pSrc += src.pitch();
					pDest += pitch();
				}
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::transcopy(const RECT &rt, const bitmap32 &src,
		LONG src_x, LONG src_y, color_type clrTrans)
	{
		assert(m_hBitmap && src.m_hBitmap);
		RECT rtThis = rt;
		RECT rtSrc = { src_x, src_y, src_x + wabc::width(rt), src_y + wabc::height(rt) };
		clip_rect(*this, rtThis, rtSrc);
		clip_rect(src, rtSrc, rtThis);

		const LONG w = _min(wabc::width(rtSrc), wabc::width(rtThis));
		LONG h = _min(wabc::height(rtSrc), wabc::height(rtThis)), i;

		if (w > 0 && h > 0)
		{
			const color_type *pSrc = src.m_pBits + src.address(rtSrc.left, rtSrc.top);
			color_type *pDest = m_pBits + address(rtThis.left, rtThis.top);

			for (; h > 0; --h, pSrc += src.pitch(), pDest += pitch())
			{
				for (i = 0; i < w; ++i)
				{
					if (pSrc[i] != clrTrans)
						pDest[i] = pSrc[i];
				}
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::alpha(uint8 nAlphaValue, color_type clr)
	{
		if (nAlphaValue == 0)
			return;
		if (nAlphaValue == 0xFF)
		{
			fill(clr);
			return;
		}

		const uint beta= 255 - nAlphaValue;
		//const DWORD r= clr.r * nAlphaValue;
		//const DWORD g= clr.g * nAlphaValue;
		//const DWORD b= clr.b * nAlphaValue;

		color_type *first= m_pBits;
		color_type *last= m_pBits + m_count_of_pixels;

		__declspec(align(16)) __m128i mm0, mm1, mm3, zero = _mm_setzero_si128();

		mm3 = _mm_set1_epi32(size_t(nAlphaValue));

		mm0 = _mm_set1_epi32(clr.m_bgra);
		mm0 = _mm_unpackhi_epi8(mm0, zero);
		mm0 = _mm_unpackhi_epi16(mm0, zero);
		mm0 = _mm_madd_epi16(mm0, mm3);

		mm3 = _mm_set1_epi32(size_t(beta));
		for (; first != last; ++first)
		{
			color_type &clr= *first;

			//clr.r= uint8((r + clr.r * beta) / 256);
			//clr.g= uint8((g + clr.g * beta) / 256);
			//clr.b= uint8((b + clr.b * beta) / 256);

			mm1 = _mm_set1_epi32(clr.m_bgra);
			mm1 = _mm_unpackhi_epi8(mm1, zero);
			mm1 = _mm_unpackhi_epi16(mm1, zero);

			mm1 = _mm_madd_epi16(mm1, mm3);
			mm1 = _mm_add_epi32(mm0, mm1);

			mm1 = _mm_srli_epi32(mm1, 8);
			mm1 = _mm_packs_epi32(mm1, zero);
			mm1 = _mm_packus_epi16(mm1, zero);

			clr.m_bgra = mm1.m128i_i32[0];
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::alpha(uint8 nAlphaValue, color_type clr, const RECT &rtA)
	{
		if (nAlphaValue == 0)
			return;

		if (nAlphaValue == 0xFF)
		{
			fill(rtA, clr);
			return;
		}
		const uint beta= 255 - nAlphaValue;
		//const DWORD r= clr.r * nAlphaValue;
		//const DWORD g= clr.g * nAlphaValue;
		//const DWORD b= clr.b * nAlphaValue;

		rect rt(0,0,width(), height());
		rt&= static_cast<const rect &>(rtA);

		const LONG w= _max(rt.width(), LONG(0));
		const LONG h= _max(rt.height(),LONG(0));

		if (w > 0 && h > 0)
		{
			const LONG nPitch = pitch();
			color_type *p = m_pBits + address(rt.left, rt.top);
			int i, j;

			__declspec(align(16)) __m128i mm0, mm1, mm3, zero = _mm_setzero_si128();

			mm3 = _mm_set1_epi32(size_t(nAlphaValue));

			mm0 = _mm_set1_epi32(clr.m_bgra);
			mm0 = _mm_unpackhi_epi8(mm0, zero);
			mm0 = _mm_unpackhi_epi16(mm0, zero);
			mm0 = _mm_madd_epi16(mm0, mm3);

			mm3 = _mm_set1_epi32(size_t(beta));
			for (i = 0; i < h; ++i, p += nPitch)
			{
				for (j = 0; j < w; ++j)
				{
					color_type &clr = p[j];

					//clr.r = uint8((r + clr.r * beta) / 256);
					//clr.g = uint8((g + clr.g * beta) / 256);
					//clr.b = uint8((b + clr.b * beta) / 256);

					mm1 = _mm_set1_epi32(clr.m_bgra);
					mm1 = _mm_unpackhi_epi8(mm1, zero);
					mm1 = _mm_unpackhi_epi16(mm1, zero);

					mm1 = _mm_madd_epi16(mm1, mm3);
					mm1 = _mm_add_epi32(mm0, mm1);

					mm1 = _mm_srli_epi32(mm1, 8);
					mm1 = _mm_packs_epi32(mm1, zero);
					mm1 = _mm_packus_epi16(mm1, zero);

					clr.m_bgra = mm1.m128i_i32[0];
				}
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::transalpha(uint8 nAlphaValue, color_type clr, color_type clrTrans)
	{
		if (nAlphaValue == 0)
			return;

		color_type *first = m_pBits;
		color_type *last = m_pBits + m_count_of_pixels;

		if (nAlphaValue == 0xFF)
		{
			for (; first != last; ++first)
			{
				if (*first != clrTrans)
					*first = clr;
			}
		}
		else
		{
			const uint beta = 255 - nAlphaValue;
			//const DWORD r = clr.r * nAlphaValue;
			//const DWORD g = clr.g * nAlphaValue;
			//const DWORD b = clr.b * nAlphaValue;

			__declspec(align(16)) __m128i mm0, mm1, mm3, zero = _mm_setzero_si128();

			mm3 = _mm_set1_epi32(size_t(nAlphaValue));

			mm0 = _mm_set1_epi32(clr.m_bgra);
			mm0 = _mm_unpackhi_epi8(mm0, zero);
			mm0 = _mm_unpackhi_epi16(mm0, zero);
			mm0 = _mm_madd_epi16(mm0, mm3);

			mm3 = _mm_set1_epi32(size_t(beta));

			for (; first != last; ++first)
			{
				color_type &clr = *first;
				if (clr != clrTrans)
				{
					//clr.r = uint8((r + clr.r * beta) / 256);
					//clr.g = uint8((g + clr.g * beta) / 256);
					//clr.b = uint8((b + clr.b * beta) / 256);

					mm1 = _mm_set1_epi32(clr.m_bgra);
					mm1 = _mm_unpackhi_epi8(mm1, zero);
					mm1 = _mm_unpackhi_epi16(mm1, zero);

					mm1 = _mm_madd_epi16(mm1, mm3);
					mm1 = _mm_add_epi32(mm0, mm1);

					mm1 = _mm_srli_epi32(mm1, 8);
					mm1 = _mm_packs_epi32(mm1, zero);
					mm1 = _mm_packus_epi16(mm1, zero);

					clr.m_bgra = mm1.m128i_i32[0];
				}
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::transalpha(uint8 nAlphaValue, color_type clr, 
		const RECT &rtA, color_type clrTrans)
	{
		const uint beta= 255 - nAlphaValue;
		//const DWORD r= clr.r * nAlphaValue;
		//const DWORD g= clr.g * nAlphaValue;
		//const DWORD b= clr.b * nAlphaValue;

		rect rt(0,0,width(), height());
		rt&= rtA;

		const LONG w= rt.width();
		const LONG h= rt.height();
		
		const LONG nPitch= pitch();
		color_type *p= m_pBits + address(rt.left,rt.top);
		int i,j;

		__declspec(align(16)) __m128i mm0, mm1, mm3, zero = _mm_setzero_si128();

		mm3 = _mm_set1_epi32(size_t(nAlphaValue));

		mm0 = _mm_set1_epi32(clr.m_bgra);
		mm0 = _mm_unpackhi_epi8(mm0, zero);
		mm0 = _mm_unpackhi_epi16(mm0, zero);
		mm0 = _mm_madd_epi16(mm0, mm3);

		mm3 = _mm_set1_epi32(size_t(beta));

		for( i= 0; i < h; ++i, p+= nPitch )
		{
			for(j= 0; j < w; ++j)
			{
				color_type &clr= p[j];

				if ( clr != clrTrans )
				{
					//clr.r= uint8((r + clr.r * beta) / 256);
					//clr.g= uint8((g + clr.g * beta) / 256);
					//clr.b= uint8((b + clr.b * beta) / 256);

					mm1 = _mm_set1_epi32(clr.m_bgra);
					mm1 = _mm_unpackhi_epi8(mm1, zero);
					mm1 = _mm_unpackhi_epi16(mm1, zero);

					mm1 = _mm_madd_epi16(mm1, mm3);
					mm1 = _mm_add_epi32(mm0, mm1);

					mm1 = _mm_srli_epi32(mm1, 8);
					mm1 = _mm_packs_epi32(mm1, zero);
					mm1 = _mm_packus_epi16(mm1, zero);

					clr.m_bgra = mm1.m128i_i32[0];
				}
			}
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::fill(size_t pos, size_t count, color_type clr)
	{
		assert((pos + count) <= m_count_of_pixels);
		color_type *p = m_pBits + pos;
//		for (; count > 0; --count, ++p)
//			*p = clr;
		_asm
		{
			mov	edi, p
			mov eax, clr
			mov	ecx, count
			cld
			rep	stosd
		}
	}

	// --------------------------------------------------------------------

	void bitmap32::fill(const RECT &rtDest, color_type clr)
	{
		rect rt(0, 0, width(), height());
		rt &= rtDest;

		const LONG w = rt.width();
		const LONG h = rt.height();
		if (w > 0 && h > 0)
		{
			const LONG nPitch = pitch();
			color_type *p = m_pBits + address(rt.left, rt.top);
			int i;// , j;

			for (i = 0; i < h; ++i, p += nPitch)
			{
				//for (j = 0; j < w; ++j)
				//	p[j] = clr;	// FIXME -???
				_asm
				{
					mov	edi, p
					mov eax, clr
					mov	ecx, w
					cld
					rep	stosd
				}
			}
		}
	}

	// --------------------------------------------------------------------
	// bitmap1
	// --------------------------------------------------------------------

	bool bitmap1::create(LONG w, LONG h)
	{
		assert(!m_hBitmap);

		// --------------------------------------------------------------------

		struct monochrome_header : BITMAPINFOHEADER
		{
			RGBQUAD	bmiColors[2];

			monochrome_header(LONG cx, LONG cy)
			{
				::memset(this, 0, sizeof(*this));
				this->biSize = sizeof(BITMAPINFOHEADER);
				this->biWidth = cx;
				this->biHeight = -cy;
				this->biPlanes = 1;
				this->biBitCount = 1;	// monochrome bitmap
//				this->biClrUsed = 2;

				bmiColors[1].rgbBlue = bmiColors[1].rgbGreen = bmiColors[1].rgbRed = 0xFF;
			}
		}bmph(w, h);

		cdc hdc;
		HBITMAP hBitmap = ::CreateDIBSection(hdc, (const BITMAPINFO *)&bmph, DIB_RGB_COLORS,
			(void **)&m_pBits, 0, 0);

		//wabc::api_check( m_hBitmap != 0, "CreateDIBSection" );

		if (hBitmap)
		{
			m_hBitmap = hBitmap;
			m_width = w;
			m_height = h;

			m_count_of_pixels = pitch() * h;
			return true;
		}
		return false;
	}

	// --------------------------------------------------------------------

	void bitmap1::destroy()
	{
		if (m_hBitmap != 0)
		{
			::DeleteObject(m_hBitmap);
			::memset(this, 0, sizeof(*this));
		}
	}
	
	// --------------------------------------------------------------------
	// dcclass
	// --------------------------------------------------------------------

	dcclass::dcclass(bool is_create_mem_dc)
	{
		uint8 *first = (uint8*)&m_hdc;
		uint8 *last = ((uint8*)&m_object_capacity) + sizeof(m_object_capacity);
		::memset(first, 0, last - first);
		if (is_create_mem_dc)
		{
			HDC h = ::GetWindowDC(0);
			m_hdc = ::CreateCompatibleDC(h);
			::ReleaseDC(0, h);
			m_dc_type = 1;
		}
	}

	// --------------------------------------------------------------------

	dcclass::dcclass(bool is_create_mem_dc, HDC h) 
	{
		uint8 *first = (uint8*)&m_hdc;
		uint8 *last = ((uint8*)&m_object_capacity) + sizeof(m_object_capacity);
		::memset(first, 0, last - first);
		if (is_create_mem_dc)
		{
			m_hdc = ::CreateCompatibleDC(h);
			m_dc_type = 1;
		}
		else
			m_hdc = h;
	}

	// --------------------------------------------------------------------

	dcclass::dcclass(HWND hWnd, bool is_create_window_dc) 
	{
		uint8 *first = (uint8*)&m_hdc;
		uint8 *last = ((uint8*)&m_object_capacity) + sizeof(m_object_capacity);
		::memset(first, 0, last - first);

		if (is_create_window_dc)
		{
			m_hdc = ::GetWindowDC(m_hWnd);
		}
		else
			m_hdc = ::GetDC(hWnd);

		if (hWnd == 0)
			m_dc_type = 2;
		else
			m_hWnd = hWnd;
	}

	// --------------------------------------------------------------------

	dcclass::~dcclass()
	{
		discard_all();
		if (m_dc_type == 1)
			::DeleteDC(m_hdc);
		else if (m_dc_type == 2)
			::ReleaseDC(0, m_hdc);
		else if (m_hWnd)
			::ReleaseDC(m_hWnd, m_hdc);

		if (m_objects)
			::free(m_objects);
	}

	// --------------------------------------------------------------------
	
	void dcclass::use(HANDLE h)
	{
		if (m_object_count >= m_object_capacity)
		{
			m_object_capacity = (m_object_capacity + 1) * 2;
			m_objects = (HGDIOBJ*)::realloc(m_objects, m_object_capacity * sizeof(m_objects[0]));
		}
		m_objects[m_object_count++] = ::SelectObject(m_hdc, h);
	}

	// --------------------------------------------------------------------

	void dcclass::discard_all()
	{
		while (m_object_count > 0)
			::SelectObject(m_hdc, m_objects[--m_object_count]);
	}

	// --------------------------------------------------------------------

	void dcclass::discard()
	{
		if (m_object_count > 0)
			::SelectObject(m_hdc, m_objects[--m_object_count]);
	}

	// --------------------------------------------------------------------

	void dcclass::discard(size_t n)
	{
		if (n >= m_object_count)
			discard_all();
		else
		{
			for (; n > 0; --n)
				::SelectObject(m_hdc, m_objects[--m_object_count]);
		}
	}

	// --------------------------------------------------------------------

	void dcclass::v2w(POINT *p, size_t n, bool bNoZoon)const
	{
		const POINT worg = this->window_org();
		const POINT vorg = this->viewport_org();

		size_t i = 0;
		if (!bNoZoon)
		{
			const POINT wext = this->window_ext();
			const POINT vext = this->viewport_ext();

			if (wext.x != vext.x || wext.y != vext.y)
			{
				for (; i < n; ++i)
				{
					POINT &pt = p[i];
					pt.x = (pt.x - vorg.x) * wext.x / vext.x + worg.x;
					pt.y = (pt.y - vorg.y) * wext.y / vext.y + worg.y;
				}
				return;
			}
		}

		for (; i < n; ++i)
		{
			POINT &pt = p[i];
			pt.x = (pt.x - vorg.x) + worg.x;
			pt.y = (pt.y - vorg.y) + worg.y;
		}
	}

	// --------------------------------------------------------------------

	void dcclass::v2w(LONG &x, LONG &y, bool bNoZoon)const
	{
		const POINT worg = this->window_org();
		const POINT vorg = this->viewport_org();

		if (!bNoZoon)
		{
			const POINT wext = this->window_ext();
			const POINT vext = this->viewport_ext();
			if (wext.x != vext.x || wext.y != vext.y)
			{
				x = (x - vorg.x) * wext.x / vext.x + worg.x;
				y = (y - vorg.y) * wext.y / vext.y + worg.y;
				return;
			}
		}
		x = (x - vorg.x) + worg.x;
		y = (y - vorg.y) + worg.y;
	}

	// --------------------------------------------------------------------

	void dcclass::w2v(POINT *p, size_t n, bool bNoZoon)const
	{
		const POINT worg = this->window_org();
		const POINT vorg = this->viewport_org();

		size_t i = 0;
		if (!bNoZoon)
		{
			const POINT wext = this->window_ext();
			const POINT vext = this->viewport_ext();

			if (wext.x != vext.x || wext.y != vext.y)
			{
				for (; i < n; ++i)
				{
					POINT &pt = p[i];
					pt.x = (pt.x - worg.x) * vext.x / wext.x + vorg.x;
					pt.y = (pt.y - worg.y) * vext.y / wext.y + vorg.y;
				}
				return;
			}
		}

		for (; i < n; ++i)
		{
			POINT &pt = p[i];
			pt.x = (pt.x - worg.x) + vorg.x;
			pt.y = (pt.y - worg.y) + vorg.y;
		}
	}

	// --------------------------------------------------------------------

	void dcclass::w2v(LONG &x, LONG &y, bool bNoZoon)const
	{
		const POINT worg = this->window_org();
		const POINT vorg = this->viewport_org();

		if (!bNoZoon)
		{
			const POINT wext = this->window_ext();
			const POINT vext = this->viewport_ext();

			if (wext.x != vext.x || wext.y != vext.y)
			{
				x = (x - worg.x) * vext.x / wext.x + vorg.x;
				y = (y - worg.y) * vext.y / wext.y + vorg.y;
				return;
			}
		}

		x = (x - worg.x) + vorg.x;
		y = (y - worg.y) + vorg.y;
	}

	// --------------------------------------------------------------------

	void dcclass::line(const RECT &rt)
	{
		::MoveToEx(m_hdc, rt.left, rt.top, 0);
		::LineTo(m_hdc, rt.right - 1, rt.top);
		::LineTo(m_hdc, rt.right - 1, rt.bottom - 1);
		::LineTo(m_hdc, rt.left, rt.bottom - 1);
		::LineTo(m_hdc, rt.left, rt.top);
	}

	// --------------------------------------------------------------------
	// font
	// --------------------------------------------------------------------

	bool font::create(int nHeight, const wchar_t * lpszFaceName, int nCharset)
	{
		LOGFONT lf;
		::memset(&lf, 0, sizeof(lf));
		lf.lfCharSet = nCharset;
		lf.lfHeight = nHeight;
		lf.lfWeight = FW_NORMAL;
		lf.lfOutPrecision = OUT_TT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;

		const size_t n1 = size_t(LF_FACESIZE - 1);
		const size_t n2 = ::wcslen(lpszFaceName);
		const size_t n = min(n1, n2);

		::memcpy(lf.lfFaceName, lpszFaceName, n * sizeof(string::value_type));
		lf.lfFaceName[n] = 0;
		return create(lf);
	}

	// --------------------------------------------------------------------

	bool font::choose_dialog(HWND hWnd)
	{
		CHOOSEFONT cf;
		LOGFONT lf;

		::memset(&cf, 0, sizeof(cf));

		cf.lStructSize = sizeof(CHOOSEFONT);
		cf.hwndOwner = hWnd;
		cf.lpLogFont = &lf;
		cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
		if (::ChooseFont(&cf) == TRUE)
			return create(lf);
		return false;
	}

	// --------------------------------------------------------------------
	// drawtxt
	// --------------------------------------------------------------------

	drawtxt::drawtxt(dcclass &dc):m_gc(dc)
	{
		uint8 *first= (uint8*)&m_rect;
		uint8 *last = ((uint8*)&m_width_of_chars) + sizeof(m_width_of_chars);
		::memset(first, 0, last - first);
	}

	// --------------------------------------------------------------------

	drawtxt::~drawtxt()
	{
		::free(m_width_of_chars);
	}

	// --------------------------------------------------------------------

	drawtxt & drawtxt::put(string::const_pointer lpszText, size_t n, 
		const RECT &rtBound, size_t flags)
	{
		const LONG nBoundWidth= rtBound.right - rtBound.left;
		size_t i;

		m_text= lpszText;
		m_text_size= n;
		m_rect= rtBound;
		m_flags= flags;
		m_visible_width= 0;
		m_visible_size= 0; 

		m_width_of_chars= (int *)::realloc(m_width_of_chars, n * sizeof(m_width_of_chars[0]));

		const TEXTMETRIC tm = m_gc.text_metrics();
			
		if ( m_flags & align_bottom )
			m_rect.top= m_rect.bottom - tm.tmHeight;
		else if ( m_flags & align_vcenter )
			m_rect.top= (m_rect.top + m_rect.bottom - tm.tmHeight) / 2;

		m_rect.bottom= m_rect.top + tm.tmHeight;

		if(n == 0)
		{
			m_rect.left= m_rect.right= 0;
			return *this;
		}

		// get the chararecter's width
		
		while(true)
		{
			const UINT ch= m_text[m_visible_size];
			const LONG w= m_gc.char_width(ch);
			m_visible_width+= w;

			if(m_visible_width >= nBoundWidth)
			{
				if(m_visible_width > nBoundWidth && m_visible_size > 0)
					m_visible_width-= w;
				else
					m_width_of_chars[m_visible_size++] = w;
				break;
			}

			m_width_of_chars[m_visible_size++] = w;

			if(m_visible_size >= n)
				break;
		}

		m_width_of_dot3= 0;
		if (m_visible_size < n && (m_flags & flag_3dot))
		{
			LONG w= nBoundWidth - m_visible_width;
			if(w >= 0)
			{
				// 末尾添加"..."
				m_width_of_dot3= m_gc.char_width(UINT('.')) * 3;
				const LONG &width_of_dot= m_width_of_dot3;

				while(m_visible_size > 0)
				{
					if(w >= width_of_dot)
						break;
					w+= m_width_of_chars[--m_visible_size];
					m_visible_width-= m_width_of_chars[m_visible_size];
				}

				if(m_visible_size == 0)
				{
					// 强行显示至少一个字符
					assert(m_visible_width == 0);
					m_visible_width= m_width_of_chars[0];
					m_visible_size= 1;
				}
				assert(m_visible_width > 0);
				m_visible_width+= width_of_dot;
			}
		}

		assert(m_visible_width > 0);
		assert(m_visible_size > 0);

		if(m_visible_width >= nBoundWidth)
			return *this;

		if(m_flags & align_right)
		{
			m_rect.left+= (nBoundWidth - m_visible_width);
			return *this;
		}

		if(m_flags & align_center)
		{
			m_rect.left+= (nBoundWidth - m_visible_width) / 2;
			m_rect.right= m_rect.left + m_visible_width;
			return *this;
		}

		if (m_flags & align_adjust)
		{
			size_t n = m_visible_size;
			if (m_visible_size == m_text_size)
				--n;

			assert(n < m_text_size);

			LONG w = nBoundWidth - m_visible_width;
			while (w > 0)
			{
				for (i = 0; i < n; ++i)
				{
					const uint ch = m_text[i];
					if (ch == 0x20 || ch > 127)
					{
						if (w == 0)
							return *this;
						--w;
						m_width_of_chars[i]++;
					}
				}

				for (i = 0; i < n; ++i)
				{
					if (w == 0)
						return *this;
					--w;
					m_width_of_chars[i]++;
				}
			}
		}
		else if (m_flags & align_evenly_spaced)
		{
			LONG w = nBoundWidth - m_visible_width;
			LONG n1 = w / (m_visible_size + 1);
			LONG n2 = w - n1 * (m_visible_size + 1);

			m_rect.left += n1;
			for (i = 0; i < m_visible_size; ++i)
				m_width_of_chars[i] += n1;

			while (n2 > 0)
			{
				for (i = 0; i < m_visible_size; ++i)
				{
					m_width_of_chars[i]++;
					if (--n2 == 0)
						return *this;
				}
				assert(n2 > 0);
				m_rect.left++;
				--n2;
			}
		}
		else
			m_rect.right = m_rect.left + m_visible_width;
		return *this;
	}

	// --------------------------------------------------------------------

	void drawtxt::draw(LONG xoffset, LONG yoffset)
	{
		if(m_visible_size == 0)
			return;

		const LONG x= m_rect.left + xoffset;
		const LONG y= m_rect.top + yoffset;
		if(m_visible_width <= (m_rect.right - m_rect.left))
		{
			::ExtTextOut(m_gc, x, y, 0, NULL, 
				m_text, m_visible_size , m_width_of_chars);
			if(m_visible_size < m_text_size && m_width_of_dot3 > 0)
				m_gc.text(x + m_visible_width - m_width_of_dot3, y, _T("..."), 3);
		}
		else if(m_rect.top < m_rect.bottom)
		{
			const LONG w= m_visible_width - (m_rect.right - m_rect.left);
			const LONG h= m_rect.bottom - m_rect.top;
			assert(w > 0);
			bitmap32 bmp_tmp(w, h);

			const LONG left= m_rect.right + xoffset;
			const LONG right= m_rect.left + xoffset + m_visible_width;

			if(m_gc.bmp32)
			{
				RECT rt= {left, y, right, m_rect.bottom + yoffset};
				m_gc.w2v(rt, true);

				bmp_tmp.copy(0, 0, w, h, *m_gc.bmp32, rt.left, rt.top);

				m_gc.text(x, y, m_text, 1);
				if(m_width_of_dot3 > 0)
					m_gc.text(x + m_width_of_chars[0], y, _T("..."), 3);

				m_gc.bmp32->copy(rt, bmp_tmp, 0, 0);
			}
			else
			{
				assert(false);
			}
		}
	}

	// --------------------------------------------------------------------
	// class pen
	// --------------------------------------------------------------------

	HPEN pen::create(DWORD clr, int nStyle, int nWidth)
	{
		LOGPEN temp;

		temp.lopnColor = clr;
		temp.lopnStyle = nStyle;
		temp.lopnWidth.x = nWidth;

		return create(temp);
	}

	// --------------------------------------------------------------------
	// region
	// --------------------------------------------------------------------

	region::region(const self & rhs) : m_hRgn(0)
	{
		if (rhs.m_hRgn)
		{
			m_hRgn = ::CreateRectRgn(0, 0, 0, 0);
			if (m_hRgn)
			{
				const int ret = ::CombineRgn(m_hRgn, rhs.m_hRgn, 0, RGN_COPY);
				if (ret == ERROR)
				{
					__wabc_trace(L"can't open combin region(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
				}
			}
			else
			{
				__wabc_trace(L"can't create region(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
			}
		}
	}

	// --------------------------------------------------------------------

	region::region( const region &rgn1, const region &rgn2, int nCombineMode )
	{
		m_hRgn= ::CreateRectRgn( 0, 0, 0, 0 );
		if (m_hRgn)
		{
			const int ret = ::CombineRgn(m_hRgn, rgn1.m_hRgn, rgn2.m_hRgn, nCombineMode);
			if (ret == ERROR)
			{
				__wabc_trace(L"can't open combin region(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
			}
		}
		else
		{
			__wabc_trace(L"can't create region(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
		}
	}

	// --------------------------------------------------------------------

	region::region(const POINT *pts, size_t n, int nMode)
	{
		m_hRgn= ::CreatePolygonRgn(pts, n, nMode);
		if (!m_hRgn)
		{
			__wabc_trace(L"can't create poly region(%S:%d): %08x", __FILE__, __LINE__, ::GetLastError());
		}
	}
}