// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterWeapon.h"
#include "HellWaveWeapon.generated.h"

/**
 *  Base weapon for HellWave variant
 *  Overrides the auto-reload system with a reserve ammo pool
 *  Weapons cannot fire when both magazine and reserve ammo are depleted
 */
UCLASS(abstract)
class HELLWAVE_API AHellWaveWeapon : public AShooterWeapon
{
	GENERATED_BODY()
	
protected:

	/** Maximum reserve ammo capacity */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 500))
	int32 MaxReserveAmmo = 50;

	/** Current reserve ammo count */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 500))
	int32 ReserveAmmo = 50;

	/** If true, weapon will auto-reload from reserve when magazine is empty and trigger is released */
	UPROPERTY(EditAnywhere, Category="Ammo")
	bool bAutoReloadFromReserve = true;

	/** Time to reload from reserve */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float ReloadTime = 0.5f;

	/** True while reloading */
	bool bIsReloading = false;

	FTimerHandle ReloadTimer;

	UPROPERTY(EditDefaultsOnly, Category="Sound")
	USoundBase* DryFireSound;
	
	// -- Hit Scan --
	
	UPROPERTY(EditDefaultsOnly, Category="Hitscan", meta = (ClampMin = 0, ClampMax = 50000, Units = "cm"))
	float HitscanRange = 5000.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Hitscan", meta = (ClampMin = 0, ClampMax = 500))
	float HitscanDamage = 25.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Hitscan", meta = (ClampMin = 0, ClampMax = 50000))
	float HitscanImpulse = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, Category="Hitscan")
	TSubclassOf<UDamageType> HitscanDamageType;
	
	UPROPERTY(EditDefaultsOnly, Category="Hitscan")
	bool bShouldFireHitscan = true;
	
public:

	AHellWaveWeapon();

protected:

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Overridden to use reserve ammo instead of auto-reload */
	virtual void FireProjectile(const FVector& TargetLocation) override;

	/** Overridden to check ammo before firing */
	virtual void Fire() override;

	/** Reload magazine from reserve */
	void ReloadFromReserve();

	/** Called when reload is complete */
	void OnReloadComplete();
	
	virtual void FireHitscan(const FVector& TargetLocation);
	
	void ProcessHitscan(const FHitResult& Hit, const FVector& ShotDirection);
	
	/** Blueprint event for pre-hit effects (decals, particles, sounds) */
	UFUNCTION(BlueprintImplementableEvent, Category="Hitscan", meta = (DisplayName = "On Hitscan Hit"))
	void BP_OnHitscanHit(const FHitResult& Hit);

public:

	/** Add ammo to the reserve pool */
	void AddReserveAmmo(int32 Amount);

	/** Returns current reserve ammo */
	int32 GetReserveAmmo() const { return ReserveAmmo; }

	/** Returns max reserve ammo */
	int32 GetMaxReserveAmmo() const { return MaxReserveAmmo; }

	/** Returns true if the weapon has any ammo (magazine + reserve) */
	bool HasAmmo() const { return CurrentBullets > 0 || ReserveAmmo > 0; }

	/** Returns true if currently reloading */
	bool IsReloading() const { return bIsReloading; }
};
