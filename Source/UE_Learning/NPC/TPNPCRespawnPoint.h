#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TPNPCRespawnPoint.generated.h"

class ATPNPCCharacter;
class USceneComponent;

UCLASS()
class UE_LEARNING_API ATPNPCRespawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ATPNPCRespawnPoint();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Respawn")
	TSubclassOf<ATPNPCCharacter> NPCClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Respawn", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxAliveCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Respawn", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float RespawnDelay = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Respawn", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SpawnRadius = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Respawn")
	bool bSpawnOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC|Respawn|Debug")
	bool bDrawDebugSpawnRadius = false;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<ATPNPCCharacter>> AliveNPCs;

	int32 PendingRespawnCount = 0;

	FTimerHandle RespawnTimerHandle;

	void SpawnInitialNPCs();
	void SpawnNPC();
	void RequestRespawn();
	void CleanupInvalidNPCs();

	UFUNCTION()
	void HandleSpawnedNPCDeath(AActor* DeadActor);
};