#include "com.h"
#include "rules.h"
#include "uia_tree.h"
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#include <algorithm>
#include <iomanip>

namespace {

	const wchar_t* severityLabel(Severity severity) {
		switch (severity) {
		case Severity::Error:   return L"ERROR";
		case Severity::Warning: return L"WARNING";
		}
		return L"UNKNOWN";
	}

	// "button \"Save\"", shortened if too long to keep columns aligned.
	std::wstring describe(const ElementInfo& element) {
		std::wstring text = element.localizedType + L" \"" + element.name + L"\"";
		const std::size_t maxLength = 32;
		if (text.size() > maxLength) {
			text = text.substr(0, maxLength - 3) + L"...";
		}
		return text;
	}

	std::size_t countSeverity(const std::vector<Finding>& findings, Severity severity) {
		return static_cast<std::size_t>(std::count_if(
			findings.begin(), findings.end(),
			[severity](const Finding& f) { return f.severity == severity; }));
	}

	// "1 error", "3 errors"
	std::wstring plural(std::size_t count, const wchar_t* word) {
		return std::to_wstring(count) + L" " + word + (count == 1 ? L"" : L"s");
	}

	void printReport(const std::vector<ElementInfo>& elements,
		const std::vector<Finding>& findings) {
		std::wstring title = elements.empty() ? L"(unknown)" : elements.front().name;
		std::wcout << L"Scanned " << elements.size() << L" elements in \"" << title << L"\"\n\n";

		for (const auto& finding : findings) {
			std::wcout << std::left
				<< std::setw(9) << severityLabel(finding.severity)
				<< std::setw(34) << describe(finding.element)
				<< std::setw(15) << finding.ruleId
				<< finding.message << L'\n';
		}
		if (!findings.empty()) {
			std::wcout << L'\n';
		}

		std::wcout << plural(countSeverity(findings, Severity::Error), L"error") << L", "
			<< plural(countSeverity(findings, Severity::Warning), L"warning") << L'\n';
	}

}  // namespace
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

		auto elements = collectElements(walker.Get(), root.Get(), 8);
		auto findings = audit(elements, defaultRules());
		printReport(elements, findings);
		return countSeverity(findings, Severity::Error) > 0 ? 2 : 0;
	}
	catch (const std::exception& e) {
		std::wcerr << L"error: " << e.what() << L'\n';
		return 1;
	}
}