#include "rules.h"
#include <gtest/gtest.h>

static ElementInfo makeElement(CONTROLTYPEID type, const std::wstring& name) {
	ElementInfo e;
	e.controlType = type;
	e.name = name;
	e.keyboardFocusable = true;
	return e;
}

// MissingNameRule

TEST(MissingNameRule, FlagsUnnamedButton) {
	auto issue = MissingNameRule{}.check(makeElement(UIA_ButtonControlTypeId, L""));
	ASSERT_TRUE(issue.has_value());
	EXPECT_EQ(issue->severity, Severity::Error);
}

TEST(MissingNameRule, FlagsWhitespaceOnlyName) {
	auto issue = MissingNameRule{}.check(makeElement(UIA_ButtonControlTypeId, L"   "));
	EXPECT_TRUE(issue.has_value());
}

TEST(MissingNameRule, IgnoresNamedButton) {
	auto issue = MissingNameRule{}.check(makeElement(UIA_ButtonControlTypeId, L"Save"));
	EXPECT_FALSE(issue.has_value());
}

TEST(MissingNameRule, IgnoresNonInteractiveElements) {
	auto issue = MissingNameRule{}.check(makeElement(UIA_TextControlTypeId, L""));
	EXPECT_FALSE(issue.has_value());
}

// NotKeyboardFocusableRule

TEST(NotKeyboardFocusableRule, FlagsUnfocusableButton) {
	auto e = makeElement(UIA_ButtonControlTypeId, L"Save");
	e.keyboardFocusable = false;
	auto issue = NotKeyboardFocusableRule{}.check(e);
	ASSERT_TRUE(issue.has_value());
	EXPECT_EQ(issue->severity, Severity::Warning);
}

TEST(NotKeyboardFocusableRule, IgnoresDisabledControls) {
	auto e = makeElement(UIA_ButtonControlTypeId, L"Save");
	e.keyboardFocusable = false;
	e.enabled = false;
	EXPECT_FALSE(NotKeyboardFocusableRule{}.check(e).has_value());
}

TEST(NotKeyboardFocusableRule, IgnoresOffscreenControls) {
	auto e = makeElement(UIA_ButtonControlTypeId, L"Save");
	e.keyboardFocusable = false;
	e.offscreen = true;
	EXPECT_FALSE(NotKeyboardFocusableRule{}.check(e).has_value());
}

TEST(NotKeyboardFocusableRule, IgnoresMenuItems) {
	auto e = makeElement(UIA_MenuItemControlTypeId, L"File");
	e.keyboardFocusable = false;
	EXPECT_FALSE(NotKeyboardFocusableRule{}.check(e).has_value());
}

TEST(NotKeyboardFocusableRule, IgnoresTitleBarButtons) {
	auto e = makeElement(UIA_ButtonControlTypeId, L"Close");
	e.keyboardFocusable = false;
	e.parentControlType = UIA_TitleBarControlTypeId;
	EXPECT_FALSE(NotKeyboardFocusableRule{}.check(e).has_value());
}

// ImageWithoutNameRule

TEST(ImageWithoutNameRule, FlagsUnnamedImage) {
	auto issue = ImageWithoutNameRule{}.check(makeElement(UIA_ImageControlTypeId, L""));
	ASSERT_TRUE(issue.has_value());
	EXPECT_EQ(issue->severity, Severity::Warning);
}

TEST(ImageWithoutNameRule, IgnoresNamedImage) {
	auto issue = ImageWithoutNameRule{}.check(makeElement(UIA_ImageControlTypeId, L"Logo"));
	EXPECT_FALSE(issue.has_value());
}

// audit

TEST(Audit, CollectsFindingsFromEveryRule) {
	auto e = makeElement(UIA_ButtonControlTypeId, L"");
	e.keyboardFocusable = false;   // unnamed AND unfocusable

	auto findings = audit({ e }, defaultRules());
	ASSERT_EQ(findings.size(), 2u);
	EXPECT_EQ(findings[0].ruleId, L"missing-name");
	EXPECT_EQ(findings[1].ruleId, L"not-focusable");
}

TEST(Audit, CleanTreeHasNoFindings) {
	std::vector<ElementInfo> elements = {
		makeElement(UIA_ButtonControlTypeId, L"Save"),
		makeElement(UIA_EditControlTypeId, L"File name"),
		makeElement(UIA_TextControlTypeId, L""),
	};
	EXPECT_TRUE(audit(elements, defaultRules()).empty());
}