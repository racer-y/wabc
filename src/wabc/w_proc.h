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
	struct _msg_struct;

	// --------------------------------------------------------------------

	class wndproc
	{
		// --------------------------------------------------------------------

	public:
		static bool process_WM(_msg_struct &msg, WPARAM wParam = 0, size_t no_flag = 0);

		static bool process_WM_CREATE(_msg_struct &msg);
		static bool process_WM_NCDESTROY(_msg_struct &msg);
		static bool process_WM_COMMAND(_msg_struct &msg);
		static bool process_WM_SYSCOMMAND(_msg_struct &msg);
		static bool process_WM_TIMER(_msg_struct &msg);
		static bool process_WM_NOTIFY(_msg_struct &msg);
		static bool process_WM_DRAWITEM(_msg_struct &msg);
		static bool process_WM_WTL(_msg_struct &msg);

		static bool process_procmsg(const _msg_struct &msg);

		static bool process(_msg_struct &msg);

	public:
		static 	LRESULT CALLBACK wnd_init(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static 	LRESULT CALLBACK wnd_main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static 	LRESULT CALLBACK scwnd_main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		static 	INT_PTR CALLBACK dlg_init(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static 	INT_PTR CALLBACK dlg_main(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}