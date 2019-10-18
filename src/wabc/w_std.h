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

#if(_MSC_VER <= 1400)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif
#endif

#include <windows.h>
#include <CommCtrl.h>
#include <TChar.h>
#include <string>
#include <assert.h>

template< bool b >
struct abc_static_assertion_failure;

template<>
struct abc_static_assertion_failure< true >{};

template< int N >
struct abc_static_assertion_test{};


#define __wabc_static_assert(b) typedef abc_static_assertion_test< \
	sizeof(abc_static_assertion_failure< (bool)(b) >) > static_assert_typedef##__LINE__

//#define countof(a) (sizeof(a)/sizeof((a)[0]))

template <typename T, size_t N>
char(&ArraySizeHelper(T(&array)[N]))[N];

#ifndef _MSC_VER
template <typename T, size_t N>
char(&ArraySizeHelper(const T(&array)[N]))[N];
#endif

#define countof(array) (sizeof(ArraySizeHelper(array)))

inline LONG int_atomic_inc(volatile LONG & x)
{
	return ::InterlockedIncrement(&x);
}

inline LONG int_atomic_dec(volatile LONG & x)
{
	return ::InterlockedDecrement(&x);
}

inline LONG int_atomic_get(volatile LONG & x)
{
	LONG result = 0;
	::InterlockedExchange(&result, x);
	return result;
}

// cas
inline LONG cmpxchg(volatile LONG *dest, const LONG & compare, const LONG & new_)
{
#if _MSC_VER >= 1300
	return ::InterlockedCompareExchange(dest, new_, compare);
#else
	return (LONG)::InterlockedCompareExchange((PVOID *)dest, (PVOID)(new_), (PVOID)(compare));
#endif
	//		if( *dest == compare)
	//		{  
	//			const LONG old= *dest;
	//			*dest= new_; 
	//			return old;
	//		}
	//		return *dest;
}

namespace wabc
{
	typedef std::wstring string;

	typedef unsigned char uint8;
	typedef unsigned short uint16;
	typedef unsigned int uint32;
	typedef unsigned int uint;
	typedef unsigned __int64 uint64;

	void * alloc(size_t new_size);
	void * realloc(void *p, size_t new_size);
	void free(void *);

//#define WABC_DECLARE_ATTR(thisAttr) \
//	union attr_t{ thisAttr }; \
//	attr_t * operator->() { return reinterpret_cast<attr_t *>(this); }\
//	const attr_t * operator->()const { return reinterpret_cast<const attr_t *>(this); }
//
//	// --------------------------------------------------------------------
//
//	template<typename T>
//	struct union_attr
//	{
//		T & me(){ return reinterpret_cast<T &>(*this); }
//		const T & me()const{ return reinterpret_cast<const T &>(*this); }
//	};

	// --------------------------------------------------------------------

	template<typename T>
	struct union_attr
	{
		T *_this;
	};

	// --------------------------------------------------------------------

	struct rect : RECT
	{
		rect(){}// { ::memset(this, 0, sizeof(*this)); }

		rect(LONG x1, LONG y1, LONG x2, LONG y2) 
		{
			left = x1; top = y1;
			right = x2; bottom = y2;
		}

		//rect(const rect &rhs)
		//{
		//	static_cast<RECT &>(*this) = rhs;
		//}

		// operator RECT &(){ return *this; }

		rect & operator=(const RECT &rhs)
		{
			static_cast<RECT &>(*this) = rhs;
			return *this;
		}

		rect & zero(){ ::memset(this, 0, sizeof(*this)); return *this; }

		rect & operator+=(LONG n);
		rect & operator-=(LONG n)
		{ return operator+=(-n); }

		bool operator==(const rect &rhs)const 
		{ return ::memcmp(this, &rhs, sizeof(rhs)) == 0; }

		bool operator!=(const rect &rhs)const 
		{ return ::memcmp(this, &rhs, sizeof(rhs)) != 0; }

		LONG height()const { return bottom - top; }
		LONG width()const { return right - left; }

