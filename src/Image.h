#ifndef DIP_IMAGE_H
#define DIP_IMAGE_H

#include <Windows.h>
#include <string>

#define DIP_IMAGE_FILTER_BOX        0
#define DIP_IMAGE_FILTER_BICUBIC    1
#define DIP_IMAGE_FILTER_BILINEAR   2
#define DIP_IMAGE_FILTER_BSPLINE    3
#define DIP_IMAGE_FILTER_CATMULLROM 4
#define DIP_IMAGE_FILTER_LANCZOS3   5

namespace DIP {

	// the wrapper class for a image-processing library
	class Image
	{
	public:
		Image();
		Image(int width, int height, BYTE red = 0, BYTE green = 0, BYTE blue = 0);
		Image(int width, int height, const RGBQUAD &color);
		Image(const wchar_t *filename);
		~Image();

		bool isInitialized() const;

		int width() const;
		int height() const;
		unsigned int bpp() const;

		bool hasAlpha() const;

		HBITMAP bitmap() const;
		void draw(HDC hdc, int x, int y) const;
		void draw(const DIP::Image &destination, int x, int y) const;
		void draw(const DIP::Image &destination, int x, int y, int width, int height, int filter = DIP_IMAGE_FILTER_BOX) const;

		std::pair<int, int> inscribe(int width, int height) const;

		DIP::Image *scaled(int width, int height, int filter = DIP_IMAGE_FILTER_BOX) const;
		DIP::Image *inscribed(int width, int height, int filter = DIP_IMAGE_FILTER_BOX) const;

		DIP::Image *composited(bool checkerboard = true) const;
		DIP::Image *composited(BYTE red, BYTE green, BYTE blue) const;
		DIP::Image *composited(const DIP::Image &background) const;

		static std::wstring version();

	private:
		void updateMetrics();

		void *m_data;
		int m_width;
		int m_height;
	};

} // namespace DIP

#endif // DIP_IMAGE_H
