# Claude Code Prompt Pack (Token-Optimised Dev Workflow)

These prompts are designed to:

- 🔻 Minimise token usage
- 🎯 Keep Claude focused
- 🧠 Prevent runaway behaviour
- ⚡ Speed up development loops
- 🚀 Maintain strict scope control

---

## 🚀 1. Start a new session (ALWAYS use this)

```
Read AI_HANDOFF.md and AI_STATE.md.
Follow AI_RULES.md strictly.

Do not scan the full repository.

Summarise current task in 2 sentences, then wait.
```   

👉 This prevents:
- runaway scanning
- huge context builds
- random assumptions

End session with

Update AI_STATE.md.

Include:
- Completed
- In Progress
- Broken
- Next Step

Keep it concise. Do not scan the full repo.

- Full Backup to github



---

## 🔧 2. Continue working (main loop)

```
Read AI_HANDOFF.md and AI_STATE.md.

We are in Integration Phase.

Only inspect files directly related to audio routing.

Current task:
Wire SamplerEngine and Effects into processBlock.

Do not modify unrelated systems.
```

---

## 🎯 3. Targeted file work (VERY important)

Instead of:
> “fix my plugin”

You do:

```
Read AI_HANDOFF.md and AI_STATE.md.

Only inspect:
- PluginProcessor.cpp
- SamplerEngine.cpp
- Delay.cpp
- Reverb.cpp

Task:
Ensure all engines are mixed and routed correctly.

Do not scan other files.
```

👉 This is where you save the most tokens.

---

## 🧪 4. Debug mode (when something is broken)

```
Read AI_HANDOFF.md and AI_STATE.md.

Do not scan the full project.

Focus only on audio output path.

Trace signal flow step-by-step from MIDI input to final output.

Identify exactly where audio becomes zero.
```

---

## 🧠 5. Prevent over-engineering

If Claude starts doing too much, drop this:

```
Follow AI_RULES.md strictly.

Do not refactor.
Do not redesign.

Only fix the minimal code required to complete the task.
```

---

## 🔄 6. Resume after a break (MAGIC PROMPT)

```
Read AI_HANDOFF.md and AI_STATE.md.

Summarise:
1. What the project is
2. What is currently broken
3. What we are doing next

Then continue the task.
```

👉 This replaces re-explaining everything.

---

## ⚡ 7. When you want to move fast

```
Read AI_HANDOFF.md and AI_STATE.md.

Act as a senior JUCE developer.

Execute the current task with minimal explanation.
Focus on correct, production-ready code.
```

---

## 🛑 8. If it starts scanning everything again

```
Stop.

Do not scan the repository.

Only use the handoff files and explicitly requested files.
```

---

## 🧩 Real example (your current situation)

```
Read AI_HANDOFF.md and AI_STATE.md.
Follow AI_RULES.md strictly.

Only inspect PluginProcessor.cpp.

Task:
Integrate SynthEngine, DrumSequencer, SamplerEngine, Delay, and Reverb into a single audio pipeline inside processBlock.

Do not modify other files.
Do not refactor architecture.
```

---

## 🧠 Why this works

You’ve now created:

- AI_HANDOFF.md → memory
- AI_STATE.md → direction
- AI_RULES.md → discipline

These prompts:
👉 tell Claude how to think, not just what to do

---

## 🎯 Final tip

Your power now comes from:

**controlling scope**

Not:

asking better questions

