# MultiverseUltimate — AI Usage Guide

## File Reading Order (Automatic on Session Start)
1. `AI_RULES.md` — Strict rules, no-scan policy, minimal changes (overrides all conflicting instructions)
2. `AI_HANDOFF.md` — Project/architecture context, technical facts, conventions
3. `AI_STATE.md` — Task status, completed work, next steps

*Full spec: `MULTIVERSE_ULTIMATE_SPEC.md` (read only when AI_HANDOFF.md lacks subsystem detail)*

---

## Automatic Token Savings
- `CLAUDE.md` reduced to 32 lines (from 119) — saves ~600-900 tokens per session start
- `AI_RULES.md` and `AI_HANDOFF.md` trimmed of redundancies — saves tokens on each read
- No action needed: Savings apply automatically to every session and coding turn

---

## Workflow Modes

### 1. Lightweight Mode (Trivial/Single-File Tasks)
- Auto-triggered for: Single-file fixes, trivial changes
- Behavior: Skips Deep Trilogy, reads only relevant files, applies minimal changes per AI_RULES.md
- Explicit trigger: `simple fix: [task description]`

### 2. Deep Trilogy Mode (Non-Trivial Tasks)
- Auto-triggered for: Multi-file changes, new features, complex fixes
- 3-Step Pipeline (complies with AI_RULES.md):
  1. Deep Project: Clarify requirements, identify edge cases, break into pieces
  2. Deep Plan: Build implementation plan, get second opinion if possible
  3. Deep Implement: Write tests first, then code, modify only relevant files
- Explicit trigger: `run Deep Trilogy for [task description]`

---

## When to Use Full Spec
Trigger only when `AI_HANDOFF.md` lacks detail on a specific subsystem:
`Read MULTIVERSE_ULTIMATE_SPEC.md for [subsystem name]`

---

## Quick Reference
| Action | Trigger Command |
|--------|-----------------|
| Lightweight task | `simple fix: [task]` |
| Deep Trilogy task | `run Deep Trilogy for [task]` |
| Full spec lookup | `Read MULTIVERSE_ULTIMATE_SPEC.md for [subsystem]`