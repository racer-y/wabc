#include "w_file.h"
#include <assert.h>

namespace wabc
{
	// --------------------------------------------------------------------
	// file
	// --------------------------------------------------------------------

	bool file::open(string::const_pointer lpszFileName,
		string::const_pointer mode, DWORD dwShare)throw()
	{
		DWORD dwAccess = 0, dwCreate = 0;
		bool bSeekEnd = false;

		assert(m_hFile == INVALID_HANDLE_VALUE);
		if (mode[0] == 'r')
		{
			if (mode[1] == 0)
			{
				// Opens for reading. If the file does not exist or cannot be found, the fopen call fails.
				dwAccess = GENERIC_READ;
				dwCreate = OPEN_EXISTING;
			}
			else if (mode[1] == '+' && mode[2] == 0)
			{
				// Opens for both reading and writing. (The file must exist.)
				dwAccess = GENERIC_READ | GENERIC_WRITE;
				dwCreate = OPEN_EXISTING;
			}
		}
		else if (mode[0] == 'w')
		{
			if (mode[1] == 0)
			{
				// Opens an empty file for writing. If the given file exists, its contents are destroyed.
				dwAccess = GENERIC_WRITE;
				dwCreate = CREATE_ALWAYS;
			}
			else if (mode[1] == '+' && mode[2] == 0)
			{
				// Opens an empty file for both reading and writing. If the given file exists, its contents are destroyed.
				dwAccess = GENERIC_READ | GENERIC_WRITE;
				dwCreate = CREATE_ALWAYS;
			}
		}
		else if (mode[0] == 'a')
		{
			bSeekEnd = true;
			if (mode[1] == 0)
			{
				// Opens for writing at the end of the file (appending) without removing the EOF marker before writing new data to the file; creates the file first if it doesn't exist.
				dwAccess = GENERIC_WRITE;
				dwCreate = OPEN_ALWAYS;
			}
			else if (mode[1] == '+' && mode[2] == 0)
			{
				// Opens for reading and appending; the appending operation includes the removal of the EOF marker before new data is written to the file and the EOF marker is restored after writing is complete; creates the file first if it doesn't exist.
				dwAccess = GENERIC_READ | GENERIC_WRITE;
				dwCreate = OPEN_ALWAYS;
			}
		}

		m_hFile = ::CreateFile(
			lpszFileName,
			dwAccess,
			dwShare,
			0,
			dwCreate,
			FILE_ATTRIBUTE_NORMAL,
			0);

		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			if (bSeekEnd)
				::SetFilePointer(m_hFile, 0, 0, FILE_END);

			return true;
		}

//		std::wstring s = last_error_to_str(::GetLastError());
		return false;
	}

	// --------------------------------------------------------------------

	file::pos_type file::seek(file::pos_type offset)
	{
		__wabc_static_assert(sizeof(file::pos_type) == sizeof(LONG)* 2);
		LONG *p = (LONG*)&offset;
		if (offset == file::pos_type(-1))
		{
			offset = 0;
			*p = ::SetFilePointer(m_hFile, *p, p + 1, FILE_END);
		}
		else
			*p = ::SetFilePointer(m_hFile, *p, p + 1, FILE_BEGIN);
		return offset;
	}

	// --------------------------------------------------------------------

	file::pos_type file::size()const throw()
	{
		__wabc_static_assert(sizeof(file::pos_type) == sizeof(DWORD)* 2);
		file::pos_type offset = 0;
		DWORD *p = (DWORD*)&offset;

		*p = ::GetFileSize(m_hFile, p + 1);
		return offset;
	}

	// --------------------------------------------------------------------

	file::pos_type file::pos()throw()
	{
		__wabc_static_assert(sizeof(file::pos_type) == sizeof(LONG)* 2);
		file::pos_type offset = 0;
		LONG *p = (LONG*)&offset;
		*p = ::SetFilePointer(m_hFile, *p, p + 1, FILE_CURRENT);
		return offset;
	}

	// --------------------------------------------------------------------
	// directory
	// --------------------------------------------------------------------

	string directory::module()
	{
		string strDir(module_file_name());

		const size_t n = strDir.find_last_of('\\');
		if (n != string::npos)
			strDir.resize(n);
		return strDir;
	}

	// --------------------------------------------------------------------

	static bool make_dir(string &s, size_t pos)
	{
		size_t n = s.rfind('\\', pos);
		if (n != s.npos)
		{
			s[n] = '\0';
			const BOOL b = ::CreateDirectory(s.c_str(), 0);
			if (b == 0)
			{
				const DWORD err = ::GetLastError();
				if (err == ERROR_ALREADY_EXISTS)
				{
					s[n] = '\\';
					return true;
				}

				if (err == ERROR_PATH_NOT_FOUND)
				{
					if (make_dir(s, n))
					{
						s[n] = '\\';
						return ::CreateDirectory(s.c_str(), 0) != 0;
					}
				}
				return false;
			}
			else
				return true;
		}
		else
			return false;
	}

	// --------------------------------------------------------------------

	bool directory::make(const wchar_t *szDir)
	{
		if (::CreateDirectory(szDir, 0) == 0)
		{
			const DWORD err = ::GetLastError();
			if (err == ERROR_ALREADY_EXISTS)
				return true;

			if (err == ERROR_PATH_NOT_FOUND)
			{
				string s(szDir);
				if (make_dir(s, s.npos))
					return ::CreateDirectory(szDir, 0) != FALSE;
			}
			return false;
		}
		return true;
	}

	// --------------------------------------------------------------------

	string & directory::cat(string &strPath, const wchar_t *szSubPath)
	{
		size_t nForwardLevel = 0, i, nPos;
		for (i = 0;; ++i)
		{
			if (szSubPath[i] == '.')
			{
				const wchar_t &ch = szSubPath[++i];
				if (ch == '\\')
					continue;

				if (ch == '.')
				{
					const wchar_t &ch = szSubPath[++i];
					if (ch == '\\')
					{
						++nForwardLevel;
						continue;
					}
				}
				return strPath;
			}
			break;
		}

		if (i == 0)
		{
			strPath = szSubPath;
		}
		else
		{
			for (nPos = string::npos; nForwardLevel-- > 0;)
			{
				nPos = strPath.find_last_of('\\', nPos);
				if (nPos == string::npos)
					return strPath;
			}

			strPath.resize(nPos);
			strPath.append(szSubPath + i - 1);
		}
		return strPath;
	}

	// --------------------------------------------------------------------
	// module_file_name
	// --------------------------------------------------------------------

	string module_file_name()
	{
		string strDir;
		DWORD nSize = 0, n = 0;

		while (n == nSize)
		{
			nSize += 32;

			strDir.resize(nSize);
			n = ::GetModuleFileName(0, &strDir[0], nSize);
		}

		strDir.resize(n);
		return strDir;
	}

	// --------------------------------------------------------------------
	// module_file_name
	// --------------------------------------------------------------------

	void change_file_name_ext(string & strFileName, const wchar_t *szExt)
	{
		const size_t n = strFileName.rfind('.');
		if (n != string::npos)
			strFileName.resize(n);
		strFileName += szExt;
	}
}