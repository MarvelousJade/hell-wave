// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterPlayerController.h"
#include "HellWaveArenaPlayerController.generated.h"

class AHellWaveArenaCharacter;
class UHellWaveHUD;

/**
 *  Player Controller for HellWave arena mode
 *  Extends ShooterPlayerController with HellWave-specific HUD and
 *  disables respawning (arena mode = one life per game)
 */
UCLASS(abstract)
class HELLWAVE_API AHellWaveArenaPlayerController : public AShooterPlayerController
{
	GENERATED_BODY()

protected:

	/** HUD widget class */
	UPROPERTY(EditAnywhere, Category="HellWave|UI")
	TSubclassOf<UHellWaveHUD> HellWaveHUDClass;

	/** Pointer to the HUD widget */
	UPROPERTY()
	TObjectPtr<UHellWaveHUD> HellWaveHUD;

protected:

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	/** Override — no respawning in arena mode, notify game mode of death */
	UFUNCTION()
	void OnArenaPawnDestroyed(AActor* DestroyedActor);

	/** Bind to the HellWave character's delegates */
	void BindCharacterDelegates(AHellWaveArenaCharacter* Character);

	// --- HUD Update Callbacks ---

	UFUNCTION()
	void OnArmorUpdated(float CurrentArmor, float MaxArmor);

	UFUNCTION()
	void OnDashChargesUpdated(int32 CurrentCharges, int32 MaxCharges);

	UFUNCTION()
	void OnChainsawFuelUpdated(int32 CurrentFuel);

	UFUNCTION()
	void OnFlameBelchCooldownUpdated(float CooldownPercent);
};
