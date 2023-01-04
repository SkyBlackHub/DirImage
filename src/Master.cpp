#include "Master.h"

#include "Logger.h"
#include "Image.h"
#include "Thumbs.h"
#include "INI.h"

#include "FileIterator.h"
#include "NaturalCompare.h"

#include <iterator>
#include <sstream>
#include <windowsx.h>

#pragma comment(lib, "Shell32.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define DIP_CLASSNAME_LISTER_WINDOW L"ListerWindowClass"

#define MENU_MAX_COLS_ROWS 10
#define MENU_MAX_SHIFT 10

enum {
	DIP_CMD_SHOW_ABOUT               = 2000,

	DIP_CMD_LANG_EN,
	DIP_CMD_LANG_RU,

	DIP_CMD_SAVE_CONFIG,

	DIP_CMD_VIEW_ENABLED             = 2100,
	DIP_CMD_VIEW_ENLARGE,
	DIP_CMD_VIEW_TRANSPARENCY_GRID,

	DIP_CMD_VIEW_COLS,
	DIP_CMD_VIEW_ROWS = DIP_CMD_VIEW_COLS + MENU_MAX_COLS_ROWS,

	DIP_CMD_VIEW_ADAPTIVE = DIP_CMD_VIEW_ROWS + MENU_MAX_COLS_ROWS,

	DIP_CMD_SHIFT,

	DIP_CMD_VIEW_DEEP_SCAN = DIP_CMD_SHIFT + MENU_MAX_SHIFT,

	DIP_CMD_VIEW_INFO,

	DIP_CMD_VIEW_NEXT_PAGE,
	DIP_CMD_VIEW_PREVIOUS_PAGE,
	DIP_CMD_VIEW_FIRST_PAGE,
	DIP_CMD_VIEW_LAST_PAGE,

	DIP_CMD_VIEW_SIZE_0_0            = 2200,
	DIP_CMD_VIEW_SIZE_5_5            = 2255,

	DIP_CMD_THUMBS_ENABLED           = 2300
};


DIP::Master::Master(HINSTANCE hinstance, const std::wstring &basepath) : m_basepath(basepath)
{
	this->loadConfig();

	// register window class
	m_wcex.cbSize        = sizeof(WNDCLASSEX);
	m_wcex.style         = CS_DBLCLKS;
	m_wcex.lpfnWndProc   = DIP::Master::ListerWindowProc;
	m_wcex.cbClsExtra    = 0;
	m_wcex.cbWndExtra    = sizeof(intptr_t);
	m_wcex.hInstance     = hinstance;
	m_wcex.hIcon         = nullptr;
	m_wcex.hCursor       = nullptr;
	m_wcex.hbrBackground = nullptr;
	m_wcex.lpszMenuName  = nullptr;
	m_wcex.lpszClassName = DIP_CLASSNAME_LISTER_WINDOW;
	m_wcex.hIconSm       = nullptr;

	RegisterClassEx(&m_wcex);

	this->rebuildMenu();
}

