#include "Thumbs.h"

#include "Image.h"
#include "Logger.h"

#include <thread>
#include <cmath>
#include <algorithm>

DIP::Thumbs::Thumbs(const wchar_t *path, const std::vector<std::wstring> &files, int thumb_cols, int thumb_rows, int width, int height) :
	m_path(path), m_files(files), m_cols(thumb_cols), m_rows(thumb_rows)
{
	m_filter = DIP_IMAGE_FILTER_BOX;
	this->resize(width, height);
}

DIP::Thumbs::~Thumbs()
{
	this->clear();
}

int DIP::Thumbs::cols() const
{
	return m_cols;
}

void DIP::Thumbs::setCols(int cols)
{
	m_cols = cols;
	this->recalculateThumbs();
	m_reload_required = true;
}

int DIP::Thumbs::rows() const
{
	return m_rows;
}

void DIP::Thumbs::setRows(int rows)
{
	m_rows = rows;
	this->recalculateThumbs();
	m_reload_required = true;
}

void DIP::Thumbs::reallocate(int cols, int rows)
{
	m_cols = cols;
	m_rows = rows;
	this->recalculateThumbs();
	m_reload_required = true;
}

int DIP::Thumbs::width() const
{
	return m_width;
}

int DIP::Thumbs::height() const
{
	return m_height;
}

bool DIP::Thumbs::isAdaptive() const
{
	return m_adaptive;
}

void DIP::Thumbs::setAdaptive(bool adaptive)
{
	m_adaptive = adaptive;
	this->recalculateThumbs();
}

int DIP::Thumbs::indexAt(int x, int y) const
{
	int result = y / (m_thumb_height + m_pad_v) * m_current_cols + x / (m_thumb_width + m_pad_h) + this->offset();
	return result < this->count() ? result : -1;
}

std::wstring DIP::Thumbs::filenameAt(int x, int y) const
{
	return this->filename(this->indexAt(x, y));
}

std::wstring DIP::Thumbs::fullFilenameAt(int x, int y) const
{
	std::wstring filename = this->filenameAt(x, y);
	return filename.empty() ? filename : m_path + std::wstring(L"\\") + filename;
}

std::wstring DIP::Thumbs::filename(int index) const
{
	return (index >= 0 && index < this->count()) ? m_files.at(index) : std::wstring();
}

DIP::Image *DIP::Thumbs::image(int index) const
{
	index -= this->offset();
	return (index >= 0 && index < static_cast<int>(m_images.size())) ? m_images.at(index) : nullptr;
}

std::wstring DIP::Thumbs::path() const
{
	return m_path;
}

bool DIP::Thumbs::infoShow() const
{
	return m_info_show;
}

void DIP::Thumbs::setInfoShow(bool info_show)
{
	m_info_show = info_show;
}

RGBQUAD DIP::Thumbs::infoColor() const
{
	return m_info_color;
}

void DIP::Thumbs::setInfoColor(const RGBQUAD &info_color)
{
	m_info_color = info_color;
}

unsigned int DIP::Thumbs::infoSize() const
{
	return m_info_size;
}

void DIP::Thumbs::setInfoSize(unsigned int info_size)
{
	m_info_size = info_size;
}

int DIP::Thumbs::shift() const
{
	return m_shift;
}

void DIP::Thumbs::setShift(int shift)
{
	if (shift != m_shift) {
		m_shift = shift;
		m_reload_required = true;
	}
}

bool DIP::Thumbs::isEnlarge() const
{
	return m_enlarge;
}

void DIP::Thumbs::clear()
{
	for (auto image : m_images) {
		delete image;
	}
	m_images.clear();
	for (auto thumb : m_thumbs) {
		delete thumb;
	}
	m_thumbs.clear();
}

void DIP::Thumbs::loadImage(const std::wstring &filename, DIP::Thumbs *thumbs, size_t index)
{
	if (index >= thumbs->m_images.size()) {
		return;
	}

	DIP::Image *image;
	try {
		image = new DIP::Image(filename.data());
	} catch (const std::exception &exception) {
		Log.error(L"Image load exception | %s", exception.what());
		return;
	}

	if (image->isInitialized() == 0) {
		Log.info(L"Image cannot been loaded, so skipped | filename = %s", filename.data());
		delete image;
		return;
	}

	Log.debug(L"Image has been loaded | filename = %s", filename.data());

	thumbs->m_images[index] = image;

	updateThumb(thumbs, index);
}

void DIP::Thumbs::updateThumb(DIP::Thumbs *thumbs, size_t index)
{
	if (index >= thumbs->m_images.size() || index >= thumbs->m_thumbs.size()) {
		return;
	}

	DIP::Image *image = thumbs->m_images.at(index);
	if (image == nullptr) {
		return;
	}

	DIP::Image *&thumb = thumbs->m_thumbs[index];

	if (thumb) {
		delete thumb;
		thumb = nullptr;
	}

	if (thumbs->m_enlarge || image->width() > thumbs->m_thumb_width || image->height() > thumbs->m_thumb_height) {
		thumb = image->inscribed(lround(thumbs->m_thumb_width), lround(thumbs->m_thumb_height));
	}

	if (thumbs->m_transparency_grid && image->hasAlpha()) {
		if (thumb) {
			DIP::Image *temp = thumb;
			thumb = temp->composited();
			delete temp;
		} else {
			thumb = image->composited();
		}
	}
}

void DIP::Thumbs::reload()
{
	if (this->isEmpty()) {
		m_reload_required = false;
		return;
	}

	this->clear();

	int count =this->thumbsCountOnPage();

	if (count == 0) {
		m_reload_required = false;
		return;
	}

	m_images.assign(count, nullptr);
	m_thumbs.assign(count, nullptr);

	if (m_adaptive) {
		this->recalculateThumbs();
	}

	std::vector<std::thread> threads;
	threads.reserve(count);

	for (int i = 0; i < count; ++i) {
		std::wstring filename = m_path + m_files[this->offset() + i];
		threads.push_back(std::thread(loadImage, filename, this, i));
	}

	for (int i = 0; i < count; ++i) {
		threads.at(i).join();
	}

	m_reload_required = false;
}

void DIP::Thumbs::update()
{
	if (m_update_required == false && m_reload_required == false) {
		return;
	}
	if (this->isEmpty() || this->isValid() == false) {
		return;
	}

	if (m_reload_required || this->isLoaded() == false) {
		this->reload();
		m_update_required = false;
		return;
	}

	size_t size = m_images.size();

	std::vector<std::thread> threads;
	threads.reserve(size);

	for (size_t i = 0; i < size; ++i) {
		threads.push_back(std::thread(updateThumb, this, i));
	}

	for (size_t i = 0; i < threads.size(); ++i) {
		threads.at(i).join();
	}

	m_update_required = false;
}

void DIP::Thumbs::setEnlarge(bool enlarge)
{
	if (m_enlarge != enlarge) {
		m_enlarge = enlarge;
		m_update_required = true;
	}
}

int DIP::Thumbs::filter() const
{
	return m_filter;
}

void DIP::Thumbs::setFilter(int filter)
{
	m_filter = filter;
}

bool DIP::Thumbs::isShowTransparencyGrid() const
{
	return m_transparency_grid;
}

void DIP::Thumbs::setShowTransparencyGrid(bool transparency_grid)
{
	if (m_transparency_grid != transparency_grid) {
		m_transparency_grid = transparency_grid;
		m_update_required = true;
	}
}

int DIP::Thumbs::horizontalPadding() const
{
	return m_pad_h;
}

void DIP::Thumbs::setHorizontalPadding(int padding)
{
	m_pad_h = padding;
	this->recalculateThumbs();
}

int DIP::Thumbs::verticalPadding() const
{
	return m_pad_v;
}

void DIP::Thumbs::setVerticalPadding(int padding)
{
	m_pad_v = padding;
	this->recalculateThumbs();
}

RGBQUAD DIP::Thumbs::background() const
{
	return m_background;
}

void DIP::Thumbs::setBackground(const RGBQUAD &background)
{
	m_background = background;
}

bool DIP::Thumbs::isEmpty() const
{
	return m_files.empty();
}

bool DIP::Thumbs::isLoaded() const
{
	return m_images.size() != 0 || m_reload_required == false;
}

bool DIP::Thumbs::isUpdateRequired() const
{
	return m_update_required;
}

bool DIP::Thumbs::isValid() const
{
	return m_width > 0 && m_height > 0 && m_cols > 0 && m_rows > 0;
}

void DIP::Thumbs::resize(int width, int height)
{
	if (width == m_width && height == m_height) {
		return;
	}

	m_width = width;
	m_height = height;

	this->recalculateThumbs();
}

int DIP::Thumbs::count() const
{
	return static_cast<int>(m_files.size());
}

int DIP::Thumbs::offset() const
{
	return m_offset + m_shift;
}

int DIP::Thumbs::currentPage() const
{
	return m_offset / this->pageSize();
}

int DIP::Thumbs::pageSize() const
{
	return m_cols * m_rows;
}

int DIP::Thumbs::currentPageSize() const
{
	return m_current_cols * m_current_rows;
}

int DIP::Thumbs::pagesCount() const
{
	int size = this->pageSize();
	return (this->count() + size - 1 - m_shift) / size;
}

int DIP::Thumbs::thumbsCountOnPage() const
{
	return max(0, min(this->pageSize(), this->count() - this->offset()));
}

bool DIP::Thumbs::nextPage()
{
	int size = this->pageSize();
	if (m_offset + size >= this->count() - m_shift) {
		return false;
	}
	m_offset += size;
	if (m_adaptive) {
		this->recalculateThumbs();
	}
	m_reload_required = true;
	return true;
}

bool DIP::Thumbs::previousPage()
{
	int size = this->pageSize();
	if (size >= m_offset) {
		if (m_offset == 0) {
			return false;
		}
		m_offset = 0;
	} else {
		m_offset -= this->pageSize();
	}
	if (m_adaptive) {
		this->recalculateThumbs();
	}
	m_reload_required = true;
	return true;
}

bool DIP::Thumbs::firstPage()
{
	if (m_offset == 0) {
		return false;
	}
	m_offset = 0;
	if (m_adaptive) {
		this->recalculateThumbs();
	}
	m_reload_required = true;
	return true;
}

bool DIP::Thumbs::lastPage()
{
	int offset = (this->pagesCount() - 1) * this->pageSize();
	if (offset == m_offset) {
		return false;
	}
	m_offset = offset;
	if (m_adaptive) {
		this->recalculateThumbs();
	}
	m_reload_required = true;
	return true;
}

void DIP::Thumbs::recalculateThumbs()
{
	if (m_width == 0 || m_height == 0) {
		m_thumb_width = 0;
		m_thumb_height = 0;
		m_update_required = true;
		return;
	}

	int count = this->thumbsCountOnPage();

	if (m_adaptive && count < this->pageSize() && count && m_height) {
		m_current_cols = lround(std::sqrt(static_cast<float>(m_width) / m_height * count));
		if (m_current_cols == 0) {
			m_current_cols = 1;
		}
		m_current_rows = lround(ceil(static_cast<float>(count) / m_current_cols));
	} else {
		m_current_cols = m_cols;
		m_current_rows = m_rows;
	}

	m_thumb_width = m_current_cols > 0 ? (m_width - (m_current_cols - 1) * m_pad_h) / m_current_cols : 0;
	m_thumb_height = m_current_rows > 0 ? (m_height - (m_current_rows - 1) * m_pad_v) / m_current_rows : 0;

	//m_thumb_aspect_ratio = m_thumb_height > 0 ? static_cast<float>(m_thumb_width) / m_thumb_height : 0;

	m_update_required = true;
}

HBITMAP DIP::Thumbs::bitmap()
{
	DIP::Image canvas(m_width, m_height, m_background);
	this->draw(canvas, 0, 0);
	return canvas.bitmap();
}

void DIP::Thumbs::drawInfo(HDC hdc, RECT &rect) const
{
	SetTextColor(hdc, RGB(m_info_color.rgbRed, m_info_color.rgbGreen, m_info_color.rgbBlue));
	SetBkMode(hdc, TRANSPARENT);

	HFONT hFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT log_font;
	GetObject(hFont, sizeof(LOGFONT), &log_font);

	log_font.lfHeight = m_info_size;

	HFONT new_font = CreateFontIndirect(&log_font);
	HFONT old_font = (HFONT) SelectObject(hdc, new_font);

	WCHAR text[52];
	if (m_images.size()) {
		swprintf_s(text, ARRAYSIZE(text), L"%d / %d (%d - %d / %d)", this->currentPage() + 1, this->pagesCount(), this->offset() + 1, this->offset() + this->thumbsCountOnPage(), this->count());
	} else {
		swprintf_s(text, ARRAYSIZE(text), L"0 / 0 (0 - 0 / %d)", this->count());
	}
	DrawText(hdc, text, -1, &rect, DT_SINGLELINE | DT_NOCLIP);

	SelectObject(hdc, old_font);
	DeleteObject(new_font);
}

template <typename T>
void DIP::Thumbs::draw(const T &destination, int x, int y)
{
	this->update();

	int tx = 0;
	int ty = 0;

	size_t size = m_images.size();

	for (size_t i = 0; i < size; ++i) {
		DIP::Image *thumb = m_thumbs.at(i);
		if (thumb == nullptr) {
			thumb = m_images.at(i);
		}
		if (thumb == nullptr) {
			continue;
		}

		thumb->draw(
			destination,
			x + tx * (m_thumb_width + m_pad_h) + (m_thumb_width - thumb->width()) / 2,
			y + ty * (m_thumb_height + m_pad_v) + (m_thumb_height - thumb->height()) / 2
		);

		++tx;
		if (tx >= m_current_cols) {
			++ty;
			tx = 0;
		}
	}
}

template void DIP::Thumbs::draw<HDC>(const HDC&, int x, int y);
template void DIP::Thumbs::draw<DIP::Image>(const DIP::Image&, int x, int y);