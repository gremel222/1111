#include "CodexBattleArena.h"

#include "Camera/CameraActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

ACodexBattleArena::ACodexBattleArena()
{
    PrimaryActorTick.bCanEverTick = true;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    TileMesh = Cube.Object;
    UnitMesh = Cylinder.Object;
    BaseMaterial = Material.Object;
}

void ACodexBattleArena::BeginPlay()
{
    Super::BeginPlay();
    RestartBattle();

    // The prototype map may be empty; provide simple lighting so primitives are readable.
    ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 900.0f), FRotator(-50.0f, -35.0f, 0.0f));
    if (Sun) Sun->GetDirectionalLightComponent()->SetIntensity(3.0f);
    ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>();
    if (Sky) Sky->GetLightComponent()->SetIntensity(1.0f);

    BattleCamera = GetWorld()->SpawnActor<ACameraActor>(FVector(0.0f, -1550.0f, 1450.0f), FRotator(-43.0f, 90.0f, 0.0f));
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        PC->SetViewTarget(BattleCamera);
        PC->bShowMouseCursor = true;
    }
}

void ACodexBattleArena::RestartBattle()
{
    for (auto& Pair : UnitMeshes) if (Pair.Value) Pair.Value->DestroyComponent();
    for (auto& Pair : UnitLabels) if (Pair.Value) Pair.Value->DestroyComponent();
    for (UStaticMeshComponent* Tile : TileComponents) if (Tile) Tile->DestroyComponent();
    UnitMeshes.Empty();
    UnitLabels.Empty();
    TileComponents.Empty();

    Battle = std::make_unique<codex::Battle>(codex::Battle::MakePrototypeScenario());
    Accumulator = 0.0f;
    bPaused = true;
    LastSummary = TEXT("Paused. P = run/pause, N = one tick, R = restart.");
    if (!Battle->GetErrors().empty())
    {
        LastSummary = TEXT("INVALID SCENARIO: ");
        for (const std::string& Error : Battle->GetErrors()) LastSummary += UTF8_TO_TCHAR(Error.c_str());
        UE_LOG(LogTemp, Error, TEXT("%s"), *LastSummary);
    }
    BuildVisuals();
    RefreshVisuals();
}

FVector ACodexBattleArena::UnitLocation(const codex::Unit& Unit) const
{
    const float X = (static_cast<int32>(Unit.position.column) - 1) * 180.0f;
    const float TeamOffset = Unit.team == codex::Team::Heroes ? -280.0f : 280.0f;
    const float DepthOffset = static_cast<int32>(Unit.position.depth) * 150.0f;
    const float Y = TeamOffset + (Unit.team == codex::Team::Heroes ? -DepthOffset : DepthOffset);
    return FVector(X, Y, 70.0f);
}

void ACodexBattleArena::BuildVisuals()
{
    if (!Battle || !TileMesh || !UnitMesh) return;
    for (int32 TeamIndex = 0; TeamIndex < 2; ++TeamIndex)
    {
        for (int32 Depth = 0; Depth < 3; ++Depth)
        {
            for (int32 Column = 0; Column < 3; ++Column)
            {
                UStaticMeshComponent* Tile = NewObject<UStaticMeshComponent>(this);
                Tile->SetupAttachment(SceneRoot);
                Tile->RegisterComponent();
                Tile->SetStaticMesh(TileMesh);
                const float X = (Column - 1) * 180.0f;
                const float Y = (TeamIndex == 0 ? -280.0f - Depth * 150.0f : 280.0f + Depth * 150.0f);
                Tile->SetRelativeLocation(FVector(X, Y, 0.0f));
                Tile->SetRelativeScale3D(FVector(1.55f, 1.25f, 0.08f));
                Tile->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                if (BaseMaterial)
                {
                    UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
                    Mat->SetVectorParameterValue(TEXT("Color"), TeamIndex == 0 ? FLinearColor(0.12f, 0.28f, 0.34f) : FLinearColor(0.34f, 0.16f, 0.12f));
                    Tile->SetMaterial(0, Mat);
                }
                TileComponents.Add(Tile);
            }
        }
    }

    for (const codex::Unit& Unit : Battle->GetSnapshot().units)
    {
        UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(this);
        Mesh->SetupAttachment(SceneRoot);
        Mesh->RegisterComponent();
        Mesh->SetStaticMesh(UnitMesh);
        Mesh->SetRelativeLocation(UnitLocation(Unit));
        Mesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.3f));
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
            const FLinearColor Color = Unit.team == codex::Team::Heroes
                ? (Unit.role == codex::Role::Healer ? FLinearColor(0.2f, 0.75f, 0.45f) : FLinearColor(0.2f, 0.5f, 0.9f))
                : FLinearColor(0.85f, 0.25f, 0.15f);
            Mat->SetVectorParameterValue(TEXT("Color"), Color);
            Mesh->SetMaterial(0, Mat);
        }
        UnitMeshes.Add(Unit.id, Mesh);

        UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);
        Label->SetupAttachment(SceneRoot);
        Label->RegisterComponent();
        Label->SetRelativeLocation(UnitLocation(Unit) + FVector(0.0f, 0.0f, 130.0f));
        Label->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
        Label->SetHorizontalAlignment(EHTA_Center);
        Label->SetWorldSize(22.0f);
        Label->SetTextRenderColor(FColor::White);
        UnitLabels.Add(Unit.id, Label);
    }
}

void ACodexBattleArena::RefreshVisuals()
{
    if (!Battle) return;
    for (const codex::Unit& Unit : Battle->GetSnapshot().units)
    {
        if (TObjectPtr<UStaticMeshComponent>* Mesh = UnitMeshes.Find(Unit.id))
        {
            (*Mesh)->SetVisibility(Unit.hp > 0);
            (*Mesh)->SetRelativeLocation(UnitLocation(Unit));
        }
        if (TObjectPtr<UTextRenderComponent>* Label = UnitLabels.Find(Unit.id))
        {
            const FString Text = FString::Printf(TEXT("%s\nHP %d/%d%s"), UTF8_TO_TCHAR(Unit.name.c_str()), Unit.hp, Unit.maxHp,
                Unit.guardUntilTick >= Battle->GetTick() && Unit.hp > 0 ? TEXT("\nGUARD") : TEXT(""));
            (*Label)->SetText(FText::FromString(Text));
            (*Label)->SetVisibility(Unit.hp > 0);
        }
    }
}

void ACodexBattleArena::StepOnce()
{
    if (!Battle || Battle->GetOutcome() != codex::Outcome::Running) return;
    const codex::TickResult Result = Battle->Step();
    LastSummary = FString::Printf(TEXT("Tick %u | %s | P pause/run, N step, R restart"),
        Result.tick, UTF8_TO_TCHAR(codex::ToString(Result.outcome)));
    UE_LOG(LogTemp, Display, TEXT("[Codex] %s"), *LastSummary);
    for (const codex::DecisionRecord& Decision : Result.decisions)
    {
        if (Decision.selected || Decision.fallback)
        {
            UE_LOG(LogTemp, Display, TEXT("[Codex AI] tick=%u unit=%u rule=%s ability=%s target=%u reason=%s fallback=%d"),
                Decision.tick, Decision.actor, UTF8_TO_TCHAR(Decision.ruleId.c_str()), UTF8_TO_TCHAR(Decision.abilityId.c_str()), Decision.target,
                UTF8_TO_TCHAR(Decision.reason.c_str()), Decision.fallback ? 1 : 0);
        }
    }
    for (const codex::BattleEvent& Event : Result.events)
    {
        if (Event.kind == "damage" || Event.kind == "heal" || Event.kind == "down")
        {
            UE_LOG(LogTemp, Display, TEXT("[Codex Event] tick=%u %s source=%u target=%u amount=%d"),
                Event.tick, UTF8_TO_TCHAR(Event.kind.c_str()), Event.source, Event.target, Event.amount);
        }
    }
    RefreshVisuals();
}

void ACodexBattleArena::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (PC)
    {
        if (PC->WasInputKeyJustPressed(EKeys::P)) bPaused = !bPaused;
        if (PC->WasInputKeyJustPressed(EKeys::R)) RestartBattle();
        if (PC->WasInputKeyJustPressed(EKeys::N)) StepOnce();
    }
    if (!bPaused && Battle && Battle->GetOutcome() == codex::Outcome::Running)
    {
        Accumulator += FMath::Min(DeltaSeconds, 1.0f);
        int32 StepsThisFrame = 0;
        while (Accumulator >= TickSeconds && StepsThisFrame < 8)
        {
            Accumulator -= TickSeconds;
            StepOnce();
            ++StepsThisFrame;
        }
    }
    ShowStatus();
}

void ACodexBattleArena::ShowStatus() const
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(1, 0.0f, FColor::White,
            TEXT("CODEX PROTOTYPE — temporary primitives, not final art"));
        GEngine->AddOnScreenDebugMessage(2, 0.0f, FColor::Cyan, LastSummary);
        GEngine->AddOnScreenDebugMessage(3, 0.0f, FColor::Silver,
            TEXT("P: run/pause | N: one 250 ms tick | R: restart | Output Log: AI reasons"));
    }
}
