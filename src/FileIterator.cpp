#include "FileIterator.h"

#include <algorithm>

static DWORD os_version = 0;

DIP::FileIterator::FileIterator(const wchar_t *path, const std::vector<std::wstring> &extensions, int limit, Mode mode) :
	m_path(path), m_extensions(extensions), m_count(0), m_limit(limit), m_mode(mode)
{
	std::wstring search_path = path + std::wstring(L"\\*");

	if (os_version == 0) {
		// store Windows version
		OSVERSIONINFO osvi;
		ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osvi);

		os_version = osvi.dwMajorVersion * 10 + osvi.dwMinorVersion;
	}

	if (os_version >= 61) { // for OS version 6.1 or greater (Windows 7 and later) we can use some extra options
		m_handle = FindFirstFileExW(search_path.data(), (FINDEX_INFO_LEVELS) FindExInfoBasic, &m_value, (FINDEX_SEARCH_OPS) 0, nullptr, FIND_FIRST_EX_LARGE_FETCH);
	} else {
		m_handle = FindFirstFileW(search_path.data(), &m_value);
	}
	if (m_handle == INVALID_HANDLE_VALUE) {
		m_handle = nullptr;
		if (GetLastError() == ERROR_FILE_NOT_FOUND) {
			return;
		}
		//$.error(L"FindFirstFile failed: (%d)", GetLastError());
		return;
	}

	if (this->verifiy(m_value) == false) {
		this->next();
	}
}

DIP::FileIterator::~FileIterator()
{
	if (m_handle) {
		FindClose(m_handle);
	}
}

const WIN32_FIND_DATA &DIP::FileIterator::value() const
{
	return m_value;
}

const std::wstring &DIP::FileIterator::filename() const
{
	return m_filename;
}

std::wstring DIP::FileIterator::fullFilename() const
{
	return m_path + std::wstring(L"\\") + m_filename;
}

bool DIP::FileIterator::next()
{
	if (m_handle == nullptr) {
		return false;
	}
	if (m_count >= m_limit && m_limit != 0) {
		FindClose(m_handle);
		m_handle = nullptr;
		return false;
	}
	while (FindNextFile(m_handle, &m_value) != 0) {
		if (this->verifiy(m_value)) {
			++m_count;
			return true;
		}
	}
	FindClose(m_handle);
	m_handle = nullptr;
	return false;
}

bool DIP::FileIterator::isValid() const
{
	return m_handle != nullptr;
}

bool DIP::FileIterator::verifiy(const WIN32_FIND_DATA &data) const
{
	if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		if (this->isFilesMode() || wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0) {
			return false;
		}
	}

	m_filename = data.cFileName;

	if (this->isDirectoriesMode()) {
		return true;
	}

	std::wstring extension = m_filename.substr(m_filename.find_last_of(L".") + 1);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	return std::find(m_extensions.begin(), m_extensions.end(), extension) != m_extensions.end();
}

const wchar_t *DIP::FileIterator::path() const
{
	return m_path;
}

DIP::FileIterator::Mode DIP::FileIterator::mode() const
{
	return m_mode;
}

void DIP::FileIterator::setMode(DIP::FileIterator::Mode mode)
{
	m_mode = mode;
}

bool DIP::FileIterator::isFilesMode() const
{
	return m_mode == MODE_FILES;
}

bool DIP::FileIterator::isDirectoriesMode() const
{
	return m_mode == MODE_DIRECTORIES;
}