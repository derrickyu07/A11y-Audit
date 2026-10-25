#include "com.h"
#include "uia_tree.h"
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

int wmain(int argc, wchar_t* argv[]) {
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);
	if (argc < 2) {
		std::wcerr << L"usage: a11y-audit <part of window title>\n";
		return 1;
	}

	try {
		ComInit com;   // must be declared first, so it is destroyed last

		ComPtr<IUIAutomation> automation;
		check(CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&automation)),
			"CoCreateInstance(CUIAutomation)");

		HWND hwnd = findWindowByTitle(argv[1]);
		if (!hwnd) {
			std::wcerr << L"No visible window matching \"" << argv[1] << L"\"\n";
			return 1;
		}
		//root is the starting element
		ComPtr<IUIAutomationElement> root;
		check(automation->ElementFromHandle(hwnd, &root), "ElementFromHandle");
		//walker is used to navigate the tree
		ComPtr<IUIAutomationTreeWalker> walker;
		check(automation->get_ControlViewWalker(&walker), "get_ControlViewWalker");

		printTree(walker.Get(), root.Get(), 0, 8);
	}
	catch (const std::exception& e) {
		std::wcerr << L"error: " << e.what() << L'\n';
		return 1;
	}
}