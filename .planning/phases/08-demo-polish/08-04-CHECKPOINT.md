# Phase 08-04 Checkpoint: Editor Blueprint Creation

## UMG Widget Blueprints

### 1. WBP_DocumentReader
**Location:** Content/Blueprints/UI/
**Parent:** Reparent to `DocumentReaderWidget` (C++ class)
**Focusable:** Set `bIsFocusable = true` in Details panel

**Required BindWidget children** (names must match exactly):

| Name | Type | Notes |
|------|------|-------|
| `TitleText` | Text Block | Centered top, ~24pt font |
| `ContentText` | Text Block | Centered middle, ~16pt, auto-wrap ON |
| `InspectionImage` | Image | Centered, large, **Visibility: Collapsed** by default |

**Styling:**
- Dark semi-transparent background (80% black overlay)
- Paper/parchment feel for text area
- Add non-BindWidget hint text at bottom: "Press E or Esc to close"

---

### 2. WBP_TitleCard
**Location:** Content/Blueprints/UI/
**Parent:** Plain UUserWidget (no reparent needed)

- Single TextBlock centered on screen: "The Juniper Tree"
- Font: ~48pt, white
- Background: fully black (Canvas Panel with black brush or Border)
- No interaction needed — static display only

---

## Actor Blueprints

### 3. BP_ReadableActor
**Location:** Content/Blueprints/Actors/
**Parent:** `ReadableActor`

**Properties to set:**
- `DocumentReaderWidgetClass` = `WBP_DocumentReader`

---

### 4. BP_NarrativeTrigger
**Location:** Content/Blueprints/Actors/
**Parent:** `NarrativeTriggerActor`

No additional properties needed — configure per-instance in level.

---

### 5. BP_InspectableActor
**Location:** Content/Blueprints/Actors/
**Parent:** `InspectableActor`

**Properties to set:**
- `DocumentReaderWidgetClass` = `WBP_DocumentReader`

---

## Verification

After creating all 5 assets:
- [ ] WBP_DocumentReader compiles without "missing BindWidget" errors
- [ ] WBP_TitleCard has centered title text on black background
- [ ] BP_ReadableActor has `DocumentReaderWidgetClass` = WBP_DocumentReader
- [ ] BP_InspectableActor has `DocumentReaderWidgetClass` = WBP_DocumentReader
- [ ] BP_NarrativeTrigger exists and is placeable
