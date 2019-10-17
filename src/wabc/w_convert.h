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

#include <windows.h>
#include <string>

#pragma warning( push )

#pragma warning( disable : 4996 )

namespace wabc
{
namespace detail
{
	typedef std::string string;
	typedef std::wstring wstring;

	// --------------------------------------------------------------------

	template<size_t CodePage, typename T>
	class codeconv
	{
	};

	// --------------------------------------------------------------------

	template<size_t CodePage>
	struct codeconv<CodePage, wstring>
	{
		wstring	result;

		codeconv & convert(const char *p, size_t n)
		{
			const size_t len= ::MultiByteToWideChar(CodePage, 0, p, int(n),0, 0);
			result.resize(len);
			const size_t ret= ::MultiByteToWideChar(CodePage, 0, p, int(n), &result[0], (int)len);
			return *this;
		}

		codeconv & operator=(const string &s)
		{
			return convert(s.c_str(), s.size());
		}

		codeconv & operator=(const char *p)
		{
			return convert(p, ::strlen(p));
		}

		operator string()const
		{
			typedef codeconv<CodePage, string> type;
			string s;
			__wabc_static_assert(sizeof(string) == sizeof(type));
			reinterpret_cast<type &>(s)= result;
			return s;
		}
	};

	// --------------------------------------------------------------------

	template<size_t CodePage, size_t N>
	struct codeconv<CodePage, wchar_t[N]>
	{
		wchar_t	result[N];

		size_t convert(const char *p, size_t n)
		{
			const size_t len= ::MultiByteToWideChar(CodePage, 0, p, int(n), result, N - 1);
			result[len]= 0;
			return len;
		}

		size_t operator=(const string &s)
		{
			return convert(s.c_str(), s.size());
		}

		size_t operator=(const char *p)
		{
			return convert(p, strlen(p));
		}

		operator string()const
		{
			typedef codeconv<CodePage, string> type;
			string s;
			__wabc_static_assert(sizeof(string) == sizeof(type));
			reinterpret_cast<type &>(s)= result;
			return s;
		}
	};

	// --------------------------------------------------------------------

	template<size_t CodePage>
	struct codeconv<CodePage, string>
	{
		string	result;

		codeconv & convert(const wchar_t *p, size_t n)
		{
			const size_t len= ::WideCharToMultiByte(CodePage,0, p, int(n), 0, 0, 0, 0);
			result.resize(len);
			const size_t ret= ::WideCharToMultiByte(CodePage,0, p, int(n), &result[0], (int)len, 0, 0);
			return *this;
		}

		codeconv & operator=(const wstring &s)
		{
			return convert(s.c_str(), s.size());
		}

		codeconv & operator=(const wchar_t *p)
		{
			return convert(p, wcslen(p));
		}

		operator wstring()const
		{
			typedef codeconv<CodePage, wstring> type;
			wstring s;
			__wabc_static_assert(sizeof(string) == sizeof(type));
			reinterpret_cast<type &>(s)= result;
			return s;
		}
	};

	// --------------------------------------------------------------------

	template<size_t CodePage, size_t N>
	struct codeconv<CodePage, char[N]>
	{
		char	result[N];

		size_t convert(const wchar_t *p, size_t n)
		{
			const size_t len= ::WideCharToMultiByte(CodePage,0, p, int(n), result, N - 1, 0, 0);
			result[len]= 0;
			return len;
		}

		size_t operator=(const wstring &s)
		{
			return convert(s.c_str(), s.size());
		}

		size_t operator=(const wchar_t *p)
		{
			return convert(p, wcslen(p));
		}

		operator wstring()const
		{
			typedef codeconv<CodePage, wstring> type;
			wstring s;
			__wabc_static_assert(sizeof(string) == sizeof(type));
			reinterpret_cast<type &>(s)= result;
			return s;
		}
	};

} // detail
	// --------------------------------------------------------------------

