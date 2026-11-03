# THE CODEX OF RULES — проект

Единая точка входа для разработки игры «Свод Правил».

## Начать работу

1. Прочитайте [`AGENTS.md`](AGENTS.md).
2. Откройте [`Docs/00_START_HERE.md`](Docs/00_START_HERE.md).
3. Основной источник проектных решений: [`Docs/01_GAME_PASSPORT.md`](Docs/01_GAME_PASSPORT.md).
4. Перед задачей проверяйте [`Docs/03_CURRENT_STATUS.md`](Docs/03_CURRENT_STATUS.md), [`Docs/07_HANDOFF.md`](Docs/07_HANDOFF.md), [`Docs/08_PHASE0_AUDIT.md`](Docs/08_PHASE0_AUDIT.md) и соответствующие спецификации.
5. После задачи обновляйте документацию по правилам `AGENTS.md`.

## Проверенное состояние

Аудит 2026-11-03 подтвердил: подключённый репозиторий содержит документацию, но не UE-проект. Игровая сборка и тесты не запускались. Следующий блокер — получить существующий UE5.8-проект или явно согласовать создание нового каркаса. Подробности: [`Docs/08_PHASE0_AUDIT.md`](Docs/08_PHASE0_AUDIT.md).

Воспроизводимая проверка файлов и среды (не игровые тесты):

```bash
python3 tools/audit_repository.py
python3 -m unittest discover -s tests -p 'test_*.py' -v
```

## Важно

- Markdown-паспорт — канонический редактируемый источник в Git, текущая редакция v1.3.2.
- Подтверждённая графика: рисованный контурный мир, предметы и герои вместо пиксель-арта; акварельная карта и рисованные портреты сохранены. Атмосфера различается по регионам. См. [`Docs/09_ART_DIRECTION.md`](Docs/09_ART_DIRECTION.md), U-019/U-020 и ADR-0002.
- Имеется только исторический PDF-снимок v1.3; DOCX отсутствует. Экспорты нужно пересобрать из актуального Markdown и проверить. Не считать старый PDF текущим v1.3.2.
- Не все значения паспорта являются подтверждённым балансом. Статусы решений указаны в `Docs/02_DESIGN_DECISIONS.md`.
- Не считайте реализацию проверенной, пока агент не изучил фактический репозиторий, среду Unreal Engine и результаты тестов.

## Работа через Arena Agent Mode

См. [`Docs/ARENA_SETUP.md`](Docs/ARENA_SETUP.md), [`Docs/ARENA_NEW_SESSION_PROMPT.md`](Docs/ARENA_NEW_SESSION_PROMPT.md) и [`Docs/ARENA_END_OF_TASK_PROMPT.md`](Docs/ARENA_END_OF_TASK_PROMPT.md).
