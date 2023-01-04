#include "INI.h"

#include "Logger.h"

DIP::INI::INI()
{

}

DIP::INI::~INI()
{

}

static DIP::INI::Result checkError(SI_Error error, const std::wstring &filename)
{
	DIP::INI::Result result = DIP::INI::RESULT_OK;
	if (error < 0) {
		if (error == SI_FILE) {
			if (errno == ENOENT) {
				result = DIP::INI::RESULT_FILE_NOT_EXISTS;
			} else {
				wchar_t message[100];
				_wcserror_s(message, 100, errno);
				Log.error(L"Configuration file read error | filename = %s | result = %d | errno = %d | message = %s", filename.data(), error, errno, message);
			}
		} else {
			if (error == SI_NOMEM) {
				result = DIP::INI::RESULT_OUT_OF_MEMORY;
			} else {
				result = DIP::INI::RESULT_ERROR;
			}
			Log.error(L"Configuration load error | result = %d", error);
		}
	};
	return result;
}

DIP::INI::Result DIP::INI::load(const std::wstring &filename)
{
	return checkError(m_ini.LoadFile(filename.data()), filename);
}

DIP::INI::Result DIP::INI::save(const std::wstring &filename)
{
	return checkError(m_ini.SaveFile(filename.data()), filename);
}

bool DIP::INI::getBool(const wchar_t *key, bool default_value) const
{
	return m_ini.GetBoolValue(m_section, key, m_ini.GetBoolValue(m_fallback_section, key, default_value));
}

std::wstring DIP::INI::getString(const wchar_t *key, const wchar_t *default_value) const
{
	const wchar_t *result = this->getRawString(key, default_value);
	return result == nullptr ? default_value : result;
}

const wchar_t *DIP::INI::getRawString(const wchar_t *key, const wchar_t *default_value) const
{
	return m_ini.GetValue(m_section, key, m_ini.GetValue(m_fallback_section, key, default_value));
}

unsigned int DIP::INI::getUInt(const wchar_t *key, unsigned int default_value) const
{
	long result = m_ini.GetLongValue(m_section, key, m_ini.GetLongValue(m_fallback_section, key, default_value));
	return result < 0 ? default_value : result;
}

int DIP::INI::getInt(const wchar_t *key, int default_value) const
{
	return m_ini.GetLongValue(m_section, key, m_ini.GetLongValue(m_fallback_section, key, default_value));
}

double DIP::INI::getDouble(const wchar_t *key, double default_value) const
{
	return m_ini.GetDoubleValue(m_section, key, m_ini.GetDoubleValue(m_fallback_section, key, default_value));
}

std::vector<std::wstring> DIP::INI::getList(const wchar_t *key, const wchar_t *default_value, const wchar_t *separator) const
{
	return split(this->getString(key, default_value), separator);
}

unsigned int DIP::INI::getEnum(const wchar_t *key, const std::unordered_map<const wchar_t *, unsigned int> &map, unsigned int default_value, Transform transform) const
{
	const wchar_t *raw = this->getRawString(key);
	if (raw == nullptr) {
		return default_value;
	}
	std::wstring value(raw);
	this->transform(value, transform);
	for (const auto &item : map) {
		if (value == item.first) {
			return item.second;
		}
	}
	return default_value;
}

RGBQUAD DIP::INI::getColor(const wchar_t *key, const RGBQUAD &default_value) const
{
	if (const wchar_t *value = this->getRawString(key)) {
		std::vector<std::wstring> background = this->split(value);
		return {
			LOBYTE(this->convertToInt(background.at(2).data(), 0)),
			LOBYTE(this->convertToInt(background.at(1).data(), 0)),
			LOBYTE(this->convertToInt(background.at(0).data(), 0)),
		0};
	}
	return default_value;
}

void DIP::INI::readBool(const wchar_t *key, bool &target) const
{
	target = this->getBool(key, target);
}

void DIP::INI::readString(const wchar_t *key, std::wstring &target, Transform transform) const
{
	const wchar_t *value = this->getRawString(key);
	if (value == nullptr) {
		return;
	}
	target = value;
	if (transform != TRANSFORM_NONE) {
		this->transform(target, transform);
	}
}

void DIP::INI::readUInt(const wchar_t *key, unsigned int &target) const
{
	target = this->getUInt(key, target);
}

void DIP::INI::readList(const wchar_t *key, std::vector<std::wstring> &target, Transform transform, const wchar_t *separator) const
{
	const wchar_t *value = this->getRawString(key);
	if (value == nullptr) {
		return;
	}
	target = split(value, separator);
	if (transform == TRANSFORM_NONE) {
		return;
	}
	for (int i = 0; i < target.size(); ++i) {
		std::wstring &item = target.at(i);
		this->transform(item, transform);
	}
}

void DIP::INI::readEnum(const wchar_t *key, const std::unordered_map<const wchar_t *, unsigned int> &map, unsigned int &target, Transform transform) const
{
	target = this->getEnum(key, map, target, transform);
}

void DIP::INI::readColor(const wchar_t *key, RGBQUAD &target) const
{
	target = this->getColor(key, target);
}

void DIP::INI::setBool(const wchar_t *key, bool value)
{
	m_ini.SetBoolValue(m_section, key, value);
}

void DIP::INI::setString(const wchar_t *key, const wchar_t *value)
{
	m_ini.SetValue(m_section, key, value);
}

void DIP::INI::setString(const wchar_t *key, const std::wstring &value)
{
	m_ini.SetValue(m_section, key, value.data());
}

void DIP::INI::setUInt(const wchar_t *key, unsigned int value)
{
	m_ini.SetLongValue(m_section, key, value);
}

void DIP::INI::setInt(const wchar_t *key, int value)
{
	m_ini.SetLongValue(m_section, key, value);
}

void DIP::INI::setDouble(const wchar_t *key, double value)
{
	m_ini.SetDoubleValue(m_section, key, value);
}

void DIP::INI::setList(const wchar_t *key, const std::vector<std::wstring> &value, const wchar_t *separator)
{
	std::wstring temp;
	for (const std::wstring &item : value) {
		if (temp.empty()) {
			temp += item;
		} else {
			temp += separator + item;
		}
	}
	this->setString(key, temp.data());
}

void DIP::INI::setEnum(const wchar_t *key, const std::unordered_map<const wchar_t *, unsigned int> &map, unsigned int value)
{
	for (const auto &item : map) {
		if (value == item.second) {
			this->setString(key, item.first);
			return;
		}
	}
}

void DIP::INI::setColor(const wchar_t *key, const RGBQUAD &value)
{
	WCHAR buffer[12];
	swprintf_s(buffer, ARRAYSIZE(buffer), L"%d %d %d", value.rgbRed, value.rgbGreen, value.rgbBlue);
	this->setString(key, buffer);
}

int DIP::INI::convertToInt(const wchar_t *value, int default_value)
{
	int result;
	wchar_t *check = const_cast<wchar_t *>(value);
	if (value[0] == '0' && (value[1] == 'x' || value[1] == 'X')) {
		if (value[2] == 0) {
			return default_value;
		}
		result = wcstol(&value[2], &check, 16);
	} else {
		result = wcstol(value, &check, 10);
	}
	return *check ? default_value : result;
}

std::vector<std::wstring> DIP::INI::split(const std::wstring &text, const wchar_t *separator)
{
	size_t start = 0;
	size_t end = 0;
	std::vector<std::wstring> result;
	while ((end = text.find(separator, start)) != std::wstring::npos) {
		if (end - start > 0) {
			result.push_back(text.substr(start, end - start));
		}
		start = end + 1;
	}
	result.push_back(text.substr(start));
	return result;
}

void DIP::INI::transform(std::wstring &target, Transform transform)
{
	if (transform == TRANSFORM_NONE) {
		return;
	}
	std::transform(target.begin(), target.end(), target.begin(), transform == TRANSFORM_UPPER ? ::toupper : ::tolower);
}

const wchar_t *DIP::INI::section() const
{
	return m_section;
}

void DIP::INI::setSection(const wchar_t *section)
{
	m_section = section;
}

const wchar_t *DIP::INI::fallbackSection() const
{
	return m_fallback_section;
}

void DIP::INI::setFallbackSection(const wchar_t *fallback_section)
{
	m_fallback_section = fallback_section;
}