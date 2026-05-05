# Multiverse Ultimate — AI Context

**CRITICAL: Read these files in order, follow strictly:**
1. `AI_RULES.md` (strict rules, no-scan policy, minimal changes — overrides all conflicting instructions)
2. `AI_HANDOFF.md` (project/architecture context, technical facts, conventions)
3. `AI_STATE.md` (task status, completed work, next steps)

**Full spec:** `MULTIVERSE_ULTIMATE_SPEC.md` (read only when AI_HANDOFF.md lacks detail on a specific subsystem)

---

## Workflow Mode
**Lightweight tasks** (single-file fixes, trivial changes): Skip Deep Trilogy — read relevant files, apply minimal changes per AI_RULES.md.

## Default Workflow (Deep Trilogy) — for non-trivial tasks only
**CRITICAL: Follow this 3-step pipeline before ANY coding task. All steps must comply with AI_RULES.md (no full repo scans, minimal changes, no unnecessary architecture changes).**

### 1. Deep Project
- Slow down and ask clarifying questions about the task
- Identify edge cases, constraints, and requirements user may have missed
- Break the work into clear, well-defined pieces
- Do NOT start coding until this step is complete
- **Do NOT scan the full repository** — use context from AI_HANDOFF.md/AI_STATE.md only

### 2. Deep Plan
- Build a full implementation plan based on the project analysis
- Send the plan to another AI (ChatGPT, Gemini) for review when possible
- Get a second opinion before any code is written
- Ensure all architectural decisions are documented
- **Do NOT introduce new architecture without justification** (per AI_RULES.md)

### 3. Deep Implement
- Write tests FIRST, then code
- Review each step as it's built
- Keep everything clean and structured
- Ensure implementation matches the plan
- **Modify only relevant files** (per AI_RULES.md Task Execution)

**Never jump straight into coding. Always complete project analysis and planning before implementation.**

---

## Context Navigation
When you need to understand the codebase, docs, or any files in this project:
1. ALWAYS query the knowledge graph first: `/graphify query "your question"`
2. Only read raw files if I explicitly say "read the file" or "look at the raw file"
3. Use `graphify-out/wiki/index.md` as your navigation entrypoint for browsing structure