void DIP::Master::rebuildMenu()
{
	if (m_popup_menu) {
		DestroyMenu(m_popup_menu);
	}

	m_popup_menu = CreatePopupMenu();

	struct MenuItem
  {
    UINT id = 0;
    LPCWSTR caption = nullptr;
		bool checked = false;
		std::vector<MenuItem> menu;
		bool default_ = false;
  };

	auto buildMenu = [] (auto& self, HMENU menu, const std::vector<MenuItem> &items) {
		if (items.empty()) {
			return;
		}

		for (const auto &item : items) {
			if (item.menu.empty()) {
				AppendMenu(menu,
					MF_BYPOSITION | (item.id >= DIP_CMD_SHOW_ABOUT ? MF_STRING : (item.id == 1 ? MF_MENUBARBREAK : MF_SEPARATOR)) | (item.checked ? MF_CHECKED : 0),
					item.id, item.caption
				);
				if (item.default_) {
					SetMenuDefaultItem(menu, item.id, false);
				}
				continue;
			}

			HMENU submenu = CreatePopupMenu();
			self(self, submenu, item.menu);
			AppendMenu(menu, MF_BYPOSITION | MF_POPUP, UINT_PTR(submenu), item.caption);
		}
	};

	static const wchar_t * numbers[11] = {L"0", L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9", L"10"};

	std::vector<MenuItem> cols;
	std::vector<MenuItem> rows;
	for (int i = 0; i < MENU_MAX_COLS_ROWS; ++i) {
		cols.push_back({UINT(DIP_CMD_VIEW_COLS + i), numbers[i + 1]});
		rows.push_back({UINT(DIP_CMD_VIEW_ROWS + i), numbers[i + 1]});
	}

	cols[1].default_ = true;
	rows[1].default_ = true;

	std::vector<MenuItem> shifts;
	for (int i = 0; i < MENU_MAX_SHIFT; ++i) {
		shifts.push_back({UINT(DIP_CMD_SHIFT + i), numbers[i]});
	}

	shifts[0].default_ = true;

  buildMenu(buildMenu, m_popup_menu, {
		{DIP_CMD_VIEW_NEXT_PAGE, tr(LOC_NEXT_PAGE)},
		{DIP_CMD_VIEW_PREVIOUS_PAGE, tr(LOC_PREVIOUS_PAGE)},
		{DIP_CMD_VIEW_FIRST_PAGE, tr(LOC_FIRST_PAGE)},
		{DIP_CMD_VIEW_LAST_PAGE, tr(LOC_LAST_PAGE)},
		{},
		{0, tr(LOC_PRESETS), false, {
			{DIP_CMD_VIEW_SIZE_0_0 + 11, L"1x1"},
			{DIP_CMD_VIEW_SIZE_0_0 + 22, L"2x2"},
			{DIP_CMD_VIEW_SIZE_0_0 + 33, L"3x3"},
			{DIP_CMD_VIEW_SIZE_0_0 + 44, L"4x4"},
			{DIP_CMD_VIEW_SIZE_0_0 + 55, L"5x5"},
			{},
			{DIP_CMD_VIEW_SIZE_0_0 + 21, L"2x1"},
			{DIP_CMD_VIEW_SIZE_0_0 + 32, L"3x2"},
			{DIP_CMD_VIEW_SIZE_0_0 + 43, L"4x3"},
			{DIP_CMD_VIEW_SIZE_0_0 + 54, L"5x4"},
		}},
		{0, tr(LOC_COLS), false, cols},
		{0, tr(LOC_ROWS), false, rows},
		{DIP_CMD_VIEW_ADAPTIVE, tr(LOC_ADAPTIVE), m_view_config.adaptive},
		{},
		{0, tr(LOC_SHIFT), false, shifts},
		{},
		{DIP_CMD_VIEW_DEEP_SCAN, tr(LOC_DEEP_SCAN), m_view_config.deep_scan},
		{},
		{DIP_CMD_VIEW_INFO, tr(LOC_INFO), m_view_config.info},
		{},
		{DIP_CMD_VIEW_ENLARGE, tr(LOC_ENLARGE), m_view_config.enlarge},
		{DIP_CMD_VIEW_TRANSPARENCY_GRID, tr(LOC_TRANSPARENCY_GRID), m_view_config.transparency_grid},
		{},
		{0, tr(LOC_OPTIONS), false, {
			{DIP_CMD_VIEW_ENABLED, tr(LOC_THUMBS_ENABLED), m_view_config.enabled},
			{DIP_CMD_THUMBS_ENABLED, tr(LOC_VIEW_ENABLED), m_thumbs_config.enabled},
			{0, tr(LOC_LANG), false, {
				{DIP_CMD_LANG_EN, tr(LOC_LANG_EN)},
				{DIP_CMD_LANG_RU, tr(LOC_LANG_RU)}
			}},
			{DIP_CMD_SAVE_CONFIG, tr(LOC_SAVE_CONFIG)}
		}},
		{},
		{DIP_CMD_SHOW_ABOUT, tr(LOC_ABOUT)}
	});

	CheckMenuRadioItem(m_popup_menu, DIP_CMD_VIEW_COLS, DIP_CMD_VIEW_COLS + MENU_MAX_COLS_ROWS, DIP_CMD_VIEW_COLS + m_view_config.cols - 1, MF_BYCOMMAND);
	CheckMenuRadioItem(m_popup_menu, DIP_CMD_VIEW_ROWS, DIP_CMD_VIEW_ROWS + MENU_MAX_COLS_ROWS, DIP_CMD_VIEW_ROWS + m_view_config.rows - 1, MF_BYCOMMAND);
	CheckMenuRadioItem(m_popup_menu, DIP_CMD_VIEW_SIZE_0_0, DIP_CMD_VIEW_SIZE_5_5, DIP_CMD_VIEW_SIZE_0_0 + m_view_config.cols * 10 + m_view_config.rows, MF_BYCOMMAND);
	CheckMenuRadioItem(m_popup_menu, DIP_CMD_LANG_EN, DIP_CMD_LANG_RU, DIP_CMD_LANG_EN + m_language, MF_BYCOMMAND);
	CheckMenuRadioItem(m_popup_menu, DIP_CMD_SHIFT, DIP_CMD_SHIFT + MENU_MAX_SHIFT, DIP_CMD_SHIFT + m_view_config.shift, MF_BYCOMMAND);
}

DIP::Master::~Master()
{
  DestroyMenu(m_popup_menu);

	UnregisterClass(m_wcex.lpszClassName, m_wcex.hInstance);
}

DIP::Thumbs *obtainThumbsFromHandle(HWND hwnd)
{
	return reinterpret_cast<DIP::Thumbs *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}

HWND DIP::Master::createTrackingToolTip(HWND hwnd, const wchar_t *text)
{
	HWND result = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
															 WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
															 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
															 hwnd, NULL, m_wcex.hInstance, NULL);

	if (!result) {
		return nullptr;
	}

	m_tooltip_info.cbSize   = sizeof(TOOLINFO);
	m_tooltip_info.uFlags   = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	m_tooltip_info.hwnd     = hwnd;
	m_tooltip_info.hinst    = m_wcex.hInstance;
	m_tooltip_info.lpszText = const_cast<wchar_t *>(text);
	m_tooltip_info.uId      = (UINT_PTR) hwnd;

	GetClientRect(hwnd, &m_tooltip_info.rect);

	SendMessage(result, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &m_tooltip_info);
	// multiline hack
	SendMessage(result, TTM_SETMAXTIPWIDTH, 0, 100000);

	return result;
}

