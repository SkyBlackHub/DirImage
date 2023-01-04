#include "HotKey.h"
#include "Logger.h"

DIP::HotKey::HotKey(WORD key, WORD modifiers) : f_key(key), f_modifiers(modifiers) {
	this->f_hwnd = nullptr;
	this->f_id = 0;
}

//DIP::HotKey::HotKey(const wchar_t *sequence) {

//}

bool DIP::HotKey::isEqual(LPARAM parameter) const {
	return (this->f_key == HIWORD(parameter) && this->f_modifiers == LOWORD(parameter));
}

HWND DIP::HotKey::registeredTo() const {
	return this->f_hwnd;
}

int DIP::HotKey::registeredAs() const {
	return this->f_id;
}

bool DIP::HotKey::registerTo(HWND hwnd, int id) {
	if (id == 0) {
		return false;
	}
	if (this->f_id != 0) {
		this->unregister();
	}
	this->f_hwnd = hwnd;
	this->f_id = id;
	return RegisterHotKey(hwnd, id, this->f_modifiers, this->f_key);
}

bool DIP::HotKey::unregister() {
	if (this->f_id == 0) {
		return false;
	}
	bool result = UnregisterHotKey(this->f_hwnd, this->f_id);
	this->f_hwnd = nullptr;
	this->f_id = 0;
	return result;
}

bool DIP::HotKey::toggleRegister(bool value) const {
	if (this->f_id == 0) {
		return false;
	}
	if (value) {
		return RegisterHotKey(this->f_hwnd, this->f_id, this->f_modifiers, this->f_key);
	} else {
		return UnregisterHotKey(this->f_hwnd, this->f_id);
	}
}

void DIP::HotKey::writeInput(INPUT input[], UINT &index, WORD key, bool up) {
	input[index].type = INPUT_KEYBOARD;
	input[index].ki.wVk = key;
	input[index].ki.wScan = LOWORD(MapVirtualKey(key, 0));
	if (up) {
		input[index].ki.dwFlags = KEYEVENTF_KEYUP;
	}
	++index;
}

void DIP::HotKey::send() const {
	INPUT input[8];
	UINT size = 0;

	ZeroMemory(input, sizeof input);

	if (this->isShift()) {
		writeInput(input, size, VK_SHIFT);
	}
	if (this->isCtrl()) {
		writeInput(input, size, VK_CONTROL);
	}
	if (this->isAlt()) {
		writeInput(input, size, VK_MENU);
	}
	writeInput(input, size, this->f_key);
	writeInput(input, size, this->f_key, true);
	if (this->isAlt()) {
		writeInput(input, size, VK_MENU, true);
	}
	if (this->isCtrl()) {
		writeInput(input, size, VK_CONTROL, true);
	}
	if (this->isShift()) {
		writeInput(input, size, VK_SHIFT, true);
	}

	this->toggleRegister(false);
	SendInput(size, input, sizeof(INPUT));
	this->toggleRegister(true);
}