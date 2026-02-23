// Copyright Epic Games, Inc. All Rights Reserved.

#include "HellWaveSuperShotgun.h"
#include "ShooterWeaponHolder.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

AHellWaveSuperShotgun::AHellWaveSuperShotgun()
{
	// Super Shotgun defaults
	MagazineSize = 2;
	MaxReserveAmmo = 30;
	ReserveAmmo = 30;
	bFullAuto = false;
	RefireRate = 0.8f;
	FiringRecoil = -2.0f;
	ReloadTime = 0.6f;
}

void AHellWaveSuperShotgun::FireHitscan(const FVector& TargetLocation)
{
	const FVector MuzzleLoc = GetFirstPersonMesh()->GetSocketLocation(MuzzleSocketName);
	const FVector BaseDir = (TargetLocation - MuzzleLoc).GetSafeNormal();
	FHitResult HitResult;
	
	// Fire multiple pellets in a cone
	for (int32 i = 0; i < PelletCount; ++i)
	{
		// Randomize direction within the spread cone
		const FVector PelletDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(BaseDir, SpreadHalfAngle);
		const FVector TraceStart = MuzzleLoc + (PelletDir * MuzzleOffset);
		const FVector TraceEnd = TraceStart + (PelletDir * HitscanRange);
		
		GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility);
		
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			HitResult.bBlockingHit ? FColor::Green : FColor::Red, // Color changes based on whether a hit occurred
			false,          // Persistent lines? (false for temporary, true for permanent)
			5.0f,           // Life time in seconds (e.g., 5.0f or -1.0f for one frame)
			0,              // Depth priority
			1.0f            // Thickness
		);
		
		if (HitResult.bBlockingHit)
		{
			UE_LOG(LogTemp, Warning, TEXT("Calling ProcessHitscan"));
			ProcessHitscan(HitResult, PelletDir);
		}
	}
	
	// Play effects (once for all pellets)
	WeaponOwner->PlayFiringMontage(FiringMontage);
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// Consume one magazine round
	--CurrentBullets;

	// Update HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);

	// Auto-reload when empty
	if (CurrentBullets <= 0 && ReserveAmmo > 0 && bAutoReloadFromReserve)
	{
		ReloadFromReserve();
	}
}