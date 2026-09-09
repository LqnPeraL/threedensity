// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CombatGameMode.generated.h"

class ACombatSensei;

/**
 *  Simple GameMode for a third person combat game
 */
UCLASS(abstract)
class ACombatGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACombatGameMode();

protected:
	virtual void BeginPlay() override;

	/** Mentor placed near the player start */
	UPROPERTY(EditAnywhere, Category = "Sensei")
	TSubclassOf<ACombatSensei> SenseiClass;

	/** Offset from PlayerStart: forward, right, up (cm) */
	UPROPERTY(EditAnywhere, Category = "Sensei")
	FVector SenseiSpawnOffset = FVector(280.0f, 160.0f, 0.0f);

	UPROPERTY(Transient)
	TObjectPtr<ACombatSensei> SpawnedSensei;

	void SpawnSenseiAtStart();
};
