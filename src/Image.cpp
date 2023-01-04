#include "Image.h"

// FreeImage type is abstracted for a possible future replace
#ifdef _WIN64
#include "FreeImage/x64/FreeImage.h"
#else
#include "FreeImage/x32/FreeImage.h"
#endif

#define FIT FIBITMAP
#define FID static_cast<FIT *>(m_data)
#define FIDF(source) static_cast<FIT *>(source)

DIP::Image::Image()
{
	m_data = nullptr;
	m_width = 0;
	m_height = 0;
}

DIP::Image::Image(int width, int height, BYTE red, BYTE green, BYTE blue)
{
	m_data = FreeImage_Allocate(width, height, 32);
	this->updateMetrics();

	RGBQUAD color;
	color.rgbRed = red;
	color.rgbGreen = green;
	color.rgbBlue = blue;
	color.rgbReserved = 0;

	FreeImage_FillBackground(FID, &color);
}

DIP::Image::Image(int width, int height, const RGBQUAD &color)
{
	m_data = FreeImage_Allocate(width, height, 32);
	this->updateMetrics();

	FreeImage_FillBackground(FID, &color);
}

DIP::Image::Image(const wchar_t *filename)
{
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeU(filename, 0);
	if (fif == FIF_UNKNOWN) {
		fif = FreeImage_GetFIFFromFilenameU(filename);
	}
	if ((fif != FIF_UNKNOWN) && FreeImage_FIFSupportsReading(fif)) {
		m_data = FreeImage_LoadU(fif, filename);
		this->updateMetrics();
		return;
	}
	m_data = nullptr;
	m_width = 0;
	m_height = 0;
}

void DIP::Image::updateMetrics()
{
	m_width = FreeImage_GetWidth(FID);
	m_height = FreeImage_GetHeight(FID);
}

DIP::Image::~Image()
{
	FreeImage_Unload(FID);
}

bool DIP::Image::isInitialized() const
{
	return m_data;
}

int DIP::Image::width() const
{
	return m_width;
}

int DIP::Image::height() const
{
	return m_height;
}

unsigned int DIP::Image::bpp() const
{
	return FreeImage_GetBPP(FID);
}

bool DIP::Image::hasAlpha() const
{
	return FreeImage_IsTransparent(FID);
}

HBITMAP DIP::Image::bitmap() const
{
	HDC hDC = GetDC(nullptr);
	HBITMAP bitmap = CreateDIBitmap(hDC, FreeImage_GetInfoHeader(FID), CBM_INIT, FreeImage_GetBits(FID), FreeImage_GetInfo(FID), DIB_RGB_COLORS);
	DeleteDC(hDC);
	return bitmap;
}

void DIP::Image::draw(HDC hdc, int x, int y) const
{
	SetDIBitsToDevice(hdc, x, y, static_cast<DWORD>(m_width), static_cast<DWORD>(m_height), 0, 0, 0, static_cast<DWORD>(m_height), FreeImage_GetBits(FID), FreeImage_GetInfo(FID), DIB_RGB_COLORS);
}

void DIP::Image::draw(const DIP::Image &destination, int x, int y) const
{
	FreeImage_Paste(FIDF(destination.m_data), FID, x, y, 255);
}

void DIP::Image::draw(const DIP::Image &destination, int x, int y, int width, int height, int filter) const
{
	if (width == m_width && height == m_height) {
		FreeImage_Paste(FIDF(destination.m_data), FID, x, y, 255);
		return;
	}
	FIBITMAP *scaled = FreeImage_Rescale(FID, width, height, static_cast<FREE_IMAGE_FILTER>(filter));
	FreeImage_Paste(FIDF(destination.m_data), scaled, x, y, 255);
	FreeImage_Unload(scaled);
}

DIP::Image *DIP::Image::scaled(int width, int height, int filter) const
{
	DIP::Image *image = new DIP::Image();
	image->m_width = width;
	image->m_height = height;
	image->m_data = FreeImage_Rescale(FID, width, height, static_cast<FREE_IMAGE_FILTER>(filter));
	return image;
}

DIP::Image *DIP::Image::inscribed(int width, int height, int filter) const
{
	std::pair<int, int> size = this->inscribe(width, height);
	return this->scaled(size.first, size.second, filter);
}

DIP::Image *DIP::Image::composited(bool checkerboard) const
{
	DIP::Image *image = new DIP::Image();
	image->m_width = m_width;
	image->m_height = m_height;
	image->m_data = FreeImage_Composite(FID, !checkerboard);
	return image;
}

DIP::Image *DIP::Image::composited(BYTE red, BYTE green, BYTE blue) const
{
	DIP::Image *image = new DIP::Image();
	image->m_width = m_width;
	image->m_height = m_height;

	RGBQUAD color;
	color.rgbRed = red;
	color.rgbGreen = green;
	color.rgbBlue = blue;
	color.rgbReserved = 0;

	image->m_data = FreeImage_Composite(FID, false, &color);
	return image;
}

DIP::Image *DIP::Image::composited(const DIP::Image &background) const
{
	DIP::Image *image = new DIP::Image();
	image->m_width = m_width;
	image->m_height = m_height;
	image->m_data = FreeImage_Composite(FID, false, nullptr, FIDF(background.m_data));
	return image;
}

std::wstring DIP::Image::version()
{
	std::string buffer;
	buffer += "FreeImage version: ";
	buffer += FreeImage_GetVersion();
	buffer += "\n\n";
	buffer += FreeImage_GetCopyrightMessage();
	return std::wstring(buffer.begin(), buffer.end());
}

std::pair<int, int> DIP::Image::inscribe(int width, int height) const
{
	float ratio = (static_cast<float>(m_width) / m_height) / (static_cast<float>(width) / height);

	if (ratio > 1) {
		return {width, static_cast<int>(static_cast<float>(width) / m_width * m_height)};
	}	else if (ratio < 1) {
		return {static_cast<int>(static_cast<float>(height) / m_height * m_width), height};
	}

	return {width, height};
}