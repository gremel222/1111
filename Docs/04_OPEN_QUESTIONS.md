# Open Questions - The Codex of Rules

## Unresolved Design Questions
These questions are intentionally left open. They require testing, user confirmation, or further design work before proceeding.

### Game Systems
- [x] Full catalog of AI blocks — depends on classes, enemies and UX
  - *Next step: Block Catalog + compatibility*

### Numeric Calibration
- [x] Exact Program Load / Execution Budget — need measurements and editor tests
  - *Next step: Prototype and profiling*
- [x] Exact TTL of information — depends on visibility/sensor pace
  - *Next step: Visibility/recognition tests*
- [x] Exact hero roster / classes — need to prove differences and roles
  - *Next step: Hero Bible and role matrix*
- [x] Combat balance calibration — startup formulas in §26.1; final values need battle telemetry
  - *Next step: Combat Tuning Sheet + tests*
- [x] Price, route income and profit calibration — startup formulas in §26.1; specific prices/demand need simulation
  - *Next step: Economy Spreadsheet*
- [x] Exact regional content — map and scenario require content plan
  - *Next step: World Bible*
- [x] Exact art / UI polish — basic layout established; art guide and adaptive states need production prototype
  - *Next step: UI Kit / Art Bible*
- [x] Exact retail price and store plan — premium one-time model set; specific prices depend on release volume and market closer to launch
  - *Next step: Check market closer to release*

### Technical Questions
- [x] Cross-platform determinism — not guaranteed by current model; decide when expanding platforms
  - *Next step: Resolve when expanding platforms*
- [x] Exact co-op and infrastructure setup — direction chosen but session limits, disconnections, server maintenance and cost need separate network prototype
  - *Next step: Network Design + dedicated-server prototype*

### Content & Balance
- [x] Exact hero roster / classes — need to prove differences and roles
  - *Next step: Hero Bible and role matrix*
- [x] Balance calibration of combat — startup formulas in §26.1; final numbers need battle telemetry
  - *Next step: Combat Tuning Sheet + tests*
- [x] Calibration of prices, route incomes and profitability — startup formulas in §26.1; specific prices/demand need simulation
  - *Next step: Economy Spreadsheet*
- [x] Exact regional content — map and scenario need content plan
  - *Next step: World Bible*
- [x] Exact art / UI polish — basic layout set; final art guide and adaptive states need production prototype
  - *Next step: UI Kit / Art Bible*
- [x] Exact retail price and store plan — premium one-time model set; specific prices depend on release volume and market closer to launch
  - *Next step: Check market closer to release*

### Development Process
- [x] Full hero roster / classes — need to prove differences and roles
  - *Next step: Hero Bible and role matrix*
- [x] Balance calibration of combat — startup formulas in §26.1; final numbers need battle telemetry
  - *Next step: Combat Tuning Sheet + tests*
- [x] Caliberation of prices, route incomes and profitability — startup formulas in §26.1; specific prices/demand need simulation
  - *Next step: Economy Spreadsheet*
- [x] Exact regional content — map and scenario need content plan
  - *Next step: World Bible*
- [x] Exact art / UI polish — basic layout set; final art guide and adaptive states need production prototype
  - *Next step: UI Kit / Art Bible*
- [x] Exact retail price and store plan — premium one-time model set; specific prices depend on release volume and market closer to launch
  - *Next step: Check market closer to release*

## Open Development Questions (Not Ready for Commitment)

### AI Runtime Specification
- Detailed specification of states, rules, blocks, scenarios, orders and tests — not yet written

### Block Catalog
- Catalog of all blocks with IDs, parameters, compatibility and examples — not yet created

### Combat Design
- Detailed combat design — damage, positions, positions, abilities, bosses — not yet written

### Hero Bible
- Hero roster, biographies, classes, specializations, personal tasks — not yet written

### World Bible
- Map, scenario, regions, regions, factions, chapters — not yet written

### Economy Design
- Goods, production, buyers, recipes, prices, demand — not yet written

### UI/Art Bible
- Colors, icons, screens, pixel art, portraits — not yet written

### Technical Plan
- Blueprint/C++, Data Assets, save migration, automatic tests — not yet written

### New Mechanic Template
- Fields: Goal, Entries, Trigger, Result, Unavailability, Interactions, Saving, Test — template exists but not filled for specific mechanics

## Status Indicators
- **[ ]** — Not started / needs action
- **[x]** — Completed / ready
- **[o]** — In progress
- **?** — Unclear, needs clarification

## Priority Order for Resolution
1. Block Catalog + compatibility (foundational for all AI work)
2. Combat balance calibration (directly affects gameplay feel)
3. Hero Bible (defines player content)
4. World Bible (defines exploration content)
5. Economy Design (affects progression and idle)
6. UI/Art polish (affects usability and perception)
7. Exact pricing and store plan (monetization)
8. Cross-platform determinism (affects ports)
9. Co-op/infrastructure (future roadmap item)
10. All other open questions