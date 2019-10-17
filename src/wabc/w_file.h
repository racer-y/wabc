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

	class file
	{
	public:

		enum
		{
			share_none = 0,
			share_r = FILE_SHARE_READ,
			share_w = FILE_SHARE_WRITE,
			share_rw = FILE_SHARE_READ | FILE_SHARE_WRITE,
			share_delete = FILE_SHARE_DELETE,
			share_all = share_rw | share_delete
		};

		typedef string string;
		typedef __int64 pos_type;

		typedef file self;

	public:
		HANDLE m_hFile;

		file() : m_hFile(INVALID_HANDLE_VALUE) {}

		// mode: "r", "r+", "w", "w+", "a", "a+"
		file(string::const_pointer lpszFileName, string::const_pointer mode, DWORD dwShare = 0)throw()
			: m_hFile(INVALID_HANDLE_VALUE)
		{
			const bool result = open(lpszFileName, mode, dwShare);
		}
		file(const string &strFileName, string::const_pointer mode, DWORD dwShare = 0)throw()
			: m_hFile(INVALID_HANDLE_VALUE)
		{
			const bool result = open(strFileName.c_str(), mode, dwShare);
		}

		explicit file(HANDLE hFile) throw() : m_hFile(hFile)
		{
		}

		~file()
		{
			if (m_hFile != INVALID_HANDLE_VALUE)
				::CloseHandle(m_hFile);
		}

		bool operator !()const throw()
		{
			return m_hFile == INVALID_HANDLE_VALUE;
		}

		operator HANDLE()const { return m_hFile; }

		void swap(file &rhs)
		{
			std::swap(m_hFile, rhs.m_hFile);
		}
	public:

		// mode: "r", "r+", "w", "w+", "a", "a+"
		bool open(string::const_pointer lpszFileName, string::const_pointer mode, DWORD dwShare = 0)throw();
		bool open(const string &strFileName, string::const_pointer mode, DWORD dwShare = 0)throw()
		{
			return open(strFileName.c_str(), mode, dwShare);
		}

		bool is_open()const throw()
		{
			return m_hFile != INVALID_HANDLE_VALUE;
		}

		void close() throw()
		{
			if (m_hFile != INVALID_HANDLE_VALUE)
			{
				::CloseHandle(m_hFile);
				m_hFile = INVALID_HANDLE_VALUE;
			}
		}

		bool flush() throw()
		{
			return ::FlushFileBuffers(m_hFile) != 0;
		}

		bool resize(pos_type new_size) throw()
		{
			if (seek(new_size) == new_size)
			{
				::SetEndOfFile(m_hFile);
				return true;
			}
			return false;
		}

	public:

		size_t read(void *buf, size_t bytes_to_read)throw()
		{
			DWORD bytes_of_read;
			if (::ReadFile(m_hFile, buf, (DWORD)bytes_to_read, &bytes_of_read, 0))
				return bytes_of_read;
			return size_t(-1);
		}

		size_t write(const void * buf, size_t bytes_to_write)throw()
		{
			DWORD bytes_of_written;
			if (::WriteFile(m_hFile, buf, (DWORD)bytes_to_write, &bytes_of_written, 0))
				return bytes_of_written;
			return size_t(-1);
		}

		pos_type seek(pos_type offset);

		pos_type pos()throw();

		pos_type size()const throw();
		pos_type length()const throw(){ return size(); }

	private:

		file(const self &);
		self & operator=(const self &);
	};

	// --------------------------------------------------------------------

	struct ofn : OPENFILENAME
	{
		ofn()
		{
			::memset(this, 0, sizeof(*this));
			lStructSize = sizeof(OPENFILENAME);
		}
	};

	// --------------------------------------------------------------------

	struct open_file_name : ofn
	{
		bool get()
		{
			const BOOL b = ::GetOpenFileName(this);
			return b == TRUE;
		}
	};

	// --------------------------------------------------------------------

	struct save_file_name : ofn
	{
		bool get()
		{
			const BOOL b = ::GetSaveFileName(this);
			return b == TRUE;
		}
	};

	// --------------------------------------------------------------------

	struct findfile : WIN32_FIND_DATA
	{
		typedef findfile self;

		typedef size_t size_type;
		typedef file::pos_type	pos_type;

		findfile() : m_hFind(INVALID_HANDLE_VALUE){}

		~findfile() { close(); }

		void close()
		{
			if (m_hFind != INVALID_HANDLE_VALUE)
			{
				::FindClose(m_hFind);
				m_hFind = INVALID_HANDLE_VALUE;
			}
		}

		bool first(string::const_pointer lpszFileName)
		{
			if (m_hFind != INVALID_HANDLE_VALUE)
				::FindClose(m_hFind);

			m_hFind = ::FindFirstFile(lpszFileName, this);
			return (m_hFind != INVALID_HANDLE_VALUE);
		}

		bool first(string strFileName, string::const_pointer lpszWildcard)
		{
			assert(lpszWildcard[0] == '\\');
			strFileName += lpszWildcard;
			return first(strFileName.c_str());
		}

		bool first(const string &strFileName)
		{
			return first(strFileName.c_str());
		}

		bool next()
		{
			const BOOL result = ::FindNextFile(m_hFind, this) != 0;
			return result != 0;

		}

		const TCHAR * name()const { return cFileName; }
		bool is_dir()const { return (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; }

		file::pos_type file_size()const
		{
			file::pos_type result = nFileSizeLow;
			result |= uint64(nFileSizeHigh) << 32;
			return result;
		}

	private:
		HANDLE			m_hFind;

		findfile(const self &);
		self & operator=(const self &);
	};

	// --------------------------------------------------------------------
	// directory
	// --------------------------------------------------------------------

	struct directory
	{
		static string module();

		inline static bool make(const string & strDir) throw()
		{
			return make(strDir.c_str());
		}

		static bool make(const wchar_t *s) throw();

		static string & cat(string &strPath, const wchar_t *szSubPath);
	};

	string module_file_name();

	void change_file_name_ext(string & strFileName, const wchar_t *szExt);
}