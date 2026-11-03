#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimulationCore/Battle.hpp"
#include <memory>
#include "CodexBattleArena.generated.h"

class UStaticMeshComponent;
class UTextRenderComponent;
class UMaterialInterface;
class ACameraActor;

/** Temporary UE presentation adapter. It never decides hits, damage or AI rules. */
UCLASS()
class ACodexBattleArena : public AActor
{
    GENERATED_BODY()

public:
    ACodexBattleArena();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    void BuildVisuals();
    void RefreshVisuals();
    void RestartBattle();
    void StepOnce();
    void ShowStatus() const;
    FVector UnitLocation(const codex::Unit& Unit) const;

    UPROPERTY()
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY()
    TObjectPtr<UStaticMesh> UnitMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> TileMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BaseMaterial;

    UPROPERTY()
    TMap<uint32, TObjectPtr<UStaticMeshComponent>> UnitMeshes;

    UPROPERTY()
    TMap<uint32, TObjectPtr<UTextRenderComponent>> UnitLabels;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> TileComponents;

    UPROPERTY()
    TObjectPtr<ACameraActor> BattleCamera;

    std::unique_ptr<codex::Battle> Battle;
    float Accumulator = 0.0f;
    bool bPaused = true;
    FString LastSummary;
    static constexpr float TickSeconds = 0.25f;
};
