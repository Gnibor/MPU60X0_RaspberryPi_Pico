# GIT_COMMIT_RULES.md

## Zweck

Diese Datei definiert feste Regeln für Git-Commit-Nachrichten im Projekt.

Das Ziel ist nicht, besonders „schöne“ Commits zu schreiben, sondern schnell und zuverlässig nachvollziehen zu können:

* **was** geändert wurde
* **wo** es geändert wurde
* **warum** es geändert wurde
* welche Art von Änderung es ist (Feature, Fix, Refactor, …)

Saubere und einheitliche Commit-Nachrichten helfen bei:

* Debugging
* Rückverfolgung von Änderungen
* Code-Reviews
* Reverts
* Changelogs
* Orientierung im Projekt

---

## Grundformat

Jeder Commit folgt diesem Schema:

```
<TYPE>: <scope> - <kurze Beschreibung>
```

### Bedeutung

* `<TYPE>` → Art der Änderung
* `<scope>` → betroffener Bereich (Datei / Modul / Funktion)
* `<Beschreibung>` → was konkret gemacht wurde

---

## Beispiele

```
Fix: input.c - handle CRLF correctly
Add: ringbuffer - implement rb_foreach iterator
Comment: mpu.c - document power management registers
Refactor: tui_layout - simplify area visibility handling
```

---

## Erweiterte Form

Wenn nötig, kann ein Commit zusätzlich erklärt werden:

```
Fix: mpu_read - correct register offset handling

Fix off-by-one behavior in burst register reads.
This prevented empty or shifted register values.
```

Oder:

```
Fix: mpu_read - correct register offset handling

- fix off-by-one error
- align with datasheet
- prevent empty register output
```

**Regel:**
Die erste Zeile muss immer allein verständlich sein.

---

## Commit-Typen

### Core-Typen (Standard)

#### Add:

Neue Funktionalität

* neue Funktionen
* neue Dateien
* neue Module
* neue APIs

```
Add: input.h - add input_event_t definition
```

---

#### Update:

Bestehendes Verhalten geändert oder erweitert

* Logik verbessert
* Verhalten angepasst
* Features erweitert

```
Update: input.c - improve escape sequence parsing
```

---

#### Fix:

Fehlerbehebung

* Bugs
* falsches Verhalten
* Abstürze
* Off-by-one Fehler

```
Fix: ringbuffer - correct wraparound index calculation
```

---

#### Remove:

Code entfernen

* veralteter Code
* ungenutzte Funktionen
* Legacy entfernen

```
Remove: ansi.h - drop deprecated helpers
```

---

#### Refactor:

Struktur ändern ohne Verhaltensänderung

* Code aufteilen
* Logik vereinfachen
* Duplikate entfernen

```
Refactor: input.c - extract CSI parser into helper
```

---

#### Comment:

Nur Kommentare / Dokumentation im Code

```
Comment: mpu.c - document power management registers
```

---

## Erweiterte Typen (optional)

#### Style:

Nur Formatierung

```
Style: input.c - normalize indentation
```

---

#### Rename:

Umbenennung

```
Rename: tui_area - rename hidden flag to visible
```

---

#### Move:

Code verschoben

```
Move: input - move keycodes into separate header
```

---

#### Docs:

Externe Dokumentation

```
Docs: README - add build instructions
```

---

#### Test:

Tests

```
Test: ringbuffer - add wraparound test
```

---

#### Build:

Build-System / Tooling

```
Build: Makefile - add debug target
```

---

#### Config:

Konfiguration

```
Config: conf.h - add terminal flags
```

---

#### Perf:

Performance-Verbesserung

```
Perf: ringbuffer - reduce modulo operations
```

---

#### Revert:

Commit rückgängig machen

```
Revert: tui_layout - revert clipping behavior
```

---

#### WIP:

Zwischenstand (nicht für Main-Branch)

```
WIP: input.c - experiment with ESC timeout
```

---

## Scope-Regeln

Der Scope beschreibt **wo** die Änderung passiert.

### Erlaubt

* Datei → `input.c`
* Modul → `ringbuffer`
* Funktion → `rb_foreach()`
* Subsystem → `tui_layout`

### Gute Beispiele

```
input.c
ringbuffer
mpu_read()
tui_layout
ansi_escape
```

### Schlechte Beispiele

```
stuff
misc
code
```

---

## Beschreibung

### Anforderungen

* kurz
* technisch präzise
* keine Füllwörter

### Schreibstil

**Imperativ (Befehlston):**

```
Fix: input.c - handle CRLF correctly
Add: ringbuffer - implement reverse iteration
Remove: tui.c - drop unused redraw path
```

### Vermeiden

```
Fix: input.c - fixed bug
Update: stuff - improved things
```

---

## Allgemeine Regeln

### 1. Eine Änderung pro Commit

Keine Misch-Commits

### 2. Erste Zeile ≤ ~72 Zeichen

Bleibt lesbar in Git-Logs

### 3. Technisch statt erzählerisch

Keine Story, nur Fakten

### 4. Konsistenz ist wichtiger als Perfektion

Lieber immer gleich als selten perfekt

---

## Kurzfassung

```
<TYPE>: <scope> - <klare technische Änderung>

Beispiel:
Fix: input - handle CRLF correctly
```

---

## Empfehlung (für dieses Projekt)

Minimaler Satz:

```
Add:
Update:
Fix:
Remove:
Refactor:
Comment:
```

Standardformat immer:

```
<TYPE>: <scope> - <what changed>
```

---

**Ziel:**
Commits sollen in Sekunden verständlich sein – auch nach Monaten.
