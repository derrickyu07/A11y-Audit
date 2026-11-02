# a11y-audit

A command line tool that audits the accessibility of running Windows applications. It attaches to a window, walks its element tree through the **UI Automation** API (the same API screen readers like JAWS and NVDA use), and reports controls that a screen reader or keyboard user would have trouble with.

I built it to learn how assistive technology perceives desktop apps, and to get hands on with C++, COM, and the Win32 API.

## Example

```
> a11y-audit "Broken Dialog"
Scanned 14 elements in "Broken Dialog"

ERROR    edit ""                           missing-name   Interactive control has no accessible name
ERROR    button ""                         missing-name   Interactive control has no accessible name
WARNING  image ""                          image-no-name  Image has no accessible name (fine if purely decorative)

2 errors, 1 warning
```

## Usage

```
a11y-audit <part of window title>
a11y-audit --list
```

- The first visible top level window whose title contains the given text is audited. Matching is case sensitive. Quote titles that contain spaces.
- `--list` prints the titles of all visible windows, which helps when a search finds nothing.

### Exit codes

| Code | Meaning |
|---|---|
| `0` | Audit ran, no errors found (warnings allowed) |
| `1` | The tool itself failed: bad arguments, no matching window, or a COM error |
| `2` | Audit ran and found errors |

Separating "the app has problems" from "the tool failed" lets a build script or CI job fail on accessibility errors without hiding tool failures.

## Rules

| Rule | Severity | Flags |
|---|---|---|
| `missing-name` | Error | An interactive control (button, edit, check box, menu item, list item, and so on) whose name is empty or only whitespace. A screen reader can announce its role but not its purpose. |
| `not-focusable` | Warning | An enabled, onscreen interactive control that cannot receive keyboard focus. |
| `image-no-name` | Warning | An image with no name. This is fine for purely decorative images, which the tool cannot distinguish from meaningful ones. |

### Deliberate exceptions

A noisy auditor gets ignored, so some cases are excluded on purpose. Each exception is pinned down by a unit test.

- **Menu items are not checked for focusability.** Classic Win32 menus report their items as not focusable because menus have their own keyboard model (Alt, then arrow keys).
- **Title bar buttons are not checked for focusability.** Minimize, Maximize, and Close are reachable through the system menu (Alt+Space).
- **Disabled and offscreen controls are skipped** by the focus rule, since they are not expected to be reachable.
- **Only interactive controls need names.** Static text, title bars, and layout containers without names are normal and not flagged.

## How it works

```
window title
    │
findWindowByTitle ──► UI Automation tree ──► collectElements ──► std::vector<ElementInfo>
                          (COM)                                        (plain data)
                                                                            │
                                                         rules ──► findings ──► report
```

**Finding the window.** `EnumWindows` visits every top level window, and the first visible one whose title contains the search text is used.

**Walking the tree.** COM is initialized in the multithreaded apartment, as Microsoft recommends for UI Automation clients. The tool walks the **control view** of the tree, which keeps the elements meaningful to a user and skips layout only containers, recursively up to a fixed depth. Apps can change while being read (a menu closing removes its elements), so failed reads are treated as missing data rather than aborting the audit.

**Snapshotting.** Each element is copied into an `ElementInfo` struct: control type, parent's control type, name, and whether it is focusable, enabled, and onscreen. The rules only ever see this struct, never COM objects.

**Checking.** Rules implement a small `Rule` interface with a virtual `check` method that returns an optional issue. `audit` runs every rule against every element and collects the findings.

### Why the snapshot matters

Because rules work on plain data, they can be unit tested by building `ElementInfo` values by hand, with no window, no COM, and no UI Automation involved. The COM code stays thin and is checked against real apps and Inspect.exe instead.

## Testing

`tests/rules_test.cpp` covers each rule and the audit loop with fake elements, including every deliberate exception above. GoogleTest is downloaded automatically on first configure.

```
ctest --test-dir build --output-on-failure
```

For end to end checks, I built **BrokenDialog**, a small MFC dialog with deliberate accessibility mistakes, so the expected results are known in advance:

| Case | Control | What it tests |
|---|---|---|
| A | Edit box with no label | Missing name |
| B | Edit box with its label placed immediately before it | The correct pattern, which should be clean |
| C | Edit box whose label comes after it in tab order | A label that looks correct visually but is not associated with the field |
| D | Button with no caption | Missing name, as with icon only buttons |
| E | Check box without the `WS_TABSTOP` style | Skipped by Tab |
| F | Icon with no text alternative | Image without a name |

Classic Win32 edit boxes have no caption, so screen readers borrow the name from the static text control just before them in the dialog's order. Cases A and C exercise that rule from both sides.

## Limitations

- **Wrong names are not detected.** The tool can tell when a name is missing, not when it is incorrect. A field labeled visually as "Phone" but announced as something else (case C, depending on how its name resolves) needs a human tester.
- **Tab order is not checked.** The focus rule uses UI Automation's `IsKeyboardFocusable`, which describes whether a control *can* take focus, not whether Tab reaches it. A Win32 control without `WS_TABSTOP` can still report as focusable.
- **Decorative and meaningful images look the same,** so `image-no-name` is a warning rather than an error.
- **Fixed depth.** The walk stops at depth 8, which can cut off content in deep trees.
- **Elevated apps cannot be audited** from a non elevated process, a Windows security boundary.
- **Title matching is case sensitive** and picks the first match.

## Building

Requires Windows 10 or 11, CMake 3.20+, and Visual Studio with the Desktop development with C++ workload.

```
cmake -S . -B build
cmake --build build
.\build\a11y-audit.exe --list
```

In Visual Studio, open the folder as a CMake project and use Build All. Inspect.exe from the Windows SDK is useful for checking results by hand.

## Project structure

```
a11y-audit/
├── CMakeLists.txt
├── src/
│   ├── main.cpp          entry point, report, exit codes
│   ├── com.h             COM initialization guard and HRESULT checking
│   ├── uia_tree.h/.cpp   window search, tree walk, element snapshots
│   ├── element_info.h    the plain data snapshot rules operate on
│   └── rules.h/.cpp      Rule interface, rules, and the audit loop
└── tests/
    └── rules_test.cpp
```

## Possible next steps

- Check tab stops directly by reading each Win32 control's window style through its native window handle
- `--depth N` and `--json` options
- Case insensitive title matching
- Contrast checking for text colors
- A watch mode that rescans when focus moves to a new window
