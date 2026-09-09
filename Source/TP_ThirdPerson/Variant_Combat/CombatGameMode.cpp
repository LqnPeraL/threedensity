// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Combat/CombatGameMode.h"
#include "CombatSensei.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

ACombatGameMode::ACombatGameMode()
{
	SenseiClass = ACombatSensei::StaticClass();
}

void ACombatGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnSenseiAtStart();
}

void ACombatGameMode::SpawnSenseiAtStart()
{
	UWorld* World = GetWorld();
	if (!World || !SenseiClass)
	{
		return;
	}

	// Avoid duplicates if a sensei was already placed in the level
	TArray<AActor*> Existing;
	UGameplayStatics::GetAllActorsOfClass(World, ACombatSensei::StaticClass(), Existing);
	if (Existing.Num() > 0)
	{
		SpawnedSensei = Cast<ACombatSensei>(Existing[0]);
		return;
	}

	AActor* Start = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;

	if (Start)
	{
		const FVector Forward = Start->GetActorForwardVector();
		const FVector Right = Start->GetActorRightVector();
		Location = Start->GetActorLocation()
			+ Forward * SenseiSpawnOffset.X
			+ Right * SenseiSpawnOffset.Y
			+ FVector(0.0f, 0.0f, SenseiSpawnOffset.Z);

		// Face toward the player start
		FVector ToStart = Start->GetActorLocation() - Location;
		ToStart.Z = 0.0f;
		if (!ToStart.IsNearlyZero())
		{
			Rotation = ToStart.Rotation();
		}
		else
		{
			Rotation = Start->GetActorRotation() + FRotator(0.0f, 180.0f, 0.0f);
		}
	}
	else if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(World, 0))
	{
		Location = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 280.0f + Pawn->GetActorRightVector() * 160.0f;
		Rotation = (Pawn->GetActorLocation() - Location).Rotation();
		Rotation.Pitch = 0.0f;
		Rotation.Roll = 0.0f;
	}
	else
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnedSensei = World->SpawnActor<ACombatSensei>(SenseiClass, Location, Rotation, Params);
}
