#pragma once
#include "element_info.h"
#include <UIAutomation.h>
#include <string>
#include <vector>
#include <windows.h>

HWND findWindowByTitle(const std::wstring& fragment);

std::wstring readName(IUIAutomationElement* element);
std::wstring readControlType(IUIAutomationElement* element);

void printTree(IUIAutomationTreeWalker* walker,
	IUIAutomationElement* element,
	int depth, int maxDepth);

std::vector<ElementInfo> collectElements(IUIAutomationTreeWalker* walker,
	IUIAutomationElement* root,
	int maxDepth);