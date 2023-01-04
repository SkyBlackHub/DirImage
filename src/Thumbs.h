#ifndef DIP_THUMBS_H
#define DIP_THUMBS_H

#include <vector>
#include <string>
#include <Windows.h>

namespace DIP {

	class Image;

	class Thumbs
	{
	public:
		Thumbs(const wchar_t *path, const std::vector<std::wstring> &files, int thumb_cols, int thumb_rows, int width = 0, int height = 0);
		~Thumbs();

		int cols() const;
		void setCols(int cols);
		int rows() const;
		void setRows(int rows);

		void reallocate(int cols, int rows);

		int width() const;
		int height() const;

		bool isAdaptive() const;
		void setAdaptive(bool adaptive);

		bool isEnlarge() const;
		void setEnlarge(bool enlarge);

		int filter() const;
		void setFilter(int filter);

		bool isShowTransparencyGrid() const;
		void setShowTransparencyGrid(bool transparency_grid);

		int horizontalPadding() const;
		void setHorizontalPadding(int padding);

		int verticalPadding() const;
		void setVerticalPadding(int padding);

		RGBQUAD background() const;
		void setBackground(const RGBQUAD &background);

		bool isEmpty() const;
		bool isLoaded() const;
		bool isUpdateRequired() const;
		bool isValid() const;

		void resize(int width, int height);

		int count() const;
		int offset() const;

		int currentPage() const;
		int pageSize() const;
		int currentPageSize() const;
		int pagesCount() const;
		int thumbsCountOnPage() const;

		bool nextPage();
		bool previousPage();
		bool firstPage();
		bool lastPage();

		HBITMAP bitmap();
		template <typename T>
		void draw(const T &destination, int x, int y);

		void drawInfo(HDC hdc, RECT &rect) const;

		std::vector<std::wstring> scan(const wchar_t *path, int files_limit) const;

		void update();

		int indexAt(int x, int y) const;
		std::wstring filenameAt(int x, int y) const;
		std::wstring fullFilenameAt(int x, int y) const;

		std::wstring filename(int index) const;
		DIP::Image * image(int index) const;

		std::wstring path() const;

		bool infoShow() const;
		void setInfoShow(bool info_show);

		RGBQUAD infoColor() const;
		void setInfoColor(const RGBQUAD &info_color);

		unsigned int infoSize() const;
		void setInfoSize(unsigned int info_size);

		int shift() const;
		void setShift(int shift);

	private:
		std::wstring m_path;
		std::vector<std::wstring> m_files;

		int m_cols;
		int m_rows;

		int m_current_cols;
		int m_current_rows;

		int m_width = 0;
		int m_height = 0;

		int m_files_limit = 0;

		bool m_adaptive = true;

		bool m_enlarge = false;
		int m_filter = 0;
		bool m_transparency_grid = true;

		int m_pad_h = 0;
		int m_pad_v = 0;

		int m_thumb_width = 0;
		int m_thumb_height = 0;
		//float m_thumb_aspect_ratio = 0;

		RGBQUAD m_background = {0, 0, 0, 0};

		bool m_info_show = false;
		RGBQUAD m_info_color = {0, 0, 0, 0};
		unsigned int m_info_size = 16;

		std::vector<DIP::Image *> m_images;
		std::vector<DIP::Image *> m_thumbs;

		int m_offset = 0;
		int m_shift = 0;

		bool m_update_required = true;
		bool m_reload_required = true;

		void reload();
		void clear();

		void recalculateThumbs();

		static void loadImage(const std::wstring &filename, DIP::Thumbs *thumbs, size_t index);
		static void updateThumb(DIP::Thumbs *thumbs, size_t index);
	};
} // namespace DIP

#endif // DIP_THUMBS_H
