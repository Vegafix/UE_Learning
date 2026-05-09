#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HW3ModuleActor.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class HW3MODULE_API AHW3ModuleActor : public AActor
{
	GENERATED_BODY()

public:
	AHW3ModuleActor();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Actor")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Test Actor")
	TObjectPtr<UPointLightComponent> PointLightComponent;
};