LRESULT DIP::Master::ListerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//Log.debug(L"Window proc\t| message = %d\t| wParam = %d\t| lParam = %d", message, wParam, lParam);

	switch (message) {
		case WM_MOUSELEAVE:
			DIP::Master::instance().processMouseLeave(hwnd);
			return 0;

		case WM_MOUSEMOVE: {
			DIP::Master::instance().processMouseMove(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;
		}

		case WM_SIZE: {
			DIP::Thumbs *thumbs = obtainThumbsFromHandle(hwnd);
			if (thumbs == nullptr) {
				break;
			}

			RECT rect;
			GetClientRect(hwnd, &rect);
			thumbs->resize(rect.right - rect.left, rect.bottom - rect.top);
			if (thumbs->isUpdateRequired()) {
				thumbs->update();
				DIP::Master::instance().invalidate(hwnd);
			}
			break;
		}

		case WM_CREATE:
			DIP::Master::instance().processCreate(hwnd);
			break;

		case WM_DESTROY: {
			DIP::Master::instance().processDestroy(hwnd);
			delete obtainThumbsFromHandle(hwnd);
			break;
		}

		case WM_PAINT: {
			DIP::Thumbs *thumbs = obtainThumbsFromHandle(hwnd);
			if (thumbs == nullptr) {
				break;
			}

			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			RECT rect = {0, 0, static_cast<long>(thumbs->width()), static_cast<long>(thumbs->height())};
			HBRUSH brush = CreateSolidBrush(RGB(thumbs->background().rgbRed, thumbs->background().rgbGreen, thumbs->background().rgbBlue));
			FillRect(hdc, &rect, brush);
			DeleteObject(brush);

			if (thumbs->isLoaded()) {
				thumbs->draw(hdc, 0, 0);
			} else {
				thumbs->update();
				DIP::Master::instance().invalidate(hwnd);
			}

			if (thumbs->infoShow()) {
				GetClientRect(hwnd, &rect);

				rect.left = 10;
				rect.top = rect.bottom - thumbs->infoSize() - 10;

				thumbs->drawInfo(hdc, rect);
			}

			EndPaint(hwnd, &ps);
			break;
		}

		case WM_ERASEBKGND:
			return 1;

		case WM_COMMAND:
			DIP::Master::instance().processCommand(hwnd, LOWORD(wParam));
			return 0;

		case WM_LBUTTONDOWN:
			SetForegroundWindow(hwnd);
			SetFocus(hwnd);
			return 0;

		case WM_LBUTTONDBLCLK:
			DIP::Master::instance().processDoubleClick(hwnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_RBUTTONUP: {
			auto &master = DIP::Master::instance();

			SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM) master.m_popup_menu, 0);
			{
				POINT cursor_position;
				GetCursorPos(&cursor_position);
				WORD command = TrackPopupMenu(master.m_popup_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, cursor_position.x, cursor_position.y, 0, hwnd, NULL);
				SendMessage(hwnd, WM_COMMAND, command, 0);
			}
			return 0;
		}

		case WM_KEYDOWN:
			DIP::Master::instance().processKey(hwnd, wParam);
			break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

void DIP::Master::processCreate(HWND hwnd)
{
	// the text MUST NOT be empty, otherwise the tooltip will not appear! Damn you dumb indians who wrote the MS documentation
	m_tooltip = createTrackingToolTip(hwnd, L" ");
	this->processMouseMove(hwnd, mouse_x, mouse_y, true);
}

void DIP::Master::processDestroy(HWND hwnd)
{
	if (m_tooltip) {
		this->processMouseLeave(hwnd);
		DestroyWindow(m_tooltip);
		m_tooltip = nullptr;
		m_tooltip_image = nullptr;
	}
}

void DIP::Master::processKey(HWND hwnd, WPARAM key)
{
	switch (key) {
		case VK_LEFT:
			return this->processCommand(hwnd, DIP_CMD_VIEW_PREVIOUS_PAGE);

		case VK_RIGHT:
			return this->processCommand(hwnd, DIP_CMD_VIEW_NEXT_PAGE);

		case VK_HOME:
			return this->processCommand(hwnd, DIP_CMD_VIEW_FIRST_PAGE);

		case VK_END:
			return this->processCommand(hwnd, DIP_CMD_VIEW_LAST_PAGE);
	}
}

void DIP::Master::processCommand(HWND hwnd, UINT command)
{
	bool invalidate = false;
	DIP::Thumbs *thumbs;
	if (command > DIP_CMD_VIEW_ENABLED && command < DIP_CMD_THUMBS_ENABLED) {
		thumbs = obtainThumbsFromHandle(hwnd);
		if (thumbs == nullptr) {
			return;
		}
		invalidate = true;
	}

	switch (command) {
		case DIP_CMD_SHOW_ABOUT:
			MessageBox(hwnd, (std::wstring(tr(LOC_APP_NAME)) + L"\n" + tr(LOC_APP_VERSION) + L": " + DIP_VERSION + L"\n" + DIP::Image::version()).data(), tr(LOC_ABOUT), MB_ICONINFORMATION | MB_OK);
			break;

		case DIP_CMD_SAVE_CONFIG:
			this->saveConfig();
			break;

		case DIP_CMD_VIEW_NEXT_PAGE:
			invalidate = thumbs->nextPage();
			break;

		case DIP_CMD_VIEW_PREVIOUS_PAGE:
			invalidate = thumbs->previousPage();
			break;

		case DIP_CMD_VIEW_FIRST_PAGE:
			invalidate = thumbs->firstPage();
			break;

		case DIP_CMD_VIEW_LAST_PAGE:
			invalidate = thumbs->lastPage();
			break;

		case DIP_CMD_VIEW_ENABLED:
			this->toggleOption(command, m_view_config.enabled);
			break;

		case DIP_CMD_THUMBS_ENABLED:
			this->toggleOption(command, m_thumbs_config.enabled);
			break;

		case DIP_CMD_VIEW_ADAPTIVE:
			thumbs->setAdaptive(this->toggleOption(command, m_view_config.adaptive));
			break;

		case DIP_CMD_VIEW_INFO:
			thumbs->setInfoShow(this->toggleOption(command, m_view_config.info));
			invalidate = true;
			break;

		case DIP_CMD_VIEW_DEEP_SCAN:
			this->toggleOption(command, m_view_config.deep_scan);
			break;

		case DIP_CMD_VIEW_ENLARGE:
			thumbs->setEnlarge(this->toggleOption(command, m_view_config.enlarge));
			break;

		case DIP_CMD_VIEW_TRANSPARENCY_GRID:
			thumbs->setShowTransparencyGrid(this->toggleOption(command, m_view_config.transparency_grid));
			break;

		default:
			if (command >= DIP_CMD_LANG_EN && command <= DIP_CMD_LANG_RU) {
				m_language = static_cast<Language>(command - DIP_CMD_LANG_EN);
				CheckMenuRadioItem(m_popup_menu, DIP_CMD_LANG_EN, DIP_CMD_LANG_RU, command, MF_BYCOMMAND);
				this->rebuildMenu();
				break;
			}
			if (command > DIP_CMD_VIEW_SIZE_0_0 && command <= DIP_CMD_VIEW_SIZE_5_5) {
				UINT cols = (command - DIP_CMD_VIEW_SIZE_0_0) / 10;
				UINT rows = (command - DIP_CMD_VIEW_SIZE_0_0) % 10;
				this->toggleRadio(DIP_CMD_VIEW_COLS + cols - 1, DIP_CMD_VIEW_COLS, MENU_MAX_COLS_ROWS, m_view_config.cols);
				this->toggleRadio(DIP_CMD_VIEW_ROWS + rows - 1, DIP_CMD_VIEW_ROWS, MENU_MAX_COLS_ROWS, m_view_config.rows);
				CheckMenuRadioItem(m_popup_menu, DIP_CMD_VIEW_SIZE_0_0, DIP_CMD_VIEW_SIZE_5_5, command, MF_BYCOMMAND);
				thumbs->reallocate(cols, rows);
				break;
			}
			if (this->toggleRadio(command, DIP_CMD_VIEW_COLS, MENU_MAX_COLS_ROWS, m_view_config.cols)) {
				thumbs->setCols(m_view_config.cols);
				CheckMenuRadioItem(m_popup_menu, DIP_CMD_VIEW_SIZE_0_0, DIP_CMD_VIEW_SIZE_5_5, DIP_CMD_VIEW_SIZE_0_0 + m_view_config.cols * 10 + m_view_config.rows, MF_BYCOMMAND);
				break;
			}
			if (this->toggleRadio(command, DIP_CMD_VIEW_ROWS, MENU_MAX_COLS_ROWS, m_view_config.rows)) {
				thumbs->setRows(m_view_config.rows);
				CheckMenuRadioItem(m_popup_menu, DIP_CMD_VIEW_SIZE_0_0, DIP_CMD_VIEW_SIZE_5_5, DIP_CMD_VIEW_SIZE_0_0 + m_view_config.cols * 10 + m_view_config.rows, MF_BYCOMMAND);
				break;
			}
			if (this->toggleRadio(command, DIP_CMD_SHIFT, MENU_MAX_SHIFT, m_view_config.shift, 0)) {
				thumbs->setShift(m_view_config.shift);
				break;
			}
	}

	if (invalidate) {
		this->invalidate(hwnd);
	}
}

void DIP::Master::processDoubleClick(HWND hwnd, int x, int y)
{
	DIP::Thumbs *thumbs = obtainThumbsFromHandle(hwnd);
	if (thumbs == nullptr) {
		return;
	}
	std::wstring filename = thumbs->fullFilenameAt(x, y);
	if (filename.empty() == false) {
		ShellExecute(hwnd, NULL, filename.data(), NULL, NULL, SW_SHOWNORMAL);
	}
}

void DIP::Master::processMouseLeave(HWND /*hwnd*/)
{
	if (m_tracking_mouse) {
		SendMessage(m_tooltip, TTM_TRACKACTIVATE, 0, (LPARAM) &m_tooltip_info);
		m_tracking_mouse = false;
	}
}

void DIP::Master::processMouseMove(HWND hwnd, int x, int y, bool force)
{
	if (force || x != mouse_x || y != mouse_y) {
		mouse_x = x;
		mouse_y = y;
	} else {
		return;
	}

	DIP::Thumbs *thumbs = obtainThumbsFromHandle(hwnd);
	if (thumbs == nullptr) {
		return;
	}

	int index = thumbs->indexAt(x, y);
	if (index == -1) {
		this->processMouseLeave(hwnd);
		return;
	}

	if (m_tracking_mouse == false) {

		TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT)};
		tme.hwndTrack       = hwnd;
		tme.dwFlags         = TME_LEAVE;

		TrackMouseEvent(&tme);

		SendMessage(m_tooltip, TTM_TRACKACTIVATE, 1, (LPARAM) &m_tooltip_info);

		m_tracking_mouse = true;
	}

	Image *image = thumbs->image(index);
	if (image != m_tooltip_image) {
		m_tooltip_image = image;

		std::wstring filename = thumbs->filenameAt(x, y);

		WCHAR text[1024];
		if (image) {
			swprintf_s(text, ARRAYSIZE(text), L"%s\n#%d, %d x %d, %d BPP", filename.data(), index + 1, image->width(), image->height(), image->bpp());
		} else {
			swprintf_s(text, ARRAYSIZE(text), L"%s\n#%d", filename.data(), index + 1);
		}

		m_tooltip_info.lpszText = text;
		SendMessage(m_tooltip, TTM_SETTOOLINFO, 0, (LPARAM) &m_tooltip_info);
	}

	POINT position = {x + 30, y + 30};
	ClientToScreen(hwnd, &position);
	SendMessage(m_tooltip, TTM_TRACKPOSITION, 0, (LPARAM) MAKELONG(position.x, position.y));
}

