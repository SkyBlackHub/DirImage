#ifndef DIP_MASTER_H
#define DIP_MASTER_H

#include "Singleton.h"
#include "Thumbs.h"
#include "L10n.h"

#include <string>
#include <unordered_map>

#include <commctrl.h>

#define DIP_VERSION L"0.1.0"

namespace DIP {

	struct ShowConfig {
		bool enabled = true;
		std::vector<std::wstring> extensions = {L"jpg", L"jpeg", L"gif", L"png", L"bmp", L"gif", L"webp"};
		unsigned int cols = 2;
		unsigned int rows = 2;
		bool adaptive = true;
		RGBQUAD background = {0, 0, 0, 0};
		unsigned int filter = 0; // DIP_IMAGE_FILTER_BOX
		bool enlarge = false;
		bool ignore_dots = true;
		bool transparency_grid = true;
		unsigned int pad_h = 1;
		unsigned int pad_v = 1;
		unsigned int files_limit = 1000;
		unsigned int shift = 0;

		bool deep_scan = false;
		unsigned int deep_scan_level = 1;
		unsigned int deep_scan_limit = 1;
		unsigned int deep_scan_files_limit = 100;

		bool info = false;
		RGBQUAD info_color = {0, 255, 0, 0};
		unsigned int info_size = 16;
	};

	class INI;

	class Master : public Singleton<Master>
	{
	public:
		~Master();

		void loadConfig();
		void saveConfig();

		bool isViewEnabled() const;
		bool isThumbsEnabled() const;

		HBITMAP generateThumbs(const wchar_t *path, int width, int height) const;
		HWND generateView(const wchar_t *path, HWND parent, int x, int y, int width, int height) const;

	private:
		DIP::ShowConfig m_view_config;
		DIP::ShowConfig m_thumbs_config;

		std::wstring m_basepath;
		WNDCLASSEX m_wcex;

		HMENU m_popup_menu = nullptr;

		int mouse_x = 0;
		int mouse_y = 0;

		HWND m_tooltip = nullptr;
		TOOLINFO m_tooltip_info;
		Image *m_tooltip_image = nullptr;

		bool m_tracking_mouse = false;

		Master(HINSTANCE hinstance, const std::wstring &basepath);

		void rebuildMenu();

		HWND createTrackingToolTip(HWND hwnd, const wchar_t *text);

		static std::vector<std::wstring> scan(const wchar_t *path, const ShowConfig &config);
		static DIP::Thumbs *prepareThumbs(const wchar_t *path, int width, int height, const ShowConfig &config);

		static LRESULT CALLBACK ListerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

		void processCreate(HWND hwnd);
		void processDestroy(HWND hwnd);
		void processKey(HWND hwnd, WPARAM key);
		void processCommand(HWND hwnd, UINT command);
		void processDoubleClick(HWND hwnd, int x, int y);
		void processMouseLeave(HWND hwnd);
		void processMouseMove(HWND hwnd, int x, int y, bool force = false);

		void invalidate(HWND hwnd);

		bool toggleOption(UINT command, bool &option);
		void setOption(UINT command, int &option, int value);
		bool toggleRadio(UINT command, UINT start, UINT size, unsigned int &radio, unsigned int shift = 1);

		static void switchMenuItem(HMENU menu, UINT item_id, bool value);

		void switchPopupMenuItem(UINT item_id, bool value) const;

		void loadShowConfig(const INI &ini, ShowConfig &config) const;
		void saveShowConfig(INI &ini, const ShowConfig &config) const;

		const wchar_t *tr(int key) const;

		Language m_language = LANG_EN;
		std::wstring m_log_file;

		friend class Singleton<Master>;
	};

} // namespace DIP

#endif // DIP_MASTER_H