		bool intersect(const rect &rhs)const ;

		bool intersect(LONG x, LONG y)const 
		{ return x >= left && x < right && y >= top && y < bottom; }

		bool intersect(const POINT &pt)const 
		{ return intersect(pt.x, pt.y); }

		void normalize();
		bool is_normal()const 
		{ return left <= right && top <= bottom; }

		rect & operator&=(const RECT &rhs);
		rect & operator|=(const RECT &rhs);

		rect & inflate(LONG n) ;
		rect & deflate(LONG n) ;

		rect & inflate(LONG nx, LONG ny) ;
		rect & deflate(LONG nx, LONG ny) ;

		rect & offset(LONG cx, LONG cy) ;
	};
	
	// --------------------------------------------------------------------

	struct layout
	{
		RECT	m_bound;

		explicit layout(HWND hWnd);
		explicit layout(const RECT &rt) :m_bound(rt){}
		layout(LONG left, LONG top, LONG right, LONG bottom);
		layout(LONG w, LONG h);

		layout & align_top(HWND hWnd);
		layout & align_bottom(HWND hWnd);
		layout & align_left(HWND hWnd);
		layout & align_right(HWND hWnd);
		layout & align_client(HWND hWnd);
	};

	// --------------------------------------------------------------------

	struct keyboard
	{
		uint8 key_state[256];

		keyboard()
		{
			::GetKeyboardState(key_state);
		}

		bool operator[](uint8 key_code)const
		{
			const uint8 mask = 0x80;
			return (key_state[key_code] & mask) != 0;
		}
	};

	// --------------------------------------------------------------------

	template<typename T>
	struct flag_template
	{
		typedef T value_type;
		T	value;

		operator const T & ()const { return value; }
		operator T(){ return value; }

		template< typename T1>
		flag_template & operator= (const T1 &rhs)
		{
			value = rhs;
			return  *this;
		}

		flag_template & operator|= (const T &f)
		{
			value |= f;
			return *this;
		}

		flag_template & operator&= (const T &f)
		{
			value &= f;
			return *this;
		}

		void set(const T &f)
		{
			value |= f;
		}

		void set(const T &f, const T &mask)
		{
			value &= ~mask;
			value |= f;
		}

		void unset(const T &f)
		{
			value &= ~f;
		}

		bool operator()(const T &f)const
		{
			const size_t v = value & f;
			return v != 0;
		}

		bool test(const T & f)const
		{
			const size_t v = value & f;
			return v != 0;
		}

		bool equal(const T & f)const
		{
			const size_t v = value & f;
			return v == f;
		}
	};

	typedef flag_template<size_t>	flag_t;

	// --------------------------------------------------------------------

	string last_error_to_str(DWORD dwError);
	void center_screen(HWND hWnd);

	void throw_win32_exception(const wchar_t *);

	// --------------------------------------------------------------------

	template< typename T>
	inline const T _max(const T& x, const T& y)
	{
		return (x < y) ? y : x;
	}

	template< typename T, typename Pred>
	inline const T _max(const T& x, const T& y, Pred pr)
	{
		return pr(x, y) ? y : x;
	}

	// --------------------------------------------------------------------

	template< typename T>
	inline const T _min(const T& x, const T& y)
	{
		return (x < y) ? x : y;
		//		return std::min(x,y); 
	}

	template< typename T1, typename T2>
	inline const T1 _min(const T1& x, const T2& y)
	{
		return (x < y) ? x : y;
		//		return std::min(x,y); 
	}

	template< typename T, typename Pred>
	inline const T _min(const T& x, const T& y, Pred pr)
	{
		return pr(x, y) ? x : y;
		//		return std::min(x,y,pr); 
	}

	// --------------------------------------------------------------------

	inline LONG width(const RECT &rt)
	{
		return rt.right - rt.left;
	}

	inline LONG height(const RECT &rt)
	{
		return rt.bottom - rt.top;
	}

#define __wabc_trace(format, ...)
}