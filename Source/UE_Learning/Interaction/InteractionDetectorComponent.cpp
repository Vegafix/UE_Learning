#include "Interaction/InteractionDetectorComponent.h"

#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"

UInteractionDetectorComponent::UInteractionDetectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionDetectorComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	DetectionSphere = NewObject<USphereComponent>(Owner, TEXT("InteractionDetectionSphere"));
	if (!DetectionSphere)
	{
		return;
	}

	DetectionSphere->RegisterComponent();
	DetectionSphere->AttachToComponent(
		Owner->GetRootComponent(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);

	DetectionSphere->SetSphereRadius(DetectionRadius);
	DetectionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionSphere->SetCollisionObjectType(ECC_WorldDynamic);
	DetectionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	DetectionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionSphere->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	DetectionSphere->SetGenerateOverlapEvents(true);

	DetectionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&UInteractionDetectorComponent::HandleBeginOverlap
	);

	DetectionSphere->OnComponentEndOverlap.AddDynamic(
		this,
		&UInteractionDetectorComponent::HandleEndOverlap
	);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshFocusTimerHandle,
			this,
			&UInteractionDetectorComponent::RefreshFocus,
			RefreshInterval,
			true
		);
	}
}

void UInteractionDetectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshFocusTimerHandle);
	}

	SetFocusedActor(nullptr);

	Super::EndPlay(EndPlayReason);
}

AActor* UInteractionDetectorComponent::GetFocusedActor() const
{
	return FocusedActor;
}

void UInteractionDetectorComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (IsValidInteractable(OtherActor))
	{
		CandidateActors.AddUnique(OtherActor);
		RefreshFocus();
	}
}

void UInteractionDetectorComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{
	CandidateActors.Remove(OtherActor);

	if (FocusedActor == OtherActor)
	{
		SetFocusedActor(nullptr);
		RefreshFocus();
	}
}

bool UInteractionDetectorComponent::IsValidInteractable(AActor* Actor) const
{
	if (!IsValid(Actor) || Actor == GetOwner())
	{
		return false;
	}

	if (!Actor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		return false;
	}

	return IInteractable::Execute_CanInteract(Actor, GetOwner());
}

void UInteractionDetectorComponent::RefreshFocus()
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		SetFocusedActor(nullptr);
		return;
	}

	AActor* BestActor = nullptr;
	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (int32 Index = CandidateActors.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = CandidateActors[Index];

		if (!IsValidInteractable(Candidate))
		{
			CandidateActors.RemoveAt(Index);
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(
			Owner->GetActorLocation(),
			Candidate->GetActorLocation()
		);

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestActor = Candidate;
		}
	}

	SetFocusedActor(BestActor);
}

void UInteractionDetectorComponent::SetFocusedActor(AActor* NewFocusedActor)
{
	if (FocusedActor == NewFocusedActor)
	{
		return;
	}

	AActor* Owner = GetOwner();

	if (FocusedActor && FocusedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_OnUnfocused(FocusedActor, Owner);
	}

	FocusedActor = NewFocusedActor;

	if (FocusedActor && FocusedActor->GetClass()->ImplementsInterface(UInteractable::StaticClass()))
	{
		IInteractable::Execute_OnFocused(FocusedActor, Owner);
	}

	OnFocusedInteractableChanged.Broadcast(FocusedActor);
}

void UInteractionDetectorComponent::RefreshFocusNow()
{
	AActor* PreviousFocusedActor = FocusedActor;

	RefreshFocus();

	if (FocusedActor == PreviousFocusedActor)
	{
		OnFocusedInteractableChanged.Broadcast(FocusedActor);
	}
}