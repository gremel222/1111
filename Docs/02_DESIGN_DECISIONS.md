# Design Decisions Log - The Codex of Rules

## Resolution Status Legend
- **CONFIRMED_BY_USER** — Direct user choice/confirmation in chat
- **PROJECT_BASELINE** — Agreed project base, not direct quote
- **PROPOSED** — AI suggestion
- **OPEN** — Unresolved
- **PROTOTYPE_PARAMETER** — Startup numbers requiring tests
- **SUPERSEDED** — Overridden old decision

---

## Confirmed User Decisions

| # | Topic | User Answer | Project Decision | Status |
|---|-------|-------------|-----------------|--------|
| 1 | AI Complexity | D | Deep visual programming with gradual learning | CONFIRMED_BY_USER |
| 2 | Combat Interaction | — | Hybrid: autonomous battle + pause + limited commands | PROJECT_BASELINE |
| 3 | World Structure | B | Large map with regions and linked routes | CONFIRMED_BY_USER |
| 4 | Caravan Role | D | Full logistics system: base, supply, modules, trading | CONFIRMED_BY_USER |
| 5 | Hero Acquisition | D | Mixed: plot, hiring, exploration, rescue | CONFIRMED_BY_USER |
| 6 | Character Progression | C | Classic RPG: levels, characteristics, equipment, talents | CONFIRMED_BY_USER |
| 7 | Idle Progress | B | Only on unlocked routes | CONFIRMED_BY_USER |
| 8 | Art Style | E | Mixed: pixel art world, watercolor portraits | CONFIRMED_BY_USER |
| 9 | Story Importance | C | Full story. | CONFIRMED_BY_USER |
| 10 | Scale | D | Expandable single game | CONFIRMED_BY_USER |

## Project Baseline Decisions

### AI & Execution
- **Formal model**: Finite state machine with states: Idle → Evaluating → Executing → Interrupted → Disabled
- **DecisionPoint**: Atomic operation at decision point, not a multi-tact state
- **Battle Snapshot**: Collected once per simulation tick, AI reads prepared data
- **Snapshot frequency**: Once per tactical tick (configurable, proto: 0.25s)
- **Tick order**: Fixed step, deterministic event order

### AI Memory & Information
- **Three knowledge levels**: Confirmed → Assumed → Stale
- **Information layers**: Knowledge level, freshness, distribution, source, reliability
- **Obsolescence**: Data fades based on type-specific TTL (startup values, not final balance)
- **No telepathy**: Heroes only know what was observed or transmitted

### Combat System
- **Grid**: 3×3 per side, 9 slots total
- **Factions**: Own grid
- **Distance**: Abstract: `1 + depthAttacker + depthTarget + abs(columnAttacker - columnTarget)`
- **No free navigation**: On first stage, not full 3D arena
- **Bosses**: Telegrams for major attacks, check algorithms, not just HP

### AI Execution
- **Rule conflicts**: Priority list, first matching rule wins; if action impossible, skip to next
- **Sequences**: Steps with actuality checks before each step
- **Policies**: ResumeIfValid after temporary control; Abort on death/target lost/order/timout; Restart disabled in MVP
- **Reserve behavior**: Class-based default if no rule matches; does not use rare consumables auto
- **Player Orders**: Limited impulses per battle (start: 3), resource that can be upgraded

### Information & Communication
- **Ignorance levels**: Unknown → Discovered → Recognized → Learned → Scanned
- **Obsolescence**: Data fades; each type has startup TTL (not final balance)
- **Reliability**: Confirmed for ordinary decisions; Assumed for search; Stale allows going to last known location
- **Transfer**: Requires communication channel (distance/visibility or signal action); arrives next tick; can be blocked

### AI Restrictions
- **No infinite loops**: MaxIterations limit
- **No uncontrolled variable creation**
- **No per-rule wait loops**
- **Action unavailable**: Rule skipped, reason recorded, check next rule
- **Equal priorities**: List order is tie-breaker; deterministic sorting, no hidden random tie-breaker
- **Action atomicity**: SELECT → DECLARE → [WINDUP] → COMMIT → RESOLVE / DELIVER → RECOVER
- **Commit**: Cost/item fixed; after COMMIT usually not returned even if projectile misses (exception defined by ability)
- **Mewer action**: May skip WINDUP

### Karavan & Economy
- **Only stabilized routes**: Offline only on stabilized routes
- **Route services**: Separate from main caravan; not second full caravan; not hero collection
- **Offline limit**: Startup limit 8 hours, test value
- **Offline formula**: `NetCycle = Revenue − Wages − Supplies − MaintenanceReserve − Tolls − ExpectedLossReserve`
- **Economic floor**: Emergency reserve, basic resource purchase at guaranteed price, restorative contract, settlement help, plot protection, return path

### Hero Development
- **Separate progression**: Physical RPG progression separate from AI development
- **INT does not automatically increase AI capacity**: Capacity = ClassBase + SpecializationBonus + ArchiveBonus + TalentBonus + GearBonus; global cap 40
- **No permanent death**: Hero becomes "defeated" at 0 HP; after battle gets wounds; no permanent death in standard campaign
- **Reserve heroes**: Limited experience in training and personal tasks; slower than active composition
- **Equipment**: Suggested slots: main weapon, armor, helmet, gloves, shoes, amulet, two rings, relic; no permanent wear for hero gear; wagons and mechanisms wear

### AI Model (Formal)
- **Hero states**: Idle, ExecutingAction, ExecutingSequence, Interrupted, Disabled
- **Evaluating not a state**: Evaluation — atomic procedure at DecisionPoint
- **Sequence steps**: Each has type, inputs, actuality condition, success/failure condition, timeout, interruption policy
- **Policies**: ResumeIfValid after temporary control; Abort on target lost/order/timout/NoProgress; Restart disabled in MVP
- **TRUE/FALSE/UNKNOWN**: UNKNOWN means absence/unreliability; normal condition does not trigger on UNKNOWN; special blocks may check IsUnknown, IsStale, Confidence
- **Unavailable action**: Rule skipped, reason recorded; no implicit wait or infinite repeat; explicit Wait/Retry may appear later but not needed for MVP

### Information System
- **Knowledge levels**: Unknown → Discovered → Recognized → Learned → Scanned
- **Obsolescence**: Data fades; each type has startup TTL (not final balance)
- **Reliability**: Confirmed for ordinary decisions; Assumed for search; Stale allows going to last known location
- **Transfer**: Requires communication channel; arrives next tick; can be blocked; does not disappear after source death

### AI Conflict Resolution
- **Rule conflicts**: Priority list; first matching rule wins; if action impossible, skip to next; one hero cannot execute two incompatible main actions simultaneously
- **Step actuality**: Each sequence step has actuality check; if step becomes impossible, sequence ends
- **Resume/Restart/Abort**: ResumeIfValid after temporary control; Abort on target lost/order/timout/NoProgress; Restart disabled in MVP; max one restart
- **Sequence cycles**: MaxIterations limit; progress predicate; timeout and NO_PROGRESS on no progress after NoProgressTicks; start: 8 ticks (2 seconds), further balanced by step type
- **Action lifecycle**: SELECT → DECLARE → [WINDUP] → COMMIT → RESOLVE / DELIVER → RECOVER; before COMMIT no resource cost; after COMMIT usually not returned even if projectile misses (exception defined by ability); instant action may skip WINDUP

### Karavan & Logistics
- **Module system**: Main wagon, cargo wagons, living quarters, workshop, infirmary, alchemical laboratory, cartography table, defense, rune module
- **Constraints**: Carry capacity, storage volume, number of modules, draft animals, speed, durability, maintenance cost
- **Supply categories**: Rations, water, fodder, repair materials, medicine, fuel/energy, trade/scout cargo
- **Supply consumption**: Calculated per journey segment/game day, not per second; considers crew size, animals, route length, climate, road condition, modules
- **Caravan programming**: Simpler strategic rules; first player programs heroes; later some blocks may manage logistics

### Karavan: Main & Route Services
- **Main caravan**: Physically in one place only
- **Route services**: NPC services on stabilized routes; not second full caravan; does not assign heroes from collection
- **Hero instructor**: Possible expansion, not base v1.0 rule

### Trade, Craft, Economy & Offline
- **Three item categories**: Raw materials, trade items, game items
- **Regional economy**: Production/ surplus per region, deficit/ demand per region
- **Pricing**: Base price × scarcity factor × demand factor × reputation factor; clamped 0.6× to 1.8× base price
- **Demand**: Finite; changes from supplies and events; large batches lower price of following units
- **Crafter**: Alchemy, repair, smithing, rune production, rare recipes needing regional components, blueprints, specialized module
- **Offline progress**: Simplified economic NPC model on stabilized routes; does not simulate full battle, does not defeat bosses, does not open plot blocks; passive XP for reserve heroes if enabled, separate from hero assignment in route services
- **Offline formula**: `NetCycle = Revenue − Wages − Supplies − MaintenanceReserve − Tolls − ExpectedLossReserve`

### Economic Protection
- **Emergency reserve**: Non-sellable reserve for return to nearest safe point + reserve
- **Basic resource purchase**: Settlement accepts basic materials at guaranteed low price
- **Restorative contract**: Minimal rations/repair in exchange for work or materials
- **Settlement help**: Covers only return/restoration with friendly relations and critical deficit
- **Plot protection**: Plot path remains available except explicit plot event
- **No infinite profit**: Demand finite; large batches lower price; route services have costs (time, wages, supplies, maintenance reserve, tolls, expected loss reserve); storehouses limited; old regions don't give infinite profit without expenses

### Defeat & Recovery
- **Full party defeat**: Battle ends; in standard campaign unique heroes do not die permanently
- **Consequences**: Wounds, time loss, possible partial loss of normal cargo/consumables, repair
- **Report**: Key events and defeat reasons; AI decision chronology; list of wounds, consumables, damages and losses; recovery options: infirmary, settlement, repair, safe path; possibility to repeat route after recovery
- **Report contents**: Key events and defeat reasons; AI decision chronology; list of wounds, consumables, damages and losses; available recovery options; safe path availability

### UI & Visual Direction
- **World, caravans, characters in environment, enemies**: Detailed stylized pixel art
- **World map**: Watercolor/painted, with fog, mountains, forests, roads, settlements
- **Hero portraits**: More detailed painted render
- **Ancient mechanisms**: Runes, bronze, stone, light elements, geometric patterns
- **Palette**: Graphite, dark teal, moss green, weathered bronze, warm amber, faded parchment
- **Avoid**: Anime-isekai, photorealistic 3D characters, generic mobile game UI

### Technical Architecture UE5.8
- **Simulation vs Visualization**: UE physics and frame rate do not determine hits, damage, positions in logical grid or battle outcome; battle logic has own state and execution order
- **Layers**: World/Actors (visualization), Combat Simulation (authoritative states, ticks, actions, statuses, events, positions), AI Runtime (Snapshot, condition check, target choice, program execution), Data Layer (Hero/Enemy/Ability/Condition/Action/Status/Item/Region/Route definitions), UI/UMG (display and editing; commands go to system layer), Save/Migration (permanent IDs, versioning)
- **Blueprint & C++**: AI can write Blueprints and C++; accelerates code/structures/functions/log analysis but does not remove integration, verification, profiling and testing; prototype can start in Blueprint with clean simulation contract; critical paths move to C++ after profiling; do not use Actor Tick as universal rule mechanism
- **Battle Snapshot & Performance**: Formed once per fixed tick; contains prepared data, not references to arbitrary Actors that each block queries anew; perception, Line Trace and target search separate subsystem with caching; rules read Snapshot; complex queries use indexes and candidate limitations; worst case — no rule matches; separate performance test needed

### Determinism, Repeats & Saves
- **v1.0 goal**: Repeatability within one simulation version, one build, supported platform; not bit-identical across processors, OS and builds without separate fixed-point arithmetic spec
- **Deterministic order**: Simulation fixed ticks in one authoritative thread; heroes and enemies by permanent ID; events sorted: Tick → PhasePriority → SourceID → TargetID → SequenceNumber; randomness from own simulation RNG, not global visual generator; physics and animations do not affect result; all temporal values stored in whole ticks
- **Rounding rules**: HP and resources — integer game units; damage — rounding down; minimum 1 if ability explicitly sets minimum; healing — rounding down, max HP limit; resource cost — rounding up; percentage comparisons/integer cross-multiplication
- **Battle repeat data**: Seed and simulation version; initial state and participant order; characteristics, equipment, abilities and statuses; program versions and block parameters; player commands/orders and their order; external events and RNG streams
- **Saves & Migrations**: Program schema version separate from user version; player values preserved; type change (e.g., 35% HP to 35 absolute HP) requires manual correction, automatic conversion prohibited; removed IDs not invisible; player sees inactive block and reason

### Content Scale & Roadmap
- **Miloestones**: MVP → First release goal → Long-term goal
- **Why not 100-130 abilities immediately**: Each ability requires testing with resources, targets, statuses, sequences, equipment, bosses; modularity reduces repetitive work but increases combinations; release goal 25-35 basic abilities; long-term 45-60; 100-130 not in solo release obligation
- **Development stages**: 0. Core → 1. Editor v1 → 2. Scenarios & graph → 3. RPG slice → 4. Karavan & region → 5. Economy/idle → 6. Content → 7. Polish
- ** effort estimate**: ~3,300-6,000 hours for full version; ~4.2-7.7 years at 15h/week; ~3.2-5.8 years at 20h/week; ~2.1-3.8 years at 30h/week; estimate, not guarantee; AI accelerates code writing but does not design, integration, testing, content and corrections

### Risk Register
- Detailed risks with levels and controls for editor complexity, AI conflicts, sequence loops, repeat battle differences, too much content for solo, idle printing money, economic softlock, program breaks after patch, telepathy, manual control replaces AI

### Open Questions (not resolved)
- Full catalog of blocks; exact Program Load/Execution Budget; exact TTL of information; exact hero roster/classes; balance calibration; price/route profit calibration; exact regional content; exact art/UI polish; exact retail price and store plan; exact cross-platform determinism; exact co-op and infrastructure setup

### Five Resolved Questions (v1.3)
- **Balance**: Startup formulas and coefficients in §26.1; calibration by battle telemetry; not tested balance
- **UI Editor**: Wireframe 1920×1080, zones and interactions locked; final textures, icons, adaptivity, accessibility and UX tests
- **Monetization**: Premium single-purchase; no pay-to-win and aggressive monetization; exact price, regional prices and store plan
- **Achievements/Collections**: 40 achievements full version, six collection catalogs; full list of achievement names and exact content of each region
- **Server**: Offline-first; online services in next phase; co-op on two — long-term goal; offline-first release; then cloud/services, then potential 2-player co-op

### Full Chat Topics List
- Names and brief concept; references Sword x Staff/AFK Arena/FFXII Gambits; player role; AI depth; editor format; combat interaction; four heroes in battle; roster size; classes and specializations; hero acquisition; deep RPG progression; INT and AI capacity; 3×3 field; auto-battle, pause, speed; player orders; bosses and phases; trial tower and tactical arena; IF/THEN and priorities; logical operators; variables and memory; events and sequences; interruption and lifecycle actions; block compatibility; journal and diagnostics; test arena and repeats; memory and enemy information; statuses/stacks; caravan and modules; supply and repair; trade and regional goods; crafter; idle/offline; softlock protection; map and regions; story and endings; factions and relations; hero personal stories; visual style; concept art; UE5.8 Blueprint/C++ with AI; determinism and saves; content scale; development timelines; price/monetization; server and future multiplayer; other platforms; full block/ability catalog; initial ten questions and letter answers; detailed hybrid editor; visual prompt and concept screenshot; final audit and status report; initial formulas for characteristics, damage, protection, healing, status and experience; AI computational capacity model; caravan and economy formulas; monetization model; achievements and collection; server and future multiplayer direction; other platforms; full block/ability catalog; initial ten questions and letter answers; detailed hybrid editor; visual prompt and concept screenshot; final audit and status report; initial formulas for characteristics, damage, protection, healing, status and experience; AI computational capacity model; caravan and economy formulas; monetization model; achievements and collection; server and future multiplayer direction; other platforms; full block/ability catalog; initial ten questions and letter answers

### Appendix A: Initial Ten User Answers
| # | Topic | Original Answer | Project Decision |
|---|-------|----------------|-----------------|
| 1 | Hero AI complexity | D | Deep visual programming with gradual learning |
| 2 | Combat interaction | — | Requested best recommendation; result: auto-battle, pause, speed, limited tactical orders |
| 3 | World structure | B | Large map with regions and linked routes |
| 4 | Caravan role | D | Mobile base, supply, modules, trading; path restoration |
| 5 | New hero acquisition | D | Plot, hiring, exploration, rescue; no gacha in single concept |
| 6 | Character progression importance | D | Deep RPG progression, but does not replace AI quality |
| 7 | Idle progress | B | Automation only on unlocked/stabilized routes |
| 8 | Atmosphere and visual style | E | Mixed: pixel art world, watercolor portraits |
| 9 | Story importance | C | Story supports path restoration and explains Codex system |
| 10 | Final game | D | Single strategic RPG with programmable AI, caravan and exploration |

### Appendix B: Visual Brief & Concept Prompt
- **Visual style**: Dark atmospheric fantasy; pixel art locations, roads, buildings and characters; watercolor map; expressive hand-painted portraits; warm bronze and amber details; faded parchment palette; no anime, no Minecraft, no photorealistic 3D characters, no generic mobile game UI
- **Concept scene**: Tactical encounter on overgrown ancient stone road; heavily armored Guardian intercepts enemy attacking Mage; Ranger fires from rear; Alchemist prepares flask; Arcanist casts runic spell; distinct monsters, abandoned bridge, dense forest, ruined tower, glowing road mechanism and supply caravan with lanterns and supplies; UI: four hero portraits with health/resources/statuses; readable battle log explaining AI decisions; enemy information panels; compact caravan status; route/minimap; parchment-inspired runic interface; visible rule examples: IF ALLY HEALTH BELOW 35% → INTERCEPT ENEMY; IF ENEMY TARGETS MAGE → PRIORITIZE ENEMY; IF HEALTH BELOW 25% → USE DEFENSIVE ABILITY; wide 16:9 composition like actual PC game screenshot; gameplay environment dominant; UI readable but not overwhelming; player should immediately understand this is fantasy strategy RPG with autonomous heroes and programmable tactical intelligence; Title: THE CODEX OF RULES — СВОД ПРАВИЛ

### Appendix C: Brief Glossary
- **Svods**: Universal ancient language of rules and protocol for managing autonomous objects
- **Archive**: Player-permutable library of open blocks and templates
- **AI program**: Rules, sequences, branching and hero behavior variables
- **Battle Snapshot**: Immutable prepared data snapshot of current tick
- **DecisionPoint**: Atomic procedure for choosing next action
- **UNKNOWN**: Condition cannot be reliably evaluated from available information
- **OrderOverlay**: Temporary player command over ordinary program
- **Commit**: Moment of action cost fixation
- **Resolve**: Moment of result application or delivered effect creation
- **Route service**: NPC service on stabilized route

### Final Fixation V1.3
- Expanded control list: included all chat topics and five additional decisions — balance formulas, exact UI layout, monetization model, achievements/collections and server roadmap; project contradictions checked at document level; startup formulas, layout and online development roadmap set; game balance, UX and online roadmap still need prototype confirmation

---

## Next Session Requirements
Before next session must have:
- [AGENTS.md] and `Docs/00_START_HERE.md` exist in target branch
- Canonical passport present and not abridged
- All links between local Markdown files correct
- Solution statuses distinguish direct confirmation, project baseline and proposals
- `CURRENT_STATUS` does not assert unverified implementation
- `HANDOFF` contains next step
- `README` leads to source of truth
- No secrets/tokens in documents