void DIP::Master::invalidate(HWND hwnd)
{
	InvalidateRect(hwnd, nullptr, false);
	//m_tooltip_image = nullptr;
	this->processMouseMove(hwnd, mouse_x, mouse_y, m_tracking_mouse);
}

bool DIP::Master::toggleOption(UINT command, bool &option)
{
	option = !option;
	this->switchPopupMenuItem(command, option);
	return option;
}

void DIP::Master::setOption(UINT command, int &option, int value)
{
	option = value;
	this->switchPopupMenuItem(command, option);
}

bool DIP::Master::toggleRadio(UINT command, UINT start, UINT size, unsigned int &radio, unsigned int shift)
{
	if (command >= start && command < start + size) {
		CheckMenuRadioItem(m_popup_menu, start, start + size, command, MF_BYCOMMAND);
		radio = command - start + shift;
		return true;
	}
	return false;
}

void DIP::Master::switchMenuItem(HMENU menu, UINT item_id, bool value)
{
	MENUITEMINFO mif;
	mif.cbSize = sizeof(MENUITEMINFO);
	mif.fMask = MIIM_STATE;
	GetMenuItemInfo(menu, item_id, false, &mif);
	if (value) {
		mif.fState |= MFS_CHECKED;
	} else {
		mif.fState ^= MFS_CHECKED;
	}
	SetMenuItemInfo(menu, item_id, false, &mif);
}

