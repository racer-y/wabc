#pragma once

#if defined(_UNICODE) || defined(UNICODE)

#ifdef _DEBUG
#pragma comment(lib, "D:\\Racer\\wabc\\dlib\\wabc.lib")
#else
#pragma comment(lib, "D:\\Racer\\wabc\\lib\\wabc.lib")
#endif

#else
#error "Only support UNICODE version"
#endif

#include "../src/wabc/w_dc.h"
#include "../src/wabc/w_msg.h"
#include "../src/wabc/w_file.h"
#include "../src/wabc/w_menu.h"
#include "../src/wabc/carray.h"
#include "../src/wabc/w_thread.h"
#include "../src/wabc/w_dialog.h"
#include "../src/wabc/w_convert.h"
#include "../src/wabc/w_registry.h"
#include "../src/wabc/w_stdctrl.h"
#include "../src/wabc/w_comctrl.h"
#include "../src/wabc/w_inifile.h"