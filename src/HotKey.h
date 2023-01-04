#ifndef DIP_HOTKEY_H
#define DIP_HOTKEY_H

#include <Windows.h>

namespace DIP {

	class HotKey
	{
	public:
		HotKey(WORD key = 0, WORD modifiers = 0);
//		HotKey(const wchar_t *sequence);

		inline WORD key() const { return this->f_key; }
		inline void setKey(WORD key) { this->f_key = key; }

		inline WORD modifiers() const { return this->f_modifiers; }
		inline void setModifiers(WORD modifiers) { this->f_modifiers = modifiers; }

		inline bool isCtrl() const { return this->f_modifiers & MOD_CONTROL; }
		inline bool isShift() const { return this->f_modifiers & MOD_SHIFT; }
		inline bool isAlt() const { return this->f_modifiers & MOD_ALT; }

		bool isEqual(LPARAM parameter) const;

		HWND registeredTo() const;
		int registeredAs() const;

		bool registerTo(HWND hwnd, int id);
		bool unregister();
		bool toggleRegister(bool value) const;

		void send() const;

	private:
		static void writeInput(INPUT input[], UINT &index, WORD key, bool up = false);

		WORD f_key;
		WORD f_modifiers;
		HWND f_hwnd;
		int f_id;
	};

} // namespace DIP

#endif // DIP_HOTKEY_H
