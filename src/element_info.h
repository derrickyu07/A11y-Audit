#pragma once

#include <UIAutomation.h>
#include <string>
#include <windows.h>

// A snapshot of one UIA element: everything the rules need, no COM pointers.
struct ElementInfo {
	CONTROLTYPEID controlType = 0;
	CONTROLTYPEID parentControlType = 0;
	std::wstring localizedType;
	std::wstring name;
	bool keyboardFocusable = false;
	bool enabled = true;
	bool offscreen = false;
	int depth = 0;
};