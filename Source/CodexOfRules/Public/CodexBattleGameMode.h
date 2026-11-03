#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CodexBattleGameMode.generated.h"

UCLASS()
class ACodexBattleGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ACodexBattleGameMode();
    virtual void BeginPlay() override;
};
