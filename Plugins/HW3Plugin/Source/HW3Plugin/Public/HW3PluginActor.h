#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HW3PluginActor.generated.h"

class UStaticMeshComponent;
class URotatingMovementComponent;

UCLASS()
class HW3PLUGIN_API AHW3PluginActor : public AActor
{
	GENERATED_BODY()

public:
	AHW3PluginActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Actor")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Actor")
	TObjectPtr<URotatingMovementComponent> RotatingMovementComponent;
};