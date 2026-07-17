#include "Interaction/InteractableActor.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

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
	
	OutlineMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("OutlineMeshComponent"));
	OutlineMeshComponent->SetupAttachment(MeshComponent);
	OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMeshComponent->SetGenerateOverlapEvents(false);
	OutlineMeshComponent->SetCanEverAffectNavigation(false);
	OutlineMeshComponent->SetCastShadow(false);
	OutlineMeshComponent->SetRenderCustomDepth(false);
	OutlineMeshComponent->SetHiddenInGame(true);
	OutlineMeshComponent->SetVisibility(false);
	OutlineMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	OutlineMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	OutlineMeshComponent->SetRelativeScale3D(FVector(GeometryOutlineScale));
}

void AInteractableActor::BeginPlay()
{
	Super::BeginPlay();
	
	RefreshGeometryOutlineMesh();
	ApplyHighlightVisibility(false);

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

	const bool bShouldShowHighlight =
		bUseHighlight && bHighlightRequested;

	ApplyHighlightVisibility(bShouldShowHighlight);
}

void AInteractableActor::ApplyHighlightVisibility(bool bVisible)
{
	if (!MeshComponent)
	{
		return;
	}

	const bool bShouldShowHighlight =
		bUseHighlight && bHighlightRequested && bVisible;

	const bool bShouldUsePostProcess =
		bShouldShowHighlight && bUsePostProcessHighlight;

	MeshComponent->SetRenderCustomDepth(bShouldUsePostProcess);

	if (bShouldUsePostProcess)
	{
		MeshComponent->SetCustomDepthStencilValue(FocusedStencilValue);
	}

	if (!OutlineMeshComponent)
	{
		return;
	}

	const bool bShouldShowGeometryOutline =
		bShouldShowHighlight
		&& bUseGeometryOutline
		&& GeometryOutlineMaterial != nullptr;

	if (bShouldShowGeometryOutline)
	{
		RefreshGeometryOutlineMesh();
	}

	OutlineMeshComponent->SetHiddenInGame(!bShouldShowGeometryOutline);
	OutlineMeshComponent->SetVisibility(bShouldShowGeometryOutline);
}

void AInteractableActor::RefreshGeometryOutlineMesh()
{
	if (!OutlineMeshComponent || !MeshComponent)
	{
		return;
	}

	UStaticMesh* SourceMesh = MeshComponent->GetStaticMesh();
	OutlineMeshComponent->SetStaticMesh(SourceMesh);
	OutlineMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	OutlineMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
	OutlineMeshComponent->SetRelativeScale3D(FVector(GeometryOutlineScale));
	OutlineMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OutlineMeshComponent->SetCastShadow(false);
	OutlineMeshComponent->SetRenderCustomDepth(false);

	if (!GeometryOutlineMaterial || !SourceMesh)
	{
		OutlineMeshComponent->SetHiddenInGame(true);
		OutlineMeshComponent->SetVisibility(false);
		return;
	}

	const int32 MaterialSlotsCount = FMath::Max(1, MeshComponent->GetNumMaterials());

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialSlotsCount; ++MaterialIndex)
	{
		OutlineMeshComponent->SetMaterial(MaterialIndex, GeometryOutlineMaterial);
	}
}