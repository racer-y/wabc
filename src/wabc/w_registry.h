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

namespace wabc
{
	// --------------------------------------------------------------------

	class registry
	{
		// --------------------------------------------------------------------

		struct union_property
		{
			HKEY	m_hKey;
		};

		// --------------------------------------------------------------------

		struct as_c_str_t : union_property
		{
			bool get(const TCHAR *value_name, TCHAR *buf, size_t cbBuf)const;

			template<size_t N>
			bool get(const TCHAR *value_name, TCHAR(&a)[N])const
			{
				return get(value_name, a, N);
			}

			bool operator()(const TCHAR *value_name, string &s)const;

			bool operator()(const TCHAR *value_name, TCHAR *buf, size_t cbBuf)const
			{
				return get(value_name, buf, cbBuf);
			}

			template<size_t N>
			bool operator()(const TCHAR *value_name, TCHAR(&a)[N])const
			{
				return get(value_name, a, N);
			}

			string operator()(const TCHAR *value_name, const TCHAR *def = _T(""))const
			{
				string s;
				if (!operator()(value_name, s))
					s = def;
				return s;
			}
		};

		// --------------------------------------------------------------------

		struct as_int_t : union_property
		{
			bool get(const TCHAR *value_name, int &result)const;
			bool set(const TCHAR *value_name, int result);

			bool operator()(const TCHAR *value_name, int &result)const
			{
				__wabc_static_assert(sizeof(int) == sizeof(DWORD));
				return get(value_name, result);
			}

			int operator()(const TCHAR *value_name, int def = 0)
			{
				int result;
				if (!get(value_name, result))
					result = def;
				return result;
			}
		};

		// --------------------------------------------------------------------

		struct as_uint_t : union_property
		{
			bool get(const TCHAR *value_name, unsigned int &result)const
			{
				return reinterpret_cast<const as_int_t&>(*this).get(value_name, (int &)result);
			}

			bool operator()(const TCHAR *value_name, unsigned int &result)const
			{
				__wabc_static_assert(sizeof(int) == sizeof(DWORD));
				return get(value_name, result);
			}

			unsigned int operator()(const TCHAR *value_name, unsigned int def = 0)
			{
				unsigned int result;
				if (!get(value_name, result))
					result = def;
				return result;
			}
		};

		// --------------------------------------------------------------------

		struct as_binary_t : union_property
		{
			size_t get(const TCHAR *value_name, void *buf, size_t bytes_of_buf)const;
			size_t set(const TCHAR *value_name, const void *buf, size_t bytes_of_buf);

			template<typename T>
			bool get(const TCHAR *value_name, T &result)
			{
				return get(value_name, &result, sizeof(T)) >= sizeof(T);
			}
		};

		// --------------------------------------------------------------------

		struct subkey_t : union_property
		{
			size_t get_name(size_t index, TCHAR *szName, DWORD cbName)const;
			size_t size()const;

			template<size_t N>
			size_t get_name(size_t index, TCHAR(&a)[N])
			{
				return get_name(index, a, N);
			}
		};

	public:

		union
		{
			HKEY	m_hKey;

			as_c_str_t	as_c_str;
			as_binary_t	as_binary;
			as_int_t	as_int;
			as_uint_t	as_uint;

			subkey_t	subkey;
		};

		registry() :m_hKey(0){}
		~registry()
		{
			if (m_hKey)
				::RegCloseKey(m_hKey);
		}

		bool open(HKEY hKey, const TCHAR *lpSubKey, REGSAM samDesired)
		{
			assert(!m_hKey);
			const LONG ret = RegOpenKeyEx(hKey, lpSubKey, 0, samDesired, &m_hKey);
			return ret == 0;
		}

		bool create(HKEY hKey, const TCHAR *lpSubKey, REGSAM samDesired)
		{
			assert(!m_hKey);
			DWORD dwDisposition;
			const LONG ret = RegCreateKeyEx(hKey, lpSubKey, 0, 0, 0, samDesired, 0, &m_hKey, &dwDisposition);
			return ret == 0;
		}

		void close()
		{
			if (m_hKey)
			{
				::RegCloseKey(m_hKey);
				m_hKey = 0;
			}
		}

		operator HKEY()const { return m_hKey; }
	};
}