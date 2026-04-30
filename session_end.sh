#!/bin/bash
set -e

PROJECT_DIR="/Users/jason/MultiphaseAudio/MultiverseUltimate"
SESSION_LOG="$PROJECT_DIR/SESSION_LOG.md"
AI_STATE="$PROJECT_DIR/AI_STATE.md"
AI_HANDOFF="$PROJECT_DIR/AI_HANDOFF.md"
CLAUDE="$PROJECT_DIR/CLAUDE.md"
AI_RULES="$PROJECT_DIR/AI_RULES.md"

cd "$PROJECT_DIR" || exit 1

# Skip if no session log exists
if [ ! -f "$SESSION_LOG" ]; then
  echo "No session log found, skipping updates."
  exit 0
fi

# Extract status sections from session log
COMPLETED=$(sed -n '/^## Completed/,/^## In Progress/p' "$SESSION_LOG" | sed '$d')
IN_PROGRESS=$(sed -n '/^## In Progress/,/^## Broken/p' "$SESSION_LOG" | sed '$d')
BROKEN=$(sed -n '/^## Broken/,/^## Next Step/p' "$SESSION_LOG" | sed '$d')
NEXT_STEP=$(sed -n '/^## Next Step/,/^---/p' "$SESSION_LOG" | sed '$d' | sed '/^## Next Step/d')

# 1. Update AI_STATE.md (append session status)
{
  echo -e "\n## Session Update $(date +%Y-%m-%d)"
  [ -n "$COMPLETED" ] && echo "$COMPLETED"
  [ -n "$IN_PROGRESS" ] && echo "$IN_PROGRESS"
  [ -n "$BROKEN" ] && echo "$BROKEN"
  [ -n "$NEXT_STEP" ] && echo -e "## Next Step\n$NEXT_STEP"
} >> "$AI_STATE"

# 2. Update AI_HANDOFF.md Next Steps (replace in-place)
if [ -n "$NEXT_STEP" ]; then
  NEXT_LINE=$(grep -n "^## Next Steps$" "$AI_HANDOFF" | cut -d: -f1)
  SEP_LINE=$(grep -n "^---$" "$AI_HANDOFF" | awk -v nl="$NEXT_LINE" '$1 > nl {print $1; exit}')
  if [ -n "$NEXT_LINE" ] && [ -n "$SEP_LINE" ]; then
    head -n $((NEXT_LINE - 1)) "$AI_HANDOFF" > "$AI_HANDOFF.tmp"
    echo -e "## Next Steps\n$NEXT_STEP" >> "$AI_HANDOFF.tmp"
    tail -n +$SEP_LINE "$AI_HANDOFF" >> "$AI_HANDOFF.tmp"
    mv "$AI_HANDOFF.tmp" "$AI_HANDOFF"
  fi
fi

# 3. Update CLAUDE.md Next Steps (replace in-place)
if [ -n "$NEXT_STEP" ]; then
  sed -i '' "/^## Next Steps$/,$ d" "$CLAUDE"
  echo -e "## Next Steps\n$NEXT_STEP" >> "$CLAUDE"
fi

# 4. Clean up session log
rm "$SESSION_LOG"

# 5. GitHub full backup
git add "$AI_STATE" "$AI_HANDOFF" "$CLAUDE" "$AI_RULES" 2>/dev/null
git commit -m "Session $(date +%Y-%m-%d): Update AI context files" || echo "No changes to commit"
git push || echo "Push failed: check remote connection"
