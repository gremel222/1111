# Arena Agent Memory Protocol

## Project Context
- **Project**: The Codex of Rules / «Свод Правил»
- **Branch**: arena/fa4a7c9d-1111
- **Current Session**: active
- **Baseline Commit**: 867df27 Initial commit
- **Passport Version**: v1.3

## Memory Structure
The following documents form the project's persistent memory. Each serves a specific purpose:

### 00_START_HERE.md
- Indicates reading order and sources of truth
- Points to canonical documents
- Entry point for new sessions

### 01_GAME_PASSPORT.md
- Full design specification
- Consolidated decisions from all sessions
- Do not modify without user confirmation

### 02_DESIGN_DECISIONS.md
- ADR (Architecture Decision Records) log
- Each decision has: status, date, context, consequence
- Distinguishes: CONFIRMED_BY_USER, PROJECT_BASELINE, PROPOSED, OPEN, PROTOTYPE_PARAMETER, SUPERSEDED

### 03_CURRENT_STATUS.md
- Only factual implementation state verified in repository
- No unverified claims about progress
- Tracks: what's done, what's tested, open problems

### 04_OPEN_QUESTIONS.md
- All unresolved questions from design
- Clearly marked as OPEN or PROTOTYPE_PARAMETER
- Not to be answered without user confirmation

### 05_ROADMAP.md
- Tasks and milestones
- Clear progression from MVP to full release
- Dependencies between systems

### 06_CHANGELOG.md
- Significant changes only
- Version entries with dates
- What changed and why

### 07_HANDOFF.md
- Short transfer to next session
- What was done, what was verified, known issues
- Exact next step prompt

### Docs/ARENA_SETUP.md
- Unreal Engine 5.8 availability check
- Build tools status
- Do not claim editor access if unavailable

### Docs/ARENA_NEW_SESSION_PROMPT.md
- Prompt text for new Arena sessions
- Contains the standard prompt structure

### Docs/ARENA_END_OF_TASK_PROMPT.md
- End-of-task summary prompt
- Repository state, PR URL, test results, open items

### Doc/templates/ADR_TEMPLATE.md
- Architecture Decision Record template
- Fields: ID, Title, Status, Context, Decision, Consequences, Date, Decision Maker

### .github/pull_request_template.md
- PR template for this project
- Includes checklist of memory files
- Ensures all required documents are included

## Classification Rules
- **CONFIRMED_BY_USER**: Direct user choice/confirmation
- **PROJECT_BASELINE**: Agreed project base, not direct quote
- **PROPOSED**: AI suggestion
- **OPEN**: Unresolved
- **PROTOTYPE_PARAMETER**: Startup numbers needing tests
- **SUPERSEDED**: Overridden old decision

## Key Directives
- [AGENTS.md] contains rules, not full passport
- 00_START_HERE indicates reading order and truth sources
- 01_GAME_PASSPORT is the full design
- 02_DESIGN_DECISIONS is ADR log
- 03_CURRENT_STATUS shows only verified implementation
- 04_OPEN_QUESTIONS lists unresolved items
- 05_ROADMAP has tasks and milestones
- 06_CHANGELOG tracks significant changes
- 07_HANDOFF passes to next session

## Handoff Protocol for Next Session
1. Read [AGENTS.md] → 00_START_HERE → 02_DESIGN_DECISIONS → 03_CURRENT_STATUS → 07_HANDOFF → spec sheets → check real code → plan → implement → test → update docs → update HANDOFF → commit/push/PR
2. New session creator: create branch from current HEAD, not main
3. Max one PR per session from this branch
4. PR must include all memory files needed by next session
5. Do not merge PR yourself