void DIP::Master::switchPopupMenuItem(UINT item_id, bool value) const
{
	switchMenuItem(m_popup_menu, item_id, value);
}

bool DIP::Master::isViewEnabled() const
{
	return m_view_config.enabled;
}

bool DIP::Master::isThumbsEnabled() const
{
	return m_thumbs_config.enabled;
}

bool checkDots(const wchar_t *path)
{
	size_t length = wcslen(path);
	if (length < 3) {
		return false;
	}
	if (path[length - 2] == L'.' && path[length - 3] == L'.') {
		return true;
	}
	return false;
}

DIP::Thumbs *DIP::Master::prepareThumbs(const wchar_t *path, int width, int height, const ShowConfig &config)
{
	Log.debug(L"Generating thumbs | path = %s | size = %dx%d", path, width, height);

	if (config.ignore_dots && checkDots(path)) {
		return nullptr;
	}

	std::vector<std::wstring> files = scan(path, config);
	if (files.empty()) {
		return nullptr;
	}

	DIP::Thumbs *thumbs = new DIP::Thumbs(path, files, config.cols, config.rows, width, height);

	thumbs->setBackground(config.background);
	thumbs->setFilter(config.filter);
	thumbs->setEnlarge(config.enlarge);
	thumbs->setShowTransparencyGrid(config.transparency_grid);

	thumbs->setHorizontalPadding(config.pad_h);
	thumbs->setVerticalPadding(config.pad_v);

	thumbs->setInfoShow(config.info);
	thumbs->setInfoColor(config.info_color);
	thumbs->setInfoSize(config.info_size);

	thumbs->setShift(config.shift);

	return thumbs;
}

