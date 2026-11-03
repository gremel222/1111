# Первый запуск на Windows — UE 5.8

Эта инструкция для владельца: Windows, Unreal Engine 5.8 и Visual Studio уже установлены.

## 1. Что находится в репозитории

- `CodexOfRules.uproject` — исходный каркас проекта UE 5.8.
- `Source/` — C++ модуль, независимое ядро симуляции и временный UE-адаптер арены.
- `Config/` — минимальные настройки проекта и GameMode по умолчанию.
- `Content/` — пока без игровых `.uasset` и `.umap`.
- `Tests/` — native-тесты и консольная демонстрация ядра без Unreal.

В репозитории нет `Binaries`, `Intermediate`, `Saved`, `.sln` и готовых карт. Это нормально: они создаются локально и не коммитятся.

## 2. Получить ветку

```powershell
git clone https://github.com/gremel222/1111.git
cd 1111
git fetch origin
git switch arena/539eae51-1111
git pull --ff-only
```

Если репозиторий уже открыт, не создавайте второй проект поверх этого. Работайте в этой же папке и ветке.

## 3. Создать файлы Visual Studio

1. В проводнике Windows найдите `CodexOfRules.uproject`.
2. Правый клик → **Generate Visual Studio project files**.
3. Если пункт предлагает выбрать версию Unreal, выберите **5.8**.
4. Дождитесь появления `CodexOfRules.sln`.

Если пункта нет, запустите генерацию из установленной версии UE:

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" `
  -projectfiles -project="$PWD\CodexOfRules.uproject" -game -rocket -progress
```

Путь к UE может отличаться. Важно использовать именно установленную 5.8.

## 4. Собрать редакторную цель

Откройте `CodexOfRules.sln` в Visual Studio:

- Solution Configuration: **Development Editor**.
- Solution Platform: **Win64**.
- Стартовый проект: **CodexOfRules**.
- Build → Build Solution.

Либо из PowerShell:

```powershell
& "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat" `
  CodexOfRulesEditor Win64 Development `
  -project="$PWD\CodexOfRules.uproject" -WaitMutex
```

Ожидаемый результат: сборка завершилась без ошибок. Если UE запросит пересборку модуля при открытии `.uproject`, согласитесь.

## 5. Создать тестовую карту

В репозитории пока нет `.umap`, поэтому первую карту нужно создать в редакторе:

1. Откройте `CodexOfRules.uproject`.
2. File → New Level → **Empty Level**.
3. Сохраните как `Content/Maps/PrototypeArena.umap`.
4. Откройте World Settings и проверьте GameMode Override: `CodexBattleGameMode`.
   Если оставить пусто, должен примениться GameMode из `Config/DefaultEngine.ini`.
5. Нажмите Play.

GameMode создаёт временную арену из стандартных примитивов UE. Это техническая визуализация, не финальный рисованный стиль.

## 6. Управление прототипом

- **P** — запуск/пауза.
- **N** — один такт 250 мс в паузе.
- **R** — перезапуск боя.
- Output Log → фильтр `Codex` — причины выбора правил и события.

Стартовое состояние: 2 героя против 3 противников, две сетки 3×3. Бой начинается в паузе, чтобы можно было посмотреть первый такт.

## 7. Проверить ядро без Unreal

Откройте **x64 Native Tools Command Prompt for VS 2022** или Developer PowerShell в папке репозитория:

```powershell
python tools/build_core.py --compiler cl
python tools/build_core.py --compiler cl --demo
```

Если используется команда `py`:

```powershell
py -3 tools/build_core.py --compiler cl
py -3 tools/build_core.py --compiler cl --demo
```

Тесты компилируют тот же `Battle.cpp`, который использует UE-модуль. Они не требуют Unreal и проверяют порядок тактов, выбор правил, цели, ресурсы, повторение и резервное поведение.

## 8. Что сообщить после первого запуска

Скопируйте:

1. результат Build Solution;
2. первые ошибки Output Log, если они есть;
3. скриншот арены;
4. строку `Outcome` из консольной демонстрации, если запускали её.

Не отправляйте папки `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache` и `.vs`.

## 9. Границы текущего прототипа

Уже есть: фиксированный такт, Snapshot после разрешения текущих действий, приоритетные правила, атака/лечение/защита, cooldown, ресурс, причины отказа, ролевой fallback, детерминированный порядок и native-тесты.

Ещё нет: редактора карточек, графа, сценариев, перемещения, статусов, телеграфов, приказов игрока, сохранений, replay-файлов, финального UI и финального арта.

Сборка UE и редактор должны быть проверены на компьютере владельца. В Arena UE 5.8 не обнаружен; там проверялось только независимое C++ ядро.
