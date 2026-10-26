#include "rules.h"

bool isInteractive(CONTROLTYPEID type) {
	switch (type) {
	case UIA_ButtonControlTypeId:
	case UIA_CheckBoxControlTypeId:
	case UIA_HyperlinkControlTypeId:
	case UIA_RadioButtonControlTypeId:
	case UIA_EditControlTypeId:
	case UIA_ComboBoxControlTypeId:
	case UIA_SliderControlTypeId:
	case UIA_SpinnerControlTypeId:
	case UIA_ListControlTypeId:
	case UIA_ListItemControlTypeId:
	case UIA_TreeItemControlTypeId:
	case UIA_TabItemControlTypeId:
	case UIA_DataItemControlTypeId:
	case UIA_MenuItemControlTypeId:
	case UIA_SplitButtonControlTypeId:
		return true;
	default:
		return false;
	}
}

bool isBlank(const std::wstring& text) {
	return text.find_first_not_of(L" \t") == std::wstring::npos;
}

std::optional<Issue> MissingNameRule::check(const ElementInfo& element) const {
	if (isInteractive(element.controlType) && isBlank(element.name)) {
		return Issue{ Severity::Error, L"Interactive control has no accessible name" };
	}
	return std::nullopt;
}

std::optional<Issue> NotKeyboardFocusableRule::check(const ElementInfo& element) const {
	if (isInteractive(element.controlType) && element.enabled && !element.keyboardFocusable && !element.offscreen && element.parentControlType
		!= UIA_TitleBarControlTypeId && element.controlType != UIA_MenuItemControlTypeId) {
		return Issue{ Severity::Warning, L"Interactive control cannot receive keyboard focus" };
	}
	return std::nullopt;
}


std::optional<Issue> ImageWithoutNameRule::check(const ElementInfo& element) const {
	if (element.controlType == UIA_ImageControlTypeId && isBlank(element.name)) {
		return Issue{ Severity::Warning, L"Image has no accessible name(fine if purely decorative)" };
	}
	return std::nullopt;
}

std::vector<std::unique_ptr<Rule>> defaultRules() {
	std::vector<std::unique_ptr<Rule>> rules;
	rules.push_back(std::make_unique<MissingNameRule>());
	rules.push_back(std::make_unique<NotKeyboardFocusableRule>());
	rules.push_back(std::make_unique<ImageWithoutNameRule>());
	return rules;
}

std::vector<Finding> audit(const std::vector<ElementInfo>& elements,
	const std::vector<std::unique_ptr<Rule>>& rules) {
	std::vector<Finding> findings;
	for (const auto& element : elements) {
		for (const auto& rule : rules) {
			auto ruleResult = rule->check(element);
			if (ruleResult != std::nullopt) {
				findings.push_back(Finding{ rule->id(),ruleResult->severity, ruleResult->message,element });
			}
		}
	}
	return findings;
}
