#include "Interaction/InteractableActor.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AInteractableActor::AInteractableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube")
	);

	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComponent->SetRenderCustomDepth(false);
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();

	if (bAlwaysShowHighlight)
	{
		SetFocusedHighlight(true);
	}
}

void AInteractableActor::Interact_Implementation(AActor* InstigatorActor)
{
	UE_LOG(
		LogTemp,
		Display,
		TEXT("InteractableActor interacted: %s"),
		*GetName()
	);
}

bool AInteractableActor::CanInteract_Implementation(AActor* InstigatorActor) const
{
	return bInteractionEnabled;
}

FText AInteractableActor::GetInteractionPrompt_Implementation() const
{
	return InteractionPrompt;
}

void AInteractableActor::OnFocused_Implementation(AActor* InstigatorActor)
{
	SetFocusedHighlight(true);
}

void AInteractableActor::OnUnfocused_Implementation(AActor* InstigatorActor)
{
	SetFocusedHighlight(bAlwaysShowHighlight);
}

void AInteractableActor::SetFocusedHighlight(bool bFocused)
{
	bHighlightRequested = bFocused;

	if (!MeshComponent)
	{
		return;
	}

	if (!bUseHighlight)
	{
		GetWorldTimerManager().ClearTimer(HighlightVisibilityTimerHandle);
		ApplyHighlightVisibility(false);
		return;
	}

	if (!bHighlightRequested)
	{
		GetWorldTimerManager().ClearTimer(HighlightVisibilityTimerHandle);
		ApplyHighlightVisibility(false);
		return;
	}

	if (!bHideHighlightWhenOccluded)
	{
		GetWorldTimerManager().ClearTimer(HighlightVisibilityTimerHandle);
		ApplyHighlightVisibility(true);
		return;
	}

	RefreshHighlightVisibility();

	GetWorldTimerManager().SetTimer(
		HighlightVisibilityTimerHandle,
		this,
		&AInteractableActor::RefreshHighlightVisibility,
		FMath::Max(0.05f, HighlightVisibilityCheckInterval),
		true
	);
}

void AInteractableActor::ApplyHighlightVisibility(bool bVisible)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetRenderCustomDepth(bVisible);

	if (bVisible)
	{
		MeshComponent->SetCustomDepthStencilValue(FocusedStencilValue);
	}
}

void AInteractableActor::RefreshHighlightVisibility()
{
	if (!bUseHighlight || !bHighlightRequested)
	{
		ApplyHighlightVisibility(false);
		return;
	}

	const bool bShouldShowHighlight =
		!bHideHighlightWhenOccluded ||
		IsHighlightVisibleFromLocalPlayerCamera();

	ApplyHighlightVisibility(bShouldShowHighlight);
}

bool AInteractableActor::IsHighlightVisibleFromLocalPlayerCamera() const
{
	const UWorld* World = GetWorld();

	if (!World || !MeshComponent)
	{
		return true;
	}

	const APlayerCameraManager* CameraManager =
		UGameplayStatics::GetPlayerCameraManager(this, 0);

	if (!CameraManager)
	{
		return true;
	}

	const FVector TraceStart = CameraManager->GetCameraLocation();
	const FVector TraceEnd = MeshComponent->Bounds.Origin;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(this);

	if (const APlayerController* PlayerController =
		UGameplayStatics::GetPlayerController(this, 0))
	{
		if (const APawn* PlayerPawn = PlayerController->GetPawn())
		{
			QueryParams.AddIgnoredActor(PlayerPawn);
		}
	}

	FHitResult Hit;

	const bool bBlocked = World->LineTraceSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	return !bBlocked;
}