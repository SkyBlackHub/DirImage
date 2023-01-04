#include "Master.h"
#include "Logger.h"

extern "C" {

	BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
	{
		switch (ul_reason_for_call)	{
			case DLL_PROCESS_ATTACH: {
				DIP::Master::deinitialize();
				DIP::Logger::deinitialize();

				// find out a base path
				std::wstring basepath;
				wchar_t path[MAX_PATH];
				HMODULE hm = nullptr;
				if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR) &DllMain, &hm) != 0) {
					GetModuleFileNameW(hm, path, sizeof(path));

					wchar_t drive[_MAX_DRIVE];
					wchar_t dir[_MAX_PATH];
					wchar_t fname[_MAX_FNAME];
					wchar_t ext[_MAX_EXT];

					_wsplitpath_s(path, drive, dir, fname, ext);
					basepath = std::wstring(drive) + dir;
				}

				DIP::Master::initialize(static_cast<HINSTANCE>(hModule), basepath);
				break;
			}

			case DLL_PROCESS_DETACH:
				DIP::Master::deinitialize();
				DIP::Logger::deinitialize();
				break;

			case DLL_THREAD_ATTACH:
				break;

			case DLL_THREAD_DETACH:
				break;
		}
		return true;
	}

	HWND __stdcall ListLoadW(HWND ParentWin, WCHAR *FileToLoad, int /*ShowFlags*/)
	{
		auto &master = DIP::Master::instance();
		if (master.isViewEnabled() == false) {
			return nullptr;
		}
		if ((GetFileAttributesW(FileToLoad) & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			return nullptr;
		}
		RECT rect;
		GetClientRect(ParentWin, &rect);
		HWND handle = master.generateView(FileToLoad, ParentWin, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
		if (handle) {
			ShowWindow(handle, SW_SHOW);
		}
		return handle;
	}

	HWND __stdcall ListLoad(HWND /*ParentWin*/, CHAR */*FileToLoad*/, int /*ShowFlags*/)
	{
		// it's must be here - otherwise crash :(
		return nullptr;
	}

	HBITMAP __stdcall ListGetPreviewBitmapW(WCHAR *FileToLoad, int width, int height, char */*contentbuf*/, int contentbuflen)
	{
		auto &master = DIP::Master::instance();
		if (master.isThumbsEnabled() == false) {
			return nullptr;
		}
		if (contentbuflen >= 0) {
			// we are looking only for folders, which contentbuflen is always -1
			return nullptr;
		}
		return master.generateThumbs(FileToLoad, width, height);
	}

}