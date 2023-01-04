#ifndef DIP_NATURALCOMPARE_H
#define DIP_NATURALCOMPARE_H

#include <string>

namespace DIP {

	int naturalCompare(const wchar_t *a, const wchar_t *b, bool case_sensitive = false);
	int naturalCompare(const std::wstring &a, const std::wstring &b, bool case_sensitive = false);

} // namespace DIP

#endif // DIP_NATURALCOMPARE_H