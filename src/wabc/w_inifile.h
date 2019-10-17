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
#include <wchar.h>

namespace wabc
{
	// --------------------------------------------------------------------

	struct ini_file
	{
		string	m_file_name;

	public:

		typedef int		int_type;
		typedef DWORD	dword_type;
		typedef bool		bool_type;
		typedef float		float_type;
		typedef size_t		size_type;

		typedef string::value_type	char_type;

		typedef ini_file self;

	public:

		string read_str(string::const_pointer lpszSection, string::const_pointer lpszKey,
			string::const_pointer lpszDefault, size_type nValueSize = 255)const
		{
			string result(nValueSize, 0);

			const size_type n = ::GetPrivateProfileString(lpszSection, lpszKey, lpszDefault,
				&result[0], DWORD(nValueSize), m_file_name.c_str());
			result.resize(n);
			return result;
		}

		template< size_t N>
		size_t read_str(string::const_pointer lpszSection, string::const_pointer lpszKey,
			string::const_pointer lpszDefault, wchar_t(&result)[N])const
		{
			const size_type n = ::GetPrivateProfileString(lpszSection, lpszKey, lpszDefault,
				&result[0], DWORD(N), m_file_name.c_str());
			return n;
		}

		template<typename T1, typename T2, typename T3>
		string read_str(const T1 & strSection, const T2 & strKey,
			const T3 & strDefault, size_type nValueSize = 255)const
		{
			return read_str(&strSection[0], &strKey[0], &strDefault[0], nValueSize);
		}

		template<typename T1, typename T2>
		int_type read_int(const T1 & strSection, const T2 & strKey,
			int_type nDefault)const
		{
			char_type szNumber[32] = { 0 };
			const size_t n = read_str(&strSection[0], &strKey[0], &szNumber[0], szNumber);
			if (n == 0)
				return nDefault;

			return ::_wtoi(szNumber);
		}

		template<typename T1, typename T2>
		size_t read_uint(const T1 & strSection, const T2 & strKey,
			size_t nDefault)const
		{
			char_type szNumber[32] = { 0 };
			char_type *endptr=0;
			const size_t n = read_str(&strSection[0], &strKey[0], &szNumber[0], szNumber);
			if (n == 0)
				return nDefault;
			return wcstoul(szNumber, &endptr, 10);
		}

		template<typename T1, typename T2>
		bool_type read_bool(const T1 & strSection, const T2 & strKey,
			bool_type bDefault) const
		{
			return read_int(strSection, strKey, bDefault) != 0;
		}

		template<typename T1, typename T2>
		float_type read_float(const T1 & strSection, const T2 & strKey,
			float_type fDefault)const
		{
			char_type szNumber[128] = { 0 };
			const size_t n = read_str(&strSection[0], &strKey[0], &strDefault[0], szNumber);
			if (n == 0)
				return fDefault;
			return (float)::wcstod(szNumber, 0);
		}

		bool_type write_str(string::const_pointer lpszSection, string::const_pointer lpszKey,
			string::const_pointer lpszValue)
		{
			return ::WritePrivateProfileString(lpszSection, lpszKey, lpszValue,
				m_file_name.c_str()) != 0;
		}

		template<typename T1, typename T2, typename T3>
		bool_type write_str(const T1 & strSection, const T2 & strKey,
			const T3 & strValue)
		{
			return write_str(&strSection[0], &strKey[0], &strValue[0]);
		}

		template<typename T1, typename T2>
		bool_type write_int(const T1 & strSection, const T2 & strKey,
			int_type nValue)
		{
			wchar_t buf[32];
			swprintf_s(buf, L"%d", nValue);

			return write_str(&strSection[0], &strKey[0], buf);
		}

		template<typename T1, typename T2>
		bool_type write_uint(const T1 & strSection, const T2 & strKey,
			size_type nValue)
		{
			wchar_t buf[32];
			swprintf_s(buf, L"%u", nValue);
			return write_str(&strSection[0], &strKey[0], buf);
		}

		template<typename T1, typename T2>
		bool_type write_int(const T1 & strSection, const T2 & strKey,
			size_type nValue)
		{
			wchar_t buf[24];
			swprintf_s(buf, L"%u", nValue);
			return write_str(&strSection[0], &strKey[0], buf);
		}

		template<typename T1, typename T2>
		bool_type write_bool(const T1 & strSection, const T2 & strKey,
			bool_type bValue)
		{
			const char_type s[2] = { bValue ? '1' : '0', 0 };
			return write_str(&strSection[0], &strKey[0], s);
		}

		template<typename T1, typename T2>
		bool write_float(const T1 & strSection, const T2 & strKey,
			float_type fValue)
		{
			wchar_t buf[64];
			swprintf(buf, L"%f", fValue);
			return write_str(&strSection[0], &strKey[0], buf);
		}

	public:

		template<typename T>
		bool_type erase_section(const T & strSection)throw()
		{
			return ::WritePrivateProfileString(&strSection[0], 0, 0,
				m_file_name.c_str()) != 0;
		}

		template<typename T1, typename T2 >
		bool_type erase_key(const T1 & strSection,
			const T2 & strKey) throw()
		{
			return ::WritePrivateProfileString(&strSection[0],
				strKey.c_str(), 0, m_file_name.c_str()) != 0;
		}

	public:

		ini_file(){}

		explicit ini_file(string::const_pointer strFileName)
			: m_file_name(strFileName) {}
		explicit ini_file(const string & strFileName)
			: m_file_name(strFileName) {}

		void open(string::const_pointer strFileName)throw()
		{
			m_file_name = strFileName;
		}

		void open(const string & strFileName)throw()
		{
			m_file_name = strFileName;
		}

		const string & file_name()const throw()
		{
			return m_file_name;
		}

		void update()throw()
		{
			::WritePrivateProfileString(0, 0, 0, m_file_name.c_str());
		}
	};
}