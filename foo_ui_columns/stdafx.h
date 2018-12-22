#pragma once

#define OEMRESOURCE

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <algorithm>
#include <atomic>
#include <complex>
#include <future>
#include <iostream>
#include <memory>
#include <set>
#include <string_view>
#include <utility>
#include <vector>

#include <io.h>
#include <ppl.h>
#include <share.h>

#include <gsl/gsl>
#include <range/v3/all.hpp>

// Included before windows.h, because pfc.h includes winsock2.h
#include "../pfc/pfc.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <winuser.h>
#include <gdiplus.h>
#include <zmouse.h>
#include <uxtheme.h>
#include <wincodec.h>
#include <strsafe.h>

#include "../foobar2000/SDK/foobar2000.h"
#include "../foobar2000/SDK/core_api.h"
#include "../foobar2000/helpers/helpers.h"

#include "../columns_ui-sdk/ui_extension.h"
#include "../ui_helpers/stdafx.h"
#include "../mmh/stdafx.h"
#include "../fbh/stdafx.h"
#include "../pfc/range_based_for.h"

#include "resource.h"
#include "utf8api.h"
#include "helpers.h"
#include "config_items.h"
#include "gdiplus.h"
#include "config_vars.h"
#include "config_defaults.h"
#include "extern.h"

using namespace std::chrono_literals;
using namespace std::string_literals;
