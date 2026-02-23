// Copyright Epic Games, Inc. All Rights Reserved.

#include "HellWaveWeapon.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AHellWaveWeapon::AHellWaveWeapon()
{
}

void AHellWaveWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);
}

void AHellWaveWeapon::Fire()
{
	if (!bIsFiring || bIsReloading)
	{
		return;
	}

	// Check if we have ammo in the magazine
	if (CurrentBullets <= 0)
	{
		// Try to reload from reserve
		if (ReserveAmmo > 0 && bAutoReloadFromReserve)
		{
			ReloadFromReserve();
		}
		else if (DryFireSound)
		{
			// Completely out of ammo — play dry fire click to push player toward chainsaw
			UGameplayStatics::PlaySoundAtLocation(this, DryFireSound, GetActorLocation());
		}
		return;
	}

	if (bShouldFireHitscan)
	{
		FireHitscan(WeaponOwner->GetWeaponTargetLocation());
	} else
	{
		// Fire a projectile at the target
		FireProjectile(WeaponOwner->GetWeaponTargetLocation());
	}

	// Update the time of our last shot
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// Make noise for AI perception
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	// Handle refire scheduling
	if (bFullAuto)
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AHellWaveWeapon::Fire, RefireRate, false);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AHellWaveWeapon::FireCooldownExpired, RefireRate, false);
	}
}

void AHellWaveWeapon::FireProjectile(const FVector& TargetLocation)
{
	// Calculate spawn transform
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);

	// Spawn the projectile
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

	// Play firing montage and recoil
	WeaponOwner->PlayFiringMontage(FiringMontage);
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// Consume magazine ammo — NO auto-reload
	--CurrentBullets;

	// Update HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);

	// If magazine is empty after firing, auto-reload from reserve
	if (CurrentBullets <= 0 && ReserveAmmo > 0 && bAutoReloadFromReserve && !bFullAuto)
	{
		ReloadFromReserve();
	}
}

void AHellWaveWeapon::ReloadFromReserve()
{
	if (bIsReloading || ReserveAmmo <= 0)
	{
		return;
	}

	bIsReloading = true;
	StopFiring();

	GetWorld()->GetTimerManager().SetTimer(ReloadTimer, this, &AHellWaveWeapon::OnReloadComplete, ReloadTime, false);
}

void AHellWaveWeapon::OnReloadComplete()
{
	bIsReloading = false;

	// Transfer ammo from reserve to magazine
	const int32 BulletsNeeded = MagazineSize - CurrentBullets;
	const int32 BulletsToLoad = FMath::Min(BulletsNeeded, ReserveAmmo);

	CurrentBullets += BulletsToLoad;
	ReserveAmmo -= BulletsToLoad;

	// Update HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

void AHellWaveWeapon::FireHitscan(const FVector& TargetLocation)
{
	FHitResult HitResult;
	const FVector MuzzleLoc = GetFirstPersonMesh()->GetSocketLocation(MuzzleSocketName);
	const FVector TraceDir = (TargetLocation - MuzzleLoc).GetSafeNormal();
	const FVector TraceStart = MuzzleLoc + (TraceDir * MuzzleOffset);
	
	GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceStart + (TraceDir * HitscanRange), ECC_Visibility);
	
	if (HitResult.bBlockingHit)
	{
		ProcessHitscan(HitResult, TraceDir);
	}
	
	// Play firing montage and recoil
	WeaponOwner->PlayFiringMontage(FiringMontage);
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// Consume magazine ammo — NO auto-reload
	--CurrentBullets;

	// Update HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);

	// If magazine is empty after firing, auto-reload from reserve
	if (CurrentBullets <= 0 && ReserveAmmo > 0 && bAutoReloadFromReserve && !bFullAuto)
	{
		ReloadFromReserve();
	}
}

void AHellWaveWeapon::ProcessHitscan(const FHitResult& HitResult, const FVector& ShotDirection)
{
	UE_LOG(LogTemp, Warning, TEXT("In ProcessHitscan"));
	UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s, Class: %s"),
		  *HitResult.GetActor()->GetName(),
		  *HitResult.GetActor()->GetClass()->GetName());	
	
	// have we hit a character?
	if (ACharacter* HitCharacter = Cast<ACharacter>(HitResult.GetActor()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Hitscan hit character: %s"), *HitCharacter->GetName());
		
		// ignore the owner of this projectile
		if (HitCharacter != GetOwner())
		{
			UE_LOG(LogTemp, Warning, TEXT("Owner: %s"), *(GetOwner()->GetName()));
			// apply damage to the character
			UGameplayStatics::ApplyDamage(
				HitCharacter, 
				HitscanDamage, 
				GetInstigatorController(), 
				this, 
				HitscanDamageType
			);
		}
	}

	// have we hit a physics object?
	UPrimitiveComponent* HitComp = HitResult.GetComponent();
	if (HitComp->IsSimulatingPhysics())
	{
		// give some physics impulse to the object
		HitComp->AddImpulseAtLocation(ShotDirection * HitscanImpulse, HitResult.ImpactPoint);
	}
}

void AHellWaveWeapon::AddReserveAmmo(int32 Amount)
{
	ReserveAmmo = FMath::Min(ReserveAmmo + Amount, MaxReserveAmmo);
}
