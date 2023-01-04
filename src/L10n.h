#ifndef DIP_L10N_H
#define DIP_L10N_H

#include <unordered_map>

namespace DIP {

	enum Language : unsigned int {
		LANG_EN = 0,
		LANG_RU
	};

	enum {
		LOC_NEXT_PAGE,
		LOC_PREVIOUS_PAGE,
		LOC_FIRST_PAGE,
		LOC_LAST_PAGE,
		LOC_PRESETS,
		LOC_COLS,
		LOC_ROWS,
		LOC_ADAPTIVE,
		LOC_SHIFT,
		LOC_DEEP_SCAN,
		LOC_INFO,
		LOC_ENLARGE,
		LOC_TRANSPARENCY_GRID,
		LOC_VIEW_ENABLED,
		LOC_THUMBS_ENABLED,
		LOC_OPTIONS,
		LOC_LANG,
		LOC_LANG_EN,
		LOC_LANG_RU,
		LOC_SAVE_CONFIG,
		LOC_ABOUT,
		LOC_APP_NAME,
		LOC_APP_VERSION
	};

	const wchar_t *translate(Language language, int key);

} // namespace DIP

#endif // DIP_L10N_H