static std::vector<std::wstring> searchFiles(const std::wstring &path, const std::vector<std::wstring> &extensions, int files_limit, DIP::FileIterator::Mode mode = DIP::FileIterator::MODE_FILES)
{
	std::vector<std::wstring> result;

	result.reserve(files_limit);

	DIP::FileIterator iterator(path.data(), extensions, files_limit, mode);

	if (iterator.isValid() == false) {
		return result;
	}

	do {
		result.push_back(iterator.filename());
	} while (iterator.next());

	result.shrink_to_fit();

	std::sort(result.begin(), result.end(), [] (const std::wstring &a, const std::wstring &b) {
		return DIP::naturalCompare(a.data(), b.data()) < 0;
	});

	return result;
}

static std::vector<std::wstring> innerScan(const std::wstring &path, const DIP::ShowConfig &config, unsigned int level = 0)
{
	int files_limit = level ? config.deep_scan_files_limit : config.files_limit;
	std::vector<std::wstring> result = searchFiles(path, config.extensions, files_limit);

	if (result.empty() && config.deep_scan && level < config.deep_scan_level && config.deep_scan_limit) {
		for (const std::wstring &directory : searchFiles(path, config.extensions, files_limit, DIP::FileIterator::MODE_DIRECTORIES)) {
			std::vector<std::wstring> sub = innerScan(path + L"\\" + directory, config, level + 1);
			if (sub.empty() == false) {
				size_t size = min(config.deep_scan_limit, sub.size());
				result.reserve(result.size() + size);
				for (size_t i = 0; i < size; ++i) {
					result.push_back(directory + L"\\" + sub[i]);
				}
			}
		}
	}

	return result;
}

std::vector<std::wstring> DIP::Master::scan(const wchar_t *path, const ShowConfig &config)
{
	return innerScan(path, config);
}

HBITMAP DIP::Master::generateThumbs(const wchar_t *path, int width, int height) const
{
	DIP::Thumbs *thumbs = prepareThumbs(path, width, height, m_thumbs_config);
	if (thumbs == nullptr) {
		return nullptr;
	}
	HBITMAP bitmap = thumbs->bitmap();
	delete thumbs;
	return bitmap;
}

HWND DIP::Master::generateView(const wchar_t *path, HWND parent, int x, int y, int width, int height) const
{
	DIP::Thumbs *thumbs = prepareThumbs(path, width, height, m_view_config);
	if (thumbs == nullptr) {
		return nullptr;
	}

	HWND handle = CreateWindowEx(
		0,
		m_wcex.lpszClassName,
		nullptr,
		WS_CHILD,
		x,
		y,
		thumbs->width(),
		thumbs->height(),
		parent,
		nullptr,
		m_wcex.hInstance,
		nullptr
	);

	if (handle == nullptr) {
		Log.error(L"HWND is null.");
		delete thumbs;
		return nullptr;
	}

	SetWindowLongPtr(handle, GWLP_USERDATA, reinterpret_cast<intptr_t>(thumbs));
	return handle;
}

static const std::unordered_map<const wchar_t *, unsigned int> languages_map = {
	{L"en", DIP::LANG_EN},
	{L"ru", DIP::LANG_RU},
};

static const std::unordered_map<const wchar_t *, unsigned int> log_levels_map = {
	{L"",      DIP::Logger::LEVEL_NONE},
	{L"none",  DIP::Logger::LEVEL_NONE},
	{L"error", DIP::Logger::LEVEL_ERROR},
	{L"info",  DIP::Logger::LEVEL_INFO},
	{L"debug", DIP::Logger::LEVEL_DEBUG}
};

void DIP::Master::loadConfig()
{
	DIP::INI ini;
	DIP::INI::Result result = ini.load(m_basepath + L"DirImage.ini");
	if (result != DIP::INI::RESULT_OK) {
		return;
	}

	ini.setSection(L"general");

	ini.readEnum(L"language", languages_map, reinterpret_cast<unsigned int &>(m_language));

	Log.setLevel(static_cast<DIP::Logger::Level>(ini.getEnum(L"log", log_levels_map, Log.level())));

	ini.readString(L"log_file", m_log_file);
	if (m_log_file.empty() == false) {
		Log.setOutputToFile(m_basepath + m_log_file);
		Log.setOutputToStream(false);
	}

	ini.setFallbackSection(L"common");

	ini.setSection(L"view");
	this->loadShowConfig(ini, m_view_config);

	ini.setSection(L"thumbs");
	this->loadShowConfig(ini, m_thumbs_config);
}

void DIP::Master::saveConfig()
{
	DIP::INI ini;

	ini.setSection(L"general");

	ini.setEnum(L"language", languages_map, reinterpret_cast<unsigned int &>(m_language));

	ini.setEnum(L"log", log_levels_map, Log.level());
	ini.setString(L"log_file", m_log_file);

	ini.setFallbackSection(L"common");

	ini.setSection(L"view");
	this->saveShowConfig(ini, m_view_config);

	ini.setSection(L"thumbs");
	this->saveShowConfig(ini, m_thumbs_config);

	ini.save(m_basepath + L"DirImage.ini");
}

static const std::unordered_map<const wchar_t *, unsigned int> filters_map = {
	{L"box",        DIP_IMAGE_FILTER_BOX},
	{L"bicubic",    DIP_IMAGE_FILTER_BICUBIC},
	{L"bilinear",   DIP_IMAGE_FILTER_BILINEAR},
	{L"bspline",    DIP_IMAGE_FILTER_BSPLINE},
	{L"catmullrom", DIP_IMAGE_FILTER_CATMULLROM},
	{L"lanczos3",   DIP_IMAGE_FILTER_LANCZOS3}
};

void DIP::Master::loadShowConfig(const INI &ini, ShowConfig &config) const
{
	ini.readBool(L"enabled", config.enabled);

	ini.readList(L"extensions", config.extensions, DIP::INI::TRANSFORM_LOWER);

	ini.readUInt(L"columns", config.cols);
	ini.readUInt(L"rows", config.rows);

	ini.readBool(L"adaptive", config.adaptive);

	ini.readColor(L"background", config.background);

	ini.readEnum(L"filter", filters_map, config.filter);

	ini.readBool(L"enlarge", config.enlarge);
	ini.readBool(L"transparency_grid", config.transparency_grid);

	ini.readUInt(L"pad_h", config.pad_h);
	ini.readUInt(L"pad_v", config.pad_v);

	ini.readBool(L"ignore_dots", config.ignore_dots);

	ini.readUInt(L"files_limit", config.files_limit);
	ini.readUInt(L"shift", config.shift);

	ini.readBool(L"deep_scan", config.deep_scan);
	ini.readUInt(L"deep_scan_level", config.deep_scan_level);
	ini.readUInt(L"deep_scan_limit", config.deep_scan_limit);
	ini.readUInt(L"deep_scan_files_limit", config.deep_scan_files_limit);

	ini.readBool(L"info", config.info);
	ini.readColor(L"info_color", config.info_color);
	ini.readUInt(L"info_size", config.info_size);
}

void DIP::Master::saveShowConfig(INI &ini, const ShowConfig &config) const
{
	ini.setBool(L"enabled", config.enabled);

	ini.setList(L"extensions", config.extensions);

	ini.setUInt(L"columns", config.cols);
	ini.setUInt(L"rows", config.rows);

	ini.setBool(L"adaptive", config.adaptive);

	ini.setColor(L"background", config.background);

	ini.setEnum(L"filter", filters_map, config.filter);

	ini.setBool(L"enlarge", config.enlarge);
	ini.setBool(L"transparency_grid", config.transparency_grid);

	ini.setUInt(L"pad_h", config.pad_h);
	ini.setUInt(L"pad_v", config.pad_v);

	ini.setBool(L"ignore_dots", config.ignore_dots);

	ini.setUInt(L"files_limit", config.files_limit);
	ini.setUInt(L"shift", config.shift);

	ini.setBool(L"deep_scan", config.deep_scan);
	ini.setUInt(L"deep_scan_level", config.deep_scan_level);
	ini.setUInt(L"deep_scan_limit", config.deep_scan_limit);
	ini.setUInt(L"deep_scan_files_limit", config.deep_scan_files_limit);

	ini.setBool(L"info", config.info);
	ini.setColor(L"info_color", config.info_color);
	ini.setUInt(L"info_size", config.info_size);
}

const wchar_t *DIP::Master::tr(int key) const
{
	const wchar_t *result = translate(m_language, key);
	return result ? result : translate(LANG_EN, key);
}