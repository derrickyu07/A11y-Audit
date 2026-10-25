#pragma once

#include <UIAutomation.h>
#include <string>
#include <windows.h>

HWND findWindowByTitle(const std::wstring& fragment);

std::wstring readName(IUIAutomationElement* element);
std::wstring readControlType(IUIAutomationElement* element);

void printTree(IUIAutomationTreeWalker* walker,
	IUIAutomationElement* element,
	int depth, int maxDepth);