	template<typename T> inline
		detail::codeconv<CP_UTF8, T> & utf8(T &t)
	{
		typedef detail::codeconv<CP_UTF8, T> type;
		__wabc_static_assert(sizeof(T) == sizeof(type));
		return reinterpret_cast<type &>(t);
	}

	template<typename T> inline
		const detail::codeconv<CP_UTF8, T> & utf8(const T &t)
	{
		typedef detail::codeconv<CP_UTF8, T> type;
		__wabc_static_assert(sizeof(T) == sizeof(type));
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_UTF8, wchar_t[N]> & utf8(wchar_t (&t)[N])
	{
		typedef detail::codeconv<CP_UTF8, wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::codeconv<CP_UTF8, wchar_t[N]> & utf8(const wchar_t (&t)[N])
	{
		typedef detail::codeconv<CP_UTF8, wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_UTF8, wchar_t[N]> & utf8(wchar_t *t)
	{
			typedef detail::codeconv<CP_UTF8, wchar_t[N]> type;
			return reinterpret_cast<type &>(*t);
	}

	template<size_t N> inline
		detail::codeconv<CP_UTF8, char[N]> & utf8(char (&t)[N])
	{
		typedef detail::codeconv<CP_UTF8, char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::codeconv<CP_UTF8, char[N]> & utf8(const char (&t)[N])
	{
		typedef detail::codeconv<CP_UTF8, char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_UTF8, char[N]> & utf8(char *t)
	{
			typedef detail::codeconv<CP_UTF8, char[N]> type;
			return reinterpret_cast<type &>(*t);
	}

	// --------------------------------------------------------------------

	template<typename T> inline
		detail::codeconv<CP_UTF7, T> & utf7(T &t)
	{
		typedef detail::codeconv<CP_UTF7, T> type;
		__wabc_static_assert(sizeof(T) == sizeof(type));
		return reinterpret_cast<type &>(t);
	}

	template<typename T> inline
		const detail::codeconv<CP_UTF7, T> & utf7(const T &t)
	{
		typedef detail::codeconv<CP_UTF7, T> type;
		__wabc_static_assert(sizeof(T) == sizeof(type));
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_UTF7, wchar_t[N]> & utf7(wchar_t (&t)[N])
	{
		typedef detail::codeconv<CP_UTF7, wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::codeconv<CP_UTF7, wchar_t[N]> & utf7(const wchar_t (&t)[N])
	{
		typedef detail::codeconv<CP_UTF7, wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_UTF7, char[N]> & utf7(char (&t)[N])
	{
		typedef detail::codeconv<CP_UTF7, char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::codeconv<CP_UTF7, char[N]> & utf7(const char (&t)[N])
	{
		typedef detail::codeconv<CP_UTF7, char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<const type &>(t);
	}

	// --------------------------------------------------------------------

	template<typename T> inline
		detail::codeconv<CP_ACP, T> & acp(T &t)
	{
		typedef detail::codeconv<CP_ACP, T> type;
		__wabc_static_assert(sizeof(T) == sizeof(type));
		return reinterpret_cast<type &>(t);
	}

	template<typename T> inline
		const detail::codeconv<CP_ACP, T> & acp(const T &t)
	{
		typedef detail::codeconv<CP_ACP, T> type;
		__wabc_static_assert(sizeof(T) == sizeof(type));
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_ACP, wchar_t[N]> & acp(wchar_t(&t)[N])
	{
		typedef detail::codeconv<CP_ACP, wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::codeconv<CP_ACP, wchar_t[N]> & acp(const wchar_t(&t)[N])
	{
		typedef detail::codeconv<CP_ACP, wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::codeconv<CP_ACP, char[N]> & acp(char(&t)[N])
	{
		typedef detail::codeconv<CP_ACP, char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::codeconv<CP_ACP, char[N]> & acp(const char(&t)[N])
	{
		typedef detail::codeconv<CP_ACP, char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<const type &>(t);
	}

namespace detail
{
	// --------------------------------------------------------------------

	template<typename T>
	class intconv
	{
	};

	// --------------------------------------------------------------------

	template<typename T>
	struct inttostr
	{
		typedef T T;
		T	result;

		operator string()const
		{
			typedef intconv<string> type;
			string t;
			__wabc_static_assert(sizeof(string) == sizeof(type));
			reinterpret_cast<type &>(t)= result;
			return t;
		}

		operator wstring()const
		{
			typedef intconv<wstring> type;
			wstring t;
			__wabc_static_assert(sizeof(wstring) == sizeof(type));
			reinterpret_cast<type &>(t)= result;
			return t;
		}
	};

	// --------------------------------------------------------------------

	template<>
	struct intconv<int> : inttostr<int>
	{
		T & operator=(const char *p)
		{
			return result= (T)::atoi(p);
		}

		T & operator=(const string &s)
		{
			return result= (T)::atoi(s.c_str());
		}

		T & operator=(const wchar_t *p)
		{
			return result= (T)::_wtoi(p);
		}

		T & operator=(const wstring &s)
		{
			return result= ::_wtoi(s.c_str());
		}
	};

	template<>
	struct intconv<short int> : inttostr<short int>
	{
		T & operator=(const char *p)
		{
			return result= (T)::atoi(p);
		}

		T & operator=(const string &s)
		{
			return result= (T)::atoi(s.c_str());
		}

		T & operator=(const wchar_t *p)
		{
			return result= (T)::_wtoi(p);
		}

		T & operator=(const wstring &s)
		{
			return result= ::_wtoi(s.c_str());
		}
	};

	template<>
	struct intconv<char> : inttostr<char>
	{
		T & operator=(const char *p)
		{
			return result= (T)::atoi(p);
		}

		T & operator=(const string &s)
		{
			return result= (T)::atoi(s.c_str());
		}

		T & operator=(const wchar_t *p)
		{
			return result= (T)::_wtoi(p);
		}

		T & operator=(const wstring &s)
		{
			return result= ::_wtoi(s.c_str());
		}
	};

	template<>
	struct intconv<__int64> : inttostr<__int64>
	{
		T & operator=(const char *p)
		{
			return result= (T)::_atoi64(p);
		}

		T & operator=(const string &s)
		{
			return result= (T)::_atoi64(s.c_str());
		}

		T & operator=(const wchar_t *p)
		{
			return result= (T)::_wtoi64(p);
		}

		T & operator=(const wstring &s)
		{
			return result= ::_wtoi64(s.c_str());
		}
	};

	template<>
	struct intconv<unsigned int> : inttostr<unsigned int>
	{
		T & operator=(const char *p)
		{
			char *endptr;
			return result= (T)::strtoul(p, &endptr, 10);
		}

		T & operator=(const string &s)
		{
			char *endptr;
			return result= (T)::strtoul(s.c_str(), &endptr, 10);
		}

		T & operator=(const wchar_t *p)
		{
			wchar_t *endptr;
			return result= ::wcstoul(p, &endptr, 10);
		}

		T & operator=(const wstring &s)
		{
			wchar_t *endptr;
			return result= ::wcstoul(s.c_str(), &endptr, 10);
		}
	};

	template<>
	struct intconv<unsigned short> : inttostr<unsigned short>
	{
		T & operator=(const char *p)
		{
			char *endptr;
			return result= (T)::strtoul(p, &endptr, 10);
		}

		T & operator=(const string &s)
		{
			char *endptr;
			return result= (T)::strtoul(s.c_str(), &endptr, 10);
		}

		T & operator=(const wchar_t *p)
		{
			wchar_t *endptr;
			return result= (T)::wcstoul(p, &endptr, 10);
		}

		T & operator=(const wstring &s)
		{
			wchar_t *endptr;
			return result= (T)::wcstoul(s.c_str(), &endptr, 10);
		}
	};

	template<>
	struct intconv<unsigned char> : inttostr<unsigned char>
	{
		T & operator=(const char *p)
		{
			char *endptr;
			return result= (T)::strtoul(p, &endptr, 10);
		}

		T & operator=(const string &s)
		{
			char *endptr;
			return result= (T)::strtoul(s.c_str(), &endptr, 10);
		}

		T & operator=(const wchar_t *p)
		{
			wchar_t *endptr;
			return result= (T)::wcstoul(p, &endptr, 10);
		}

		T & operator=(const wstring &s)
		{
			wchar_t *endptr;
			return result= (T)::wcstoul(s.c_str(), &endptr, 10);
		}
	};

	template<>
	struct intconv<LPARAM> : inttostr<LPARAM>
	{
		T & operator=(const char *p)
		{
			return result= (T)::atoi(p);
		}

		T & operator=(const string &s)
		{
			return result= (T)::atoi(s.c_str());
		}

		T & operator=(const wchar_t *p)
		{
			return result= (T)::_wtoi(p);
		}

		T & operator=(const wstring &s)
		{
			return result= ::_wtoi(s.c_str());
		}
	};

	template<>
	struct intconv<unsigned __int64> : inttostr<unsigned __int64>
	{
		T & operator=(const char *p)
		{
			return result= (T)::_atoi64(p);
		}

		T & operator=(const string &s)
		{
			return result= (T)::_atoi64(s.c_str());
		}

		T & operator=(const wchar_t *p)
		{
			wchar_t *endptr;
			return result= ::_wcstoui64(p, &endptr, 10);
		}

		T & operator=(const wstring &s)
		{
			wchar_t *endptr;
			return result= ::_wcstoui64(s.c_str(), &endptr, 10);
		}
	};

	// --------------------------------------------------------------------

	template<size_t N>
	struct intconv<char[N]>
	{
		char result[N];

		size_t operator=(int n)
		{
			const size_t len= sprintf_s(result, "%d", n);
			return len;
		}

		size_t operator=(const __int64 &n)
		{
			const size_t len= sprintf_s(result, "%I64i", n);
			return len;
		}

		size_t operator=(unsigned int n)
		{
			const size_t len= sprintf_s(result, "%u", n);
			return len;
		}

		size_t operator=(const unsigned __int64 &n)
		{
			const size_t len= sprintf_s(result, "%I64u", n);
			return len;
		}

		template< typename T >
		T to_int()const
		{
			typedef intconv<T> type;
			T t;
			__wabc_static_assert(sizeof(T) == sizeof(type));
			return reinterpret_cast<type &>(t)= result;
		}

		operator char()const
		{
			return to_int<char>();
		}

		operator short()const
		{
			return to_int<short>();
		}

		operator int()const
		{
			return to_int<int>();
		}

		operator __int64()const
		{
			return to_int<__int64>();
		}

		operator unsigned char()const
		{
			return to_int<unsigned char>();
		}

		operator unsigned short()const
		{
			return to_int<unsigned short>();
		}

		operator unsigned int()const
		{
			return to_int<unsigned int>();
		}

		operator unsigned __int64()const
		{
			return to_int<unsigned __int64>();
		}
	};

	// --------------------------------------------------------------------

	template<>
	struct intconv<string>
	{
		string	result;

		string & operator=(int n)
		{
			result.resize(16);
			const size_t len= sprintf(&result[0], "%d", n);
			result.resize(len);
			return result;
		}

		string & operator=(const __int64 &n)
		{
			result.resize(64);
			const size_t len= sprintf(&result[0], "%I64i", n);
			result.resize(len);
			return result;
		}

		string & operator=(unsigned int n)
		{
			result.resize(16);
			const size_t len= sprintf(&result[0], "%u", n);
			result.resize(len);
			return result;
		}

		string & operator=(const unsigned __int64 &n)
		{
			result.resize(64);
			const size_t len= sprintf(&result[0], "%I64u", n);
			result.resize(len);
			return result;
		}

		template< typename T >
		T to_int()const
		{
			typedef intconv<T> type;
			T t;
			__wabc_static_assert(sizeof(T) == sizeof(type));
			return reinterpret_cast<type &>(t)= result;
		}

		operator char()const
		{
			return to_int<char>();
		}

		operator short()const
		{
			return to_int<short>();
		}

		operator int()const
		{
			return to_int<int>();
		}

		operator __int64()const
		{
			return to_int<__int64>();
		}

		operator unsigned char()const
		{
			return to_int<unsigned char>();
		}

		operator unsigned short()const
		{
			return to_int<unsigned short>();
		}

		operator unsigned int()const
		{
			return to_int<unsigned int>();
		}

		operator unsigned __int64()const
		{
			return to_int<unsigned __int64>();
		}
	};

	template<>
	struct intconv<const char *>
	{
		const char *result;

		template< typename T >
		T to_int()const
		{
			typedef intconv<T> type;
			T t;
			__wabc_static_assert(sizeof(T) == sizeof(type));
			return reinterpret_cast<type &>(t)= result;
		}

		operator char()const
		{
			return to_int<char>();
		}

		operator short()const
		{
			return to_int<short>();
		}

		operator int()const
		{
			return to_int<int>();
		}

		operator __int64()const
		{
			return to_int<__int64>();
		}

		operator unsigned char()const
		{
			return to_int<unsigned char>();
		}

		operator unsigned short()const
		{
			return to_int<unsigned short>();
		}

		operator unsigned int()const
		{
			return to_int<unsigned int>();
		}

		operator unsigned __int64()const
		{
			return to_int<unsigned __int64>();
		}
	};

	template<size_t N>
	struct intconv< wchar_t[N] >
	{
		wchar_t	result[N];

		size_t operator=(const int &n)
		{
			const size_t len= swprintf_s(result, L"%d", n);
			return len;
		}

		size_t operator=(const __int64 &n)
		{
			const size_t len= swprintf_s(result, L"%I64i", n);
			return len;
		}

		size_t operator=(const unsigned int &n)
		{
			const size_t len= swprintf_s(result, L"%u", n);
			return len;
		}

		size_t operator=(const unsigned __int64 &n)
		{
			const size_t len= swprintf_s(result, L"%I64u", n);
			return len;
		}

		template< typename T >
		T to_int()const
		{
			typedef intconv<T> type;
			T t;
			__wabc_static_assert(sizeof(T) == sizeof(type));
			return reinterpret_cast<type &>(t)= result;
		}

		operator char()const
		{
			return to_int<char>();
		}

		operator short()const
		{
			return to_int<short>();
		}

		operator int()const
		{
			return to_int<int>();
		}

		operator __int64()const
		{
			return to_int<__int64>();
		}

		operator unsigned char()const
		{
			return to_int<unsigned char>();
		}

		operator unsigned short()const
		{
			return to_int<unsigned short>();
		}

		operator unsigned int()const
		{
			return to_int<unsigned int>();
		}

		operator unsigned __int64()const
		{
			return to_int<unsigned __int64>();
		}
	};

	// --------------------------------------------------------------------

	template<>
	struct intconv<wstring>
	{
		wstring	result;

		wstring & operator=(const int &n)
		{
			result.resize(16);
			const size_t len= swprintf(&result[0], L"%d", n);
			result.resize(len);
			return result;
		}

		wstring & operator=(const __int64 &n)
		{
			result.resize(64);
			const size_t len= swprintf(&result[0], L"%I64i", n);
			result.resize(len);
			return result;
		}

		wstring & operator=(const unsigned int &n)
		{
			result.resize(16);
			const size_t len= swprintf(&result[0], L"%u", n);
			result.resize(len);
			return result;
		}

		wstring & operator=(const unsigned __int64 &n)
		{
			result.resize(64);
			const size_t len= swprintf(&result[0], L"%I64u", n);
			result.resize(len);
			return result;
		}

		template< typename T >
		T to_int()const
		{
			typedef intconv<T> type;
			T t;
			__wabc_static_assert(sizeof(T) == sizeof(type));
			return reinterpret_cast<type &>(t)= result;
		}

		operator char()const
		{
			return to_int<char>();
		}

		operator short()const
		{
			return to_int<short>();
		}

		operator int()const
		{
			return to_int<int>();
		}

		operator __int64()const
		{
			return to_int<__int64>();
		}

		operator unsigned char()const
		{
			return to_int<unsigned char>();
		}

		operator unsigned short()const
		{
			return to_int<unsigned short>();
		}

		operator unsigned int()const
		{
			return to_int<unsigned int>();
		}

		operator unsigned __int64()const
		{
			return to_int<unsigned __int64>();
		}
	};

	template<>
	struct intconv<const wchar_t *>
	{
		const wchar_t *result;

		template< typename T >
		T to_int()const
		{
			typedef intconv<T> type;
			T t;
			__wabc_static_assert(sizeof(T) == sizeof(type));
			return reinterpret_cast<type &>(t)= result;
		}

		operator char()const
		{
			return to_int<char>();
		}

		operator short()const
		{
			return to_int<short>();
		}

		operator int()const
		{
			return to_int<int>();
		}

		operator __int64()const
		{
			return to_int<__int64>();
		}

		operator unsigned char()const
		{
			return to_int<unsigned char>();
		}

		operator unsigned short()const
		{
			return to_int<unsigned short>();
		}

		operator unsigned int()const
		{
			return to_int<unsigned int>();
		}

		operator unsigned __int64()const
		{
			return to_int<unsigned __int64>();
		}
	};
}
	// --------------------------------------------------------------------

	template<typename T> inline
		detail::intconv<T> & int_(T &t)
	{
		typedef detail::intconv<T> type;
		__wabc_static_assert(sizeof(type) == sizeof(T));
		return reinterpret_cast<type &>(t);
	}

	template<typename T> inline
	const detail::intconv<T> & int_(const T &t)
	{
		typedef detail::intconv<T> type;
		__wabc_static_assert(sizeof(type) == sizeof(T));
		return reinterpret_cast<const type &>(t);
	}

	// --------------------------------------------------------------------

	template<size_t N> inline
		detail::intconv<char[N]> & int_(char (&t)[N])
	{
		typedef detail::intconv<char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::intconv<char[N]> & int_(const char (&t)[N])
	{
		typedef detail::intconv<char[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(char) * N);
		return reinterpret_cast<const type &>(t);
	}

	template<size_t N> inline
		detail::intconv<wchar_t[N]> & int_(wchar_t (&t)[N])
	{
		typedef detail::intconv<wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<type &>(t);
	}

	template<size_t N> inline
		const detail::intconv<wchar_t[N]> & int_(const wchar_t (&t)[N])
	{
		typedef detail::intconv<wchar_t[N]> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t) * N);
		return reinterpret_cast<const type &>(t);
	}

	// --------------------------------------------------------------------

	template<typename T> inline
		T int_(const detail::string &t)
	{
		typedef detail::intconv<detail::string> type;
		__wabc_static_assert(sizeof(type) == sizeof(detail::string));
		return reinterpret_cast<const type &>(t);
	}

	template<typename T> inline
		T int_(const detail::wstring &t)
	{
		typedef detail::intconv<detail::wstring> type;
		__wabc_static_assert(sizeof(type) == sizeof(detail::wstring));
		return reinterpret_cast<const type &>(t);
	}

	template<typename T> inline
		T int_(const char *t)
	{
		typedef detail::intconv<const char *> type;
		__wabc_static_assert(sizeof(type) == sizeof(char *));

		return reinterpret_cast<const type &>(t);
	}

	template<typename T> inline
		T int_(const wchar_t *t)
	{
		typedef detail::intconv<const wchar_t *> type;
		__wabc_static_assert(sizeof(type) == sizeof(wchar_t *));

		return reinterpret_cast<const type &>(t);
	}
};

#pragma warning( pop ) 