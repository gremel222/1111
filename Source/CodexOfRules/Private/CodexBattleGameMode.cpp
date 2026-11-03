#include "CodexBattleGameMode.h"
#include "CodexBattleArena.h"
#include "Engine/World.h"

ACodexBattleGameMode::ACodexBattleGameMode()
{
    DefaultPawnClass = nullptr;
}

void ACodexBattleGameMode::BeginPlay()
{
    Super::BeginPlay();
    GetWorld()->SpawnActor<ACodexBattleArena>();
}
