# Roadmap - The Codex of Rules

## Development Phases

### Phase 0: Core Prototype (Months 1-3)
- **Goal**: Prove core AI mechanics work
- **Deliverables**:
  - 2 heroes, 3 enemy types, 3×3 arena
  - Working AI with rule cards
  - Snapshot system and deterministic tick
  - Battle log/journal
  - Save/load with program versions
- **Exit Criteria**: Repeatable battle; AI decisions explainable; no crashes

### Phase 1: Editor v1 (Months 4-6)
- **Goal**: Enable designer-friendly AI creation
- **Deliverables**:
  - Card-based rule editor
  - Library of conditions/actions
  - Program save/load with compatibility checks
  - Priority ordering
- **Exit Criteria**: Designer can create/debug rules without coding; no conflicts or hangs

### Phase 2: Scenarios & Graph (Months 7-9)
- **Goal**: Complex behavioral authoring
- **Deliverables**:
  - Multi-step scenarios with branching
  - Graph mode for expert users
  - Variables and memory
  - Interruption policies (Resume/Restart/Abort)
- **Exit Criteria**: Scenarios work with limits; can create branching logic; basic variables work

### Phase 3: RPG Slice (Months 10-12)
- **Goal**: Make combat matter for progression
- **Deliverables**:
  - 4 heroes with class abilities
  - Equipment, bosses, progression
  - Several tactical decisions per fight
- **Exit Criteria**: Several viable tactical choices per fight; not just stat comparison

### Phase 4: Karavan & Region (Months 10-14)
- **Goal**: Full expedition loop
- **Deliverables**:
  - Settlement, routes, supplies, repair, events, quests
  - Full expedition from preparation to return
- **Exit Criteria**: Complete expedition from preparation to return; no softlocks

### Phase 5: Economy & Idle (Months 12-15)
- **Goal**: Sustainable economy
- **Deliverables**:
  - Trade, NPC routes, expenses, saturation, limits
  - No free money, softlock protection
- **Exit Criteria**: No free money, softlock protection; economy functions

### Phase 5: Content (Months 15-20)
- **Goal**: Full game content
- **Deliverables**:
  - Regions, plot, heroes, enemies, dungeons
  - Tools and pipeline stable
- **Exit Criteria**: Tools and pipeline stable; all systems integrated

### Phase 7: Polish (Months 20-24)
- **Goal**: Release-ready
- **Deliverables**:
  - Optimization, UX, balance, localization, saves
- **Exit Criteria**: Acceptance tests passed; ready for release

### Phase 8: Online Roadmap (Post-release, Months 24+)
- **Stage A**: Basic release — offline first; no mandatory server or co-op
- **Stage B**: Online services after release — optional auth/cloud saves with explicit user consent; template AI exchange: load/publish programs with version, required blocks list and compatibility declaration; async trials and result tables on fixed scenarios/seed; ordinary single-player works without these services
- **Stage C**: Possible two-player cooperation — preferred format: joint PvE expedition, one shared caravan and four heroes; each player controls programs for two assigned heroes; second sees opponent programs and can suggest fixes but confirmation stays with hero owner or jointly nominated leader; shared resources, map and expedition results; no PvP; listen-server for tests; dedicated authoritative server for permanent progressing sessions/leaderboards, but separate budget for hosting, monitoring, updates, protection and support

## Milestone Table

| **Milestone** | **Duration** | **Deliverable** | **Exit Criteria** |
|--------------|-------------|-----------------|-------------------|
| Phase 0: Core | Months 1-3 | Prototype with 2 heroes, 3 enemies, arena, working AI, editor, journal, saves | Repeatable battle; AI explainable; no crashes |
| Phase 1: Editor v1 | Months 4-6 | Card-based editor, library, save/load, priorities | Designer can create/debug without code; no conflicts |
| Phase 2: Scenarios & Graph | Months 7-9 | Multi-step scenarios, graph mode, variables, interruption policies | Scenarios work with limits; basic variables |
| Phase 3: RPG Slice | Months 10-9 | 4 heroes, class abilities, equipment, boss, progression | Several viable tactical choices per fight |
| Phase 4: Karavan & Region | Months 10-14 | Settlement, routes, supplies, events, quests | Full expedition prep to return; no softlocks |
| Phase 5: Economy & Idle | Months 12-15 | Trade, NPC routes, expenses, saturation, limits | No free money, softlock protection |
| Phase 5: Content | Months 15-20 | Regions, plot, heroes, enemies, dungeons | Tools and pipeline stable |
| Phase 7: Polish | Months 20-24 | Optimization, UX, balance, localization, saves | Acceptance tests passed |
| Stage A (Post-release) | Post-release | Offline-first; no mandatory server/co-op | Campaign fully offline; local saves don't need account |
| Stage B (Post-release) | Post-release + | Online services; template AI exchange; async trials; ordinary SP works without | Auth + cloud saves with explicit consent; template AI exchange; async trials; SP works without |
| Stage C (Post-release) | Post-release + | Two-player cooperation — preferred format | Common caravan, 4 heroes (2 per player); see opponent programs, suggest fixes but confirmation with owner/leader; shared resources/map/results; no PvP; listen-server for tests; dedicated server preferred but separate budget |

## Phase Gate Checkpoints
Each phase must pass its exit criteria before the next begins. Gates are enforced by the HANDOFF document and verified by the test requirements in AGENTS.md.

## Resource Allocation
- **Estimated total**: ~3,300-6,000 hours for full version
- **By weekly hours**:
  - 15 h/week: ~4.2-7.7 years
  - 20 h/week: ~3.2-5.8 years
  - 30 h/week: ~2.1-3.8 years
- These are estimates, not guarantees; AI accelerates code writing but does not eliminate design, integration, testing, content and corrections

## Dependencies Between Systems
- Editor → RPG slice requires minimum editor functionality
- RPG slice requires karavan & region minimum
- Karavan & region requires economy & idle minimum
- Economy & idle requires content minimum
- Content requires polish minimum

## Risk Management
Major risks tracked in the Risk Register (see 02_DESIGN_DECISIONS.md):
- Editor too complex → Gradual learning, templates, simple mode, diagnostics
- AI conflicts/hangs → Single main action, order rules, timeouts, conflict tests
- Sequences loop → Loop limit, progress predicate, timeouts and NO_PROGRESS
- Battle repeats differ → Fixed tick, RNG, event order, isolation from physics
- Too much content for solo → MVP/release/long-term goals; reduce content, not quality
- Idle prints money → Supply, wages, repair, limits, market saturation
- Economic softlock → Emergency reserve, restorative contract, plot protection, return path
- Program breaks after patch → Permanent IDs, schema versions, migrations
- Telepathy → Separate knowledge, freshness, source, transfer
- Manual control replaces AI → Limited impulses and temporary overlays

## Success Metrics
- Player can create program that cannot be done with simple button commands
- Player understands why hero acted as he did
- Player can find error, change program and get different result
- Several viable ways to pass same battle exist
- Program changes depending on enemy, region and team composition
- Player feels his knowledge made the caravan stronger

## Phase Transition Protocol
Before advancing to next phase:
1. Exit criteria of current phase met
2. HANDOFF updated with current state and next step
3. All test requirements in AGENTS.md passed
4. Key team consensus on moving forward
5. No critical unresolved risks remaining for next phase