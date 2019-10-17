#include "w_registry.h"

namespace wabc
{
	// --------------------------------------------------------------------
	// registry::as_c_str_t
	// --------------------------------------------------------------------

	bool registry::as_c_str_t::operator()(const TCHAR *value_name, string &s)const
	{
		const size_t n = s.capacity();
		s.resize(n);

		DWORD cb = (n + 1) * sizeof(value_name[0]);
		DWORD type = 0;

		while (RegQueryValueEx(m_hKey, value_name, 0, &type, (LPBYTE)&s[0], &cb) == ERROR_MORE_DATA)
		{
			s.resize((cb / sizeof(value_name[0])) - 1);
		}

		s.resize((cb / sizeof(value_name[0])) - 1);
		return type == REG_SZ;
	}

	// --------------------------------------------------------------------

	bool registry::as_c_str_t::get(const TCHAR *value_name, TCHAR *buf, size_t cbBuf)const
	{
		DWORD n = cbBuf * sizeof(buf[0]);
		DWORD type = 0;

		const LONG ret = RegQueryValueEx(m_hKey, value_name, 0, &type, LPBYTE(buf), &n);
		if (ret == ERROR_SUCCESS)
			return type == REG_SZ;
		return false;
	}

	// --------------------------------------------------------------------
	// registry::as_binary_t
	// --------------------------------------------------------------------

	size_t registry::as_binary_t::get(const TCHAR *value_name, void *buf, size_t bytes_of_buf)const
	{
		DWORD n = bytes_of_buf;
		DWORD type = 0;
		const LONG ret = RegQueryValueEx(m_hKey, value_name, 0, &type, LPBYTE(buf), &n);

		if (ret == ERROR_SUCCESS && type == REG_BINARY)
			return n;
		return 0;
	}

	// --------------------------------------------------------------------

	size_t registry::as_binary_t::set(const TCHAR *value_name, const void *buf, size_t bytes_of_buf)
	{
		const LONG ret = RegSetValueEx(m_hKey, value_name, 0, REG_BINARY, (BYTE*)buf, bytes_of_buf);
		return ret == ERROR_SUCCESS;
	}

	// --------------------------------------------------------------------
	// registry::as_int_t
	// --------------------------------------------------------------------

	bool registry::as_int_t::get(const TCHAR *value_name, int &result)const
	{
		__wabc_static_assert(sizeof(int) == sizeof(DWORD));

		DWORD n = sizeof(int);
		DWORD type = 0;
		const LONG ret = RegQueryValueEx(m_hKey, value_name, 0, &type, LPBYTE(&result), &n);

		if (ret == ERROR_SUCCESS && type == REG_DWORD)
			return true;
		return false;
	}

	// --------------------------------------------------------------------

	bool registry::as_int_t::set(const TCHAR *value_name, int result)
	{
		__wabc_static_assert(sizeof(int) == sizeof(DWORD));
		const LONG ret = RegSetValueEx(m_hKey, value_name, 0, REG_DWORD, (BYTE*)&result, sizeof(result));
		return ret == ERROR_SUCCESS;
	}

	// --------------------------------------------------------------------
	// registry::subkey_t
	// --------------------------------------------------------------------

	size_t registry::subkey_t::get_name(size_t index, TCHAR *szName, DWORD cbName)const
	{
		FILETIME ftLastWriteTime;
		const LONG retCode = RegEnumKeyEx(m_hKey, index,
			szName,
			&cbName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime);
		if (retCode == ERROR_SUCCESS)
			return cbName;
		return 0;
	}

	// --------------------------------------------------------------------

	size_t registry::subkey_t::size()const
	{
		// Get the class name and the value count. 
		DWORD    cSubKeys = 0;
		const LONG retCode = RegQueryInfoKey(
			m_hKey,                    // key handle 
			0,                // buffer for class name 
			0,           // size of class string 
			NULL,                    // reserved 
			&cSubKeys,               // number of subkeys 
			0,            // longest subkey size 
			0,            // longest class string 
			0,                // number of values for this key 
			0,            // longest value name 
			0,         // longest value data 
			0,   // security descriptor 
			0);       // last write time 
		return cSubKeys;
	}
}