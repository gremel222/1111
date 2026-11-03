# How to Read This Project

## Sources of Truth (in order)

1. **01_GAME_PASSPORT.md** - Full game design passport, v1.3. Canonical version. Do not override without user confirmation.

2. **02_DESIGN_DECISIONS.md** - Architecture Decision Records. Log of decisions with status, context, and consequences.

3. **03_CURRENT_STATUS** - Only factual implementation state verified in the repository. No unverified claims.

4. **04_OPEN_QUESTIONS** - Unresolved questions. These must be resolved before proceeding with related work.

5. **05_ROADMAP.md** - Tasks and milestones. Clear progression from MVP to full release.

6. **06_CHANGELOG.md** - Significant changes only. Version entries with dates.

7. **07_HANDOFF.md** - Short transfer to next session. What was done, what was verified, known issues, exact next step.

## Reading Order for New Sessions

```
[AGENTS.md] → 00_START_HERE → 02_DESIGN_DECISIONS → 03_CURRENT_STATUS → 07_HANDOFF → 
profiling sheets → check real code → plan → implement → test → update docs → update HANDOFF → 
commit/push/PR
```

## Key Directives

- **[AGENTS.md]** contains rules, not the full passport.
- **00_START_HERE** indicates reading order and sources of truth.
- **01_GAME_PASSPORT** is the full design.
- **02_DESIGN_DECISIONS** is the ADR log.
- **03_CURRENT_STATUS** shows only verified implementation, not promises.
- **04_OPEN_QUESTIONS** lists unresolved items.
- **05_ROADMAP** has tasks and control points.
- **06_CHANGELOG** tracks significant changes.
- **07_HANDOFF** passes to the next session.

## Unreal Engine Specific
- Verify UE 5.8 availability in the sandbox.
- Visual Studio/Build Tools and Windows toolchain availability must be checked.
- Do not claim Blueprints/UMG are open or compiled if the environment does not allow it.
- Do not create fictional .uasset files.

## GitHub/Arena Specific
- Create working branch `memory/project-context-v1` (or safe unique name if taken).
- Do not commit directly to `main`.
- This Arena session must create no more than one PR.
- Do not merge PR yourself.
- Include all memory files needed by the next session in the PR.

## Test Requirements (must pass before proceeding)

1. `AGENTS.md` and `Docs/00_START_HERE.md` exist in the target branch.
2. Canonical passport is present and not abridged.
3. All links between local Markdown files are correct.
4. Solution statuses distinguish direct confirmation, project baseline, and proposals.
5. `CURRENT_STATUS` does not assert unverified implementation.
6. `HANDOFF` contains the next step.
7. `README` leads to the source of truth.
8. No secrets/tokens in documents.