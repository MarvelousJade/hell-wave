// Copyright Epic Games, Inc. All Rights Reserved.

#include "HellWaveArenaPlayerController.h"
#include "HellWaveArenaCharacter.h"
#include "HellWaveArenaGameMode.h"
#include "HellWaveHUD.h"
#include "Engine/World.h"

void AHellWaveArenaPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Create the HellWave HUD
	if (IsLocalPlayerController() && HellWaveHUDClass)
	{
		HellWaveHUD = CreateWidget<UHellWaveHUD>(this, HellWaveHUDClass);
		if (HellWaveHUD)
		{
			HellWaveHUD->AddToPlayerScreen(1);
		}
	}
}

void AHellWaveArenaPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Bind HellWave-specific delegates
	if (AHellWaveArenaCharacter* ArenaChar = Cast<AHellWaveArenaCharacter>(InPawn))
	{
		BindCharacterDelegates(ArenaChar);
	}

	// Override the destroy handler — no respawning in arena mode
	InPawn->OnDestroyed.AddDynamic(this, &AHellWaveArenaPlayerController::OnArenaPawnDestroyed);
}

void AHellWaveArenaPlayerController::OnArenaPawnDestroyed(AActor* DestroyedActor)
{
	// Notify the game mode that the player died
	if (AHellWaveArenaGameMode* GM = Cast<AHellWaveArenaGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->NotifyPlayerDeath();
	}
}

void AHellWaveArenaPlayerController::BindCharacterDelegates(AHellWaveArenaCharacter* HellCharacter)
{
	HellCharacter->OnArmorUpdated.AddDynamic(this, &AHellWaveArenaPlayerController::OnArmorUpdated);
	HellCharacter->OnDashChargesUpdated.AddDynamic(this, &AHellWaveArenaPlayerController::OnDashChargesUpdated);
	HellCharacter->OnChainsawFuelUpdated.AddDynamic(this, &AHellWaveArenaPlayerController::OnChainsawFuelUpdated);
	HellCharacter->OnFlameBelchCooldownUpdated.AddDynamic(this, &AHellWaveArenaPlayerController::OnFlameBelchCooldownUpdated);
}

void AHellWaveArenaPlayerController::OnArmorUpdated(float CurrentArmor, float MaxArmor)
{
	if (HellWaveHUD)
	{
		HellWaveHUD->BP_UpdateArmor(CurrentArmor, MaxArmor);
	}
}

void AHellWaveArenaPlayerController::OnDashChargesUpdated(int32 CurrentCharges, int32 MaxCharges)
{
	if (HellWaveHUD)
	{
		HellWaveHUD->BP_UpdateDashCharges(CurrentCharges, MaxCharges);
	}
}

void AHellWaveArenaPlayerController::OnChainsawFuelUpdated(int32 CurrentFuel)
{
	if (HellWaveHUD)
	{
		HellWaveHUD->BP_UpdateChainsawFuel(CurrentFuel);
	}
}

void AHellWaveArenaPlayerController::OnFlameBelchCooldownUpdated(float CooldownPercent)
{
	if (HellWaveHUD)
	{
		HellWaveHUD->BP_UpdateFlameBelchCooldown(CooldownPercent);
	}
}
