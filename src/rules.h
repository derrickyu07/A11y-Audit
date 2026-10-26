#pragma once

#include "element_info.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

enum class Severity { Error, Warning };

// What a rule reports about one element.
struct Issue {
	Severity severity;
	std::wstring message;
};

// An issue tied to the rule and element that produced it.
struct Finding {
	std::wstring ruleId;
	Severity severity;
	std::wstring message;
	ElementInfo element;
};

class Rule {
public:
	virtual ~Rule() = default;
	virtual std::wstring id() const = 0;
	virtual std::optional<Issue> check(const ElementInfo& element) const = 0;
};

class MissingNameRule : public Rule {
public:
	std::wstring id() const override { return L"missing-name"; }
	std::optional<Issue> check(const ElementInfo& element) const override;
};

class NotKeyboardFocusableRule : public Rule {
public:
	std::wstring id() const override { return L"not-focusable"; }
	std::optional<Issue> check(const ElementInfo& element) const override;
};

class ImageWithoutNameRule : public Rule {
public:
	std::wstring id() const override { return L"image-no-name"; }
	std::optional<Issue> check(const ElementInfo& element) const override;
};

bool isInteractive(CONTROLTYPEID type);
bool isBlank(const std::wstring& text);

std::vector<std::unique_ptr<Rule>> defaultRules();
std::vector<Finding> audit(const std::vector<ElementInfo>& elements,
	const std::vector<std::unique_ptr<Rule>>& rules);