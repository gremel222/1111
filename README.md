# THE CODEX OF RULES — проект

Единая точка входа для разработки игры «Свод Правил».

## Начать работу

1. Прочитайте [`AGENTS.md`](AGENTS.md).
2. Откройте [`Docs/00_START_HERE.md`](Docs/00_START_HERE.md).
3. Основной источник проектных решений: [`Docs/01_GAME_PASSPORT.md`](Docs/01_GAME_PASSPORT.md).
4. Перед задачей проверяйте [`Docs/03_CURRENT_STATUS.md`](Docs/03_CURRENT_STATUS.md), [`Docs/07_HANDOFF.md`](Docs/07_HANDOFF.md), [`Docs/08_PHASE0_AUDIT.md`](Docs/08_PHASE0_AUDIT.md) и соответствующие спецификации.
5. После задачи обновляйте документацию по правилам `AGENTS.md`.

## Проверенное состояние

Создан исходный каркас `CodexOfRules.uproject` для UE 5.8 и первый независимый SimulationCore-прототип: фиксированный такт, Snapshot, приоритетные правила, атака/лечение/защита, причины отказа и ролевой fallback. Native-тесты ядра проходят в Arena. Сборка UE и запуск редактора ещё NOT RUN: их нужно выполнить на компьютере владельца по [`Docs/10_WINDOWS_UE_SETUP.md`](Docs/10_WINDOWS_UE_SETUP.md).

Это первый испытательный срез Phase 1, не готовая игра. Полная спецификация среза: [`Docs/11_SIMULATION_CORE_SPEC.md`](Docs/11_SIMULATION_CORE_SPEC.md). Исторический аудит: [`Docs/08_PHASE0_AUDIT.md`](Docs/08_PHASE0_AUDIT.md).

Воспроизводимые проверки:

```bash
python3 tools/audit_repository.py
python3 -m unittest discover -s tests -p 'test_*.py' -v
python3 tools/build_core.py --compiler g++
python3 tools/build_core.py --compiler g++ --demo
```

На Windows с Visual Studio вместо `g++` используйте `cl` из Developer PowerShell.

## Важно

- Markdown-паспорт — канонический редактируемый источник в Git, текущая редакция v1.3.3.
- Подтверждённая графика: рисованный контурный мир, предметы и герои вместо пиксель-арта; акварельная карта и рисованные портреты сохранены. Атмосфера различается по регионам. См. [`Docs/09_ART_DIRECTION.md`](Docs/09_ART_DIRECTION.md), U-019/U-020 и ADR-0002.
- Имеется только исторический PDF-снимок v1.3; DOCX отсутствует. Экспорты нужно пересобрать из актуального Markdown и проверить. Не считать старый PDF текущим v1.3.3.
- Не все значения паспорта являются подтверждённым балансом. Статусы решений указаны в `Docs/02_DESIGN_DECISIONS.md`.
- Не считайте реализацию проверенной, пока агент не изучил фактический репозиторий, среду Unreal Engine и результаты тестов.

## Работа через Arena Agent Mode

См. [`Docs/ARENA_SETUP.md`](Docs/ARENA_SETUP.md), [`Docs/ARENA_NEW_SESSION_PROMPT.md`](Docs/ARENA_NEW_SESSION_PROMPT.md) и [`Docs/ARENA_END_OF_TASK_PROMPT.md`](Docs/ARENA_END_OF_TASK_PROMPT.md).
