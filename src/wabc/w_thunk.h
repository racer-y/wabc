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
#pragma pack(push,1) //该结构必须以字节对齐
	struct wndproc_thunk 
	{
		DWORD   m_mov;          // mov dword ptr [esp+0x4], pThis (esp+0x4 is hWnd)
		DWORD   m_this;         //

		BYTE    m_jmp;          // jmp WndProc
		DWORD   m_relproc;      // relative jmp
	};
#pragma pack(pop)

	void * thunk_alloc(size_t new_size);
	void * thunk_realloc(void *p, size_t new_size);
	void thunk_free(void *);
}