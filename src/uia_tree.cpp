#include "uia_tree.h"

#include <iostream>
#include <string_view>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace {

	struct SearchState {
		std::wstring fragment;
		HWND found = nullptr;
	};

	// Called by EnumWindows once per top level window. Return FALSE to stop.
	BOOL CALLBACK checkWindow(HWND hwnd, LPARAM lParam) {
		//force cast lParama to be a SearchState pointer
		auto* state = reinterpret_cast<SearchState*>(lParam);
		if (!IsWindowVisible(hwnd)) return TRUE;

		wchar_t title[256];
		int length = GetWindowTextW(hwnd, title, 256);
		//checks the title if a substring of fragment exists
		if (length > 0 &&
			std::wstring_view(title, length).find(state->fragment) != std::wstring_view::npos) {
			state->found = hwnd;
			return FALSE;
		}
		return TRUE;
	}

}

HWND findWindowByTitle(const std::wstring& fragment) {
	SearchState state{ fragment };
	//EnumWindows loops through the HWND and calls checkWindow
	//It fills state
	EnumWindows(checkWindow, reinterpret_cast<LPARAM>(&state));
	return state.found;
}

std::wstring readName(IUIAutomationElement* element) {
	BSTR raw = nullptr;
	//checks if an element exist for the current element and fills raw
	if (FAILED(element->get_CurrentName(&raw)) || !raw) return L"";
	//creates a wide string name using raw and its length
	std::wstring name(raw, SysStringLen(raw));
	SysFreeString(raw);
	return name;
}

std::wstring readControlType(IUIAutomationElement* element) {
	BSTR raw = nullptr;
	//checks the control type of the element exists and fills raw
	if (FAILED(element->get_CurrentLocalizedControlType(&raw)) || !raw) return L"";
	std::wstring name(raw, SysStringLen(raw));
	SysFreeString(raw);
	return name;
}

void printTree(IUIAutomationTreeWalker* walker,
	IUIAutomationElement* element,
	int depth, int maxDepth) {
	std::wcout << std::wstring(depth * 2, L' ')
		<< readControlType(element) << L" \"" << readName(element) << L"\"\n";
	if (depth >= maxDepth) return;

	ComPtr<IUIAutomationElement> child;
	if (FAILED(walker->GetFirstChildElement(element, &child))) {
		return;
	}

	while (child) {
		printTree(walker, child.Get(), depth + 1, maxDepth);

		ComPtr<IUIAutomationElement> nextChild;
		if (FAILED(walker->GetNextSiblingElement(child.Get(), &nextChild))) break;
		child = nextChild;
	}
}

ElementInfo readInfo(IUIAutomationElement* element, int depth, CONTROLTYPEID parentType) {
	ElementInfo info;
	info.depth = depth;
	info.parentControlType = parentType;
	info.name = readName(element);
	info.localizedType = readControlType(element);

	element->get_CurrentControlType(&info.controlType);

	BOOL value = FALSE;
	if (SUCCEEDED(element->get_CurrentIsKeyboardFocusable(&value))) info.keyboardFocusable = value != FALSE;
	if (SUCCEEDED(element->get_CurrentIsEnabled(&value)))           info.enabled = value != FALSE;
	if (SUCCEEDED(element->get_CurrentIsOffscreen(&value)))         info.offscreen = value != FALSE;
	return info;
}

void collect(IUIAutomationTreeWalker* walker,
	IUIAutomationElement* element,
	int depth, int maxDepth,
	CONTROLTYPEID parentType,
	std::vector<ElementInfo>& out) {

	out.push_back(readInfo(element, depth, parentType));
	CONTROLTYPEID myType = out.back().controlType;

	if (depth >= maxDepth) return;

	ComPtr<IUIAutomationElement> child;
	if (FAILED(walker->GetFirstChildElement(element, &child))) return;

	while (child) {
		collect(walker, child.Get(), depth + 1, maxDepth, myType, out);

		ComPtr<IUIAutomationElement> nextChild;
		if (FAILED(walker->GetNextSiblingElement(child.Get(), &nextChild))) break;
		child = nextChild;

	}

}

std::vector<ElementInfo> collectElements(IUIAutomationTreeWalker* walker,
	IUIAutomationElement* root,
	int maxDepth) {
	std::vector<ElementInfo> elementInformation;
	collect(walker, root, 0, maxDepth, 0, elementInformation);
	return elementInformation;
}

// readControlType and printTree: your turn