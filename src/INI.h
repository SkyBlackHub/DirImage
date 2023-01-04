#ifndef DIP_INI_H
#define DIP_INI_H

#include "SimpleINI/SimpleIni.h"

#include <vector>
#include <unordered_map>

namespace DIP {

	class INI
	{
	public:
		enum Result
		{
			RESULT_OK,
			RESULT_FILE_NOT_EXISTS,
			RESULT_OUT_OF_MEMORY,
			RESULT_ERROR
		};

		enum Transform {
			TRANSFORM_NONE,
			TRANSFORM_UPPER,
			TRANSFORM_LOWER
		};

		INI();
		~INI();

		Result load(const std::wstring &filename);
		Result save(const std::wstring &filename);

		bool getBool(const wchar_t *key, bool default_value = false) const;
		std::wstring getString(const wchar_t *key, const wchar_t *default_value = nullptr) const;
		const wchar_t *getRawString(const wchar_t *key, const wchar_t *default_value = nullptr) const;
		unsigned int getUInt(const wchar_t *key, unsigned int default_value = 0) const;
		int getInt(const wchar_t *key, int default_value = 0) const;
		double getDouble(const wchar_t *key, double default_value = false) const;
		std::vector<std::wstring> getList(const wchar_t *key, const wchar_t *default_value = nullptr, const wchar_t *separator = L" ") const;
		unsigned int getEnum(const wchar_t *key, const std::unordered_map<const wchar_t *, unsigned int> &map, unsigned int default_value = 0, Transform transform = TRANSFORM_LOWER) const;
		RGBQUAD getColor(const wchar_t *key, const RGBQUAD &default_value = {0, 0, 0, 0}) const;

		void readBool(const wchar_t *key, bool &target) const;
		void readString(const wchar_t *key, std::wstring &target, Transform transform = TRANSFORM_NONE) const;
		void readUInt(const wchar_t *key, unsigned int &target) const;
		void readList(const wchar_t *key, std::vector<std::wstring> &target, Transform transform = TRANSFORM_NONE, const wchar_t *separator = L" ") const;
		void readEnum(const wchar_t *key, const std::unordered_map<const wchar_t *, unsigned int> &map, unsigned int &target, Transform transform = TRANSFORM_LOWER) const;
		void readColor(const wchar_t *key, RGBQUAD &target) const;

		void setBool(const wchar_t *key, bool value);
		void setString(const wchar_t *key, const wchar_t *value);
		void setString(const wchar_t *key, const std::wstring &value);
		void setUInt(const wchar_t *key, unsigned int value);
		void setInt(const wchar_t *key, int value);
		void setDouble(const wchar_t *key, double value);
		void setList(const wchar_t *key, const std::vector<std::wstring> &value, const wchar_t *separator = L" ");
		void setEnum(const wchar_t *key, const std::unordered_map<const wchar_t *, unsigned int> &map, unsigned int value);
		void setColor(const wchar_t *key, const RGBQUAD &value);

		static int convertToInt(const wchar_t *value, int default_value = 0);
		static std::vector<std::wstring> split(const std::wstring &text, const wchar_t *separator = L" ");
		static void transform(std::wstring &target, Transform transform);

		const wchar_t *section() const;
		void setSection(const wchar_t *section);

		const wchar_t *fallbackSection() const;
		void setFallbackSection(const wchar_t *fallback_section);

	private:
		CSimpleIniCaseW m_ini;
		const wchar_t *m_section = L"general";
		const wchar_t *m_fallback_section = nullptr;
	};

} // namespace DIP

#endif // DIP_INI_H