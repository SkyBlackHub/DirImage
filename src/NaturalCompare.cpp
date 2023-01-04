#include "NaturalCompare.h"

#include <cstddef>
#include <cctype>

static int compareNumbers(const wchar_t *&a, const wchar_t *&b)
{
	while (*a == L'0') {
		++a;
	}
	while (*b == L'0') {
		++b;
	}

	int bias = 0;

	for (;; ++a, ++b) {
		wchar_t char_a = *a;
		wchar_t char_b = *b;

		if (iswdigit(char_a) == false) {
			return iswdigit(char_b) ? -1 : bias;
		}
		if (iswdigit(char_b) == false) {
			return 1;
		}

		if (bias == 0 && char_a != char_b) {
			bias = char_a > char_b ? 1 : -1;
		}
	}
}

int DIP::naturalCompare(const wchar_t *a, const wchar_t *b, bool case_sensitive)
{
	while (iswspace(*a)) {
		++a;
	}
	while (iswspace(*b)) {
		++b;
	}

	for (;; ++a, ++b) {
		wchar_t char_a = *a;
		wchar_t char_b = *b;

		if (iswdigit(char_a) && iswdigit(char_b)) {
			if (int result = compareNumbers(a, b)) {
				return result;
			}
			char_a = *a;
			char_b = *b;
		}

		if (char_a == char_b == 0) {
			return 0;
		}

		if (case_sensitive == false) {
			char_a = towupper(char_a);
			char_b = towupper(char_b);
		}

		if (char_a != char_b) {
			return char_a > char_b ? 1 : -1;
		}
	}
}

int DIP::naturalCompare(const std::wstring &a, const std::wstring &b, bool case_sensitive)
{
	return naturalCompare(a.data(), b.data(), case_sensitive);
}