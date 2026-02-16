// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameMode.h"
#include "HellWaveWaveManager.h"
#include "HellWaveArenaGameMode.generated.h"

class UHellWaveWaveManager;
class UHellWaveHUD;

/**
 *  Game Mode for the HellWave arena wave shooter
 *  Owns the wave manager, handles win/lose conditions,
 *  and coordinates spawn points in the level
 */
UCLASS(abstract)
class HELLWAVE_API AHellWaveArenaGameMode : public AShooterGameMode
{
	GENERATED_BODY()

protected:

	/** Wave manager class to instantiate */
	UPROPERTY(EditAnywhere, Category="HellWave")
	TSubclassOf<UHellWaveWaveManager> WaveManagerClass;

	/** Tag used to find spawn point actors in the level */
	UPROPERTY(EditAnywhere, Category="HellWave")
	FName SpawnPointTag = FName("EnemySpawnPoint");

	/** Delay before starting waves after game begins */
	UPROPERTY(EditAnywhere, Category="HellWave", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float StartDelay = 2.0f;

	/** The wave manager instance */
	UPROPERTY()
	TObjectPtr<UHellWaveWaveManager> WaveManager;

	/** HUD widget class for HellWave */
	UPROPERTY(EditAnywhere, Category="HellWave|UI")
	TSubclassOf<UUserWidget> HellWaveHUDClass;

	/** Pointer to the HUD widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> HellWaveHUDWidget;

	/** Timer for start delay */
	FTimerHandle StartDelayTimer;

public:

	AHellWaveArenaGameMode();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Called after start delay to begin waves */
	void OnStartDelayComplete();

	/** Gather spawn points from the level by tag */
	TArray<AActor*> GatherSpawnPoints() const;

public:

	/** Called when the wave state changes */
	UFUNCTION()
	void OnWaveStateChanged(EHellWaveState NewState, int32 WaveNumber);

	/** Get the wave manager */
	UHellWaveWaveManager* GetWaveManager() const { return WaveManager; }

	/** Notify that the player has died — triggers defeat */
	void NotifyPlayerDeath();
};
