#pragma once

#include <cstdio>
#include <stdexcept>
#include <string>
#include <windows.h>

// Throws if a COM call failed, including the HRESULT in hex.
inline void check(HRESULT hr, const char* what) {
	if (FAILED(hr)) {
		char code[32];
		std::snprintf(code, sizeof(code), " (HRESULT 0x%08lX)",
			static_cast<unsigned long>(hr));
		throw std::runtime_error(std::string(what) + " failed" + code);
	}
}

// Joins COM's multithreaded apartment for this thread, and leaves it on scope exit.
class ComInit {
public:
	ComInit() { check(CoInitializeEx(nullptr, COINIT_MULTITHREADED), "CoInitializeEx"); }
	~ComInit() { CoUninitialize(); }

	ComInit(const ComInit&) = delete;
	ComInit& operator=(const ComInit&) = delete;
};