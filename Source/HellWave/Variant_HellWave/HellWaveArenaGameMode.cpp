// Copyright Epic Games, Inc. All Rights Reserved.

#include "HellWaveArenaGameMode.h"
#include "HellWaveWaveManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"

AHellWaveArenaGameMode::AHellWaveArenaGameMode()
{
}

void AHellWaveArenaGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Create the wave manager
	if (WaveManagerClass)
	{
		WaveManager = NewObject<UHellWaveWaveManager>(this, WaveManagerClass);

		// Gather spawn points
		TArray<AActor*> SpawnPoints = GatherSpawnPoints();
		WaveManager->Initialize(GetWorld(), SpawnPoints);

		// Subscribe to wave state changes
		WaveManager->OnWaveStateChanged.AddDynamic(this, &AHellWaveArenaGameMode::OnWaveStateChanged);

		// Start waves after delay
		GetWorld()->GetTimerManager().SetTimer(StartDelayTimer, this, &AHellWaveArenaGameMode::OnStartDelayComplete, StartDelay, false);
	}
}

void AHellWaveArenaGameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(StartDelayTimer);
}

void AHellWaveArenaGameMode::OnStartDelayComplete()
{
	if (WaveManager)
	{
		WaveManager->StartWaves();
	}
}

TArray<AActor*> AHellWaveArenaGameMode::GatherSpawnPoints() const
{
	TArray<AActor*> SpawnPoints;

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->Tags.Contains(SpawnPointTag))
		{
			SpawnPoints.Add(*It);
		}
	}

	return SpawnPoints;
}

void AHellWaveArenaGameMode::OnWaveStateChanged(EHellWaveState NewState, int32 WaveNumber)
{
	// This is where the GameMode can react to wave changes
	// Blueprint can also bind to the WaveManager's delegate directly
	switch (NewState)
	{
	case EHellWaveState::Victory:
		// Game won — Blueprint handles the UI
		break;

	case EHellWaveState::Defeat:
		// Game lost — Blueprint handles the UI
		break;

	default:
		break;
	}
}

void AHellWaveArenaGameMode::NotifyPlayerDeath()
{
	if (WaveManager)
	{
		WaveManager->OnPlayerDied();
	}
}
