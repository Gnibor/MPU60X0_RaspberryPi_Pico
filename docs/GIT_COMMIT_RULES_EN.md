# GIT_COMMIT_RULES.md

## Purpose

This document defines consistent rules for writing Git commit messages in this project.

The goal is not to make commits sound “nice”, but to make them easy to understand and trace later:

* **what** was changed
* **where** it was changed
* **why** it was changed
* what kind of change it is (feature, fix, refactor, …)

Consistent commit messages help with:

* debugging
* tracking changes
* code reviews
* reverting commits
* generating changelogs
* navigating older project states

---

## Basic Format

Every commit message should follow this structure:

```id="5q4n5f"
<TYPE>: <scope> - <short description>
```

### Meaning

* `<TYPE>` → type of change
* `<scope>` → affected area (file / module / function)
* `<description>` → what was done

---

## Examples

```id="1knt9d"
Fix: input.c - handle CRLF correctly
Add: ringbuffer - implement rb_foreach iterator
Comment: mpu.c - document power management registers
Refactor: tui_layout - simplify area visibility handling
```

---

## Extended Format

If more explanation is needed, add a body below the first line:

```id="a3h8bc"
Fix: mpu_read - correct register offset handling

Fix off-by-one behavior in burst register reads.
This prevented empty or shifted register values.
```

Or:

```id="zz1tf5"
Fix: mpu_read - correct register offset handling

- fix off-by-one error
- align with datasheet
- prevent empty register output
```

**Rule:**
The first line must always be understandable on its own.

---

## Commit Types

### Core Types (standard)

#### Add:

Introduce new functionality

* new functions
* new files
* new modules
* new APIs

```id="x5n6cz"
Add: input.h - add input_event_t definition
```

---

#### Update:

Modify or extend existing behavior

* improve logic
* adjust behavior
* extend functionality

```id="q4qkfy"
Update: input.c - improve escape sequence parsing
```

---

#### Fix:

Bug fix

* incorrect behavior
* crashes
* edge cases
* off-by-one errors

```id="1dyb6p"
Fix: ringbuffer - correct wraparound index calculation
```

---

#### Remove:

Remove code

* deprecated code
* unused functions
* legacy cleanup

```id="8r7hmm"
Remove: ansi.h - drop deprecated helpers
```

---

#### Refactor:

Structural change without changing behavior

* split functions
* simplify logic
* remove duplication

```id="pjdcba"
Refactor: input.c - extract CSI parser into helper
```

---

#### Comment:

Code comments / internal documentation only

```id="7kavkl"
Comment: mpu.c - document power management registers
```

---

## Extended Types (optional)

#### Style:

Formatting only (no logic change)

```id="rcsmzb"
Style: input.c - normalize indentation
```

---

#### Rename:

Renaming

```id="ie5q8t"
Rename: tui_area - rename hidden flag to visible
```

---

#### Move:

Code relocation without logic change

```id="m0g7tw"
Move: input - move keycodes into separate header
```

---

#### Docs:

External documentation

```id="4f8qzy"
Docs: README - add build instructions
```

---

#### Test:

Tests

```id="j7h3c8"
Test: ringbuffer - add wraparound test
```

---

#### Build:

Build system / tooling

```id="rt8dr0"
Build: Makefile - add debug target
```

---

#### Config:

Configuration

```id="sd6twb"
Config: conf.h - add terminal flags
```

---

#### Perf:

Performance improvements

```id="e3jyb0"
Perf: ringbuffer - reduce modulo operations
```

---

#### Revert:

Revert a previous commit

```id="efx9jv"
Revert: tui_layout - revert clipping behavior
```

---

#### WIP:

Work in progress (avoid on main branch)

```id="3yym2r"
WIP: input.c - experiment with ESC timeout
```

---

## Scope Rules

The scope describes **where** the change happened.

### Allowed scopes

* file → `input.c`
* module → `ringbuffer`
* function → `rb_foreach()`
* subsystem → `tui_layout`

### Good examples

```id="v63wz4"
input.c
ringbuffer
mpu_read()
tui_layout
ansi_escape
```

### Bad examples

```id="0tb92q"
stuff
misc
code
```

---

## Description Rules

### Requirements

* short
* technically precise
* no filler words

### Style

Use **imperative mood**:

```id="c3p3kc"
Fix: input.c - handle CRLF correctly
Add: ringbuffer - implement reverse iteration
Remove: tui.c - drop unused redraw path
```

### Avoid

```id="q3k6kz"
Fix: input.c - fixed bug
Update: stuff - improved things
```

---

## General Rules

### 1. One change per commit

Avoid mixing unrelated changes

### 2. First line ≤ ~72 characters

Keeps logs readable

### 3. Be technical, not narrative

No storytelling, just facts

### 4. Consistency over perfection

Consistency matters more than perfect wording

---

## Summary

```id="7xg0dl"
<TYPE>: <scope> - <clear technical change>

Example:
Fix: input - handle CRLF correctly
```

---

## Recommended Setup

Minimal set:

```id="h9ptfl"
Add:
Update:
Fix:
Remove:
Refactor:
Comment:
```

Standard format:

```id="c9i4wq"
<TYPE>: <scope> - <what changed>
```

---

**Goal:**
Commits should be understandable in seconds — even months later.
