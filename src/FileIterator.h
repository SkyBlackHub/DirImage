#ifndef DIP_FILEITERATOR_H
#define DIP_FILEITERATOR_H

#include <vector>
#include <string>
#include <Windows.h>

namespace DIP {

	class FileIterator
	{
	public:
		enum Mode {
			MODE_FILES,
			MODE_DIRECTORIES
		};

		FileIterator(const wchar_t *path, const std::vector<std::wstring> &extensions = std::vector<std::wstring>(), int limit = 0, Mode mode = MODE_FILES);
		~FileIterator();
		const WIN32_FIND_DATA &value() const;
		const std::wstring &filename() const;
		std::wstring fullFilename() const;
		bool next();
		bool isValid() const;

		Mode mode() const;
		void setMode(Mode mode);
		bool isFilesMode() const;
		bool isDirectoriesMode() const;

		const wchar_t *path() const;

	private:
		bool verifiy(const WIN32_FIND_DATA &data) const;

		const wchar_t *m_path;
		HANDLE m_handle;
		WIN32_FIND_DATA m_value;
		mutable std::wstring m_filename;
		const std::vector<std::wstring> m_extensions;
		int m_count;
		int m_limit;
		Mode m_mode;
	};

} // namespace DIP

#endif // DIP_FILEITERATOR_H