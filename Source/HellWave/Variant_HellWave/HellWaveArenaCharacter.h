// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterCharacter.h"
#include "HellWaveArenaCharacter.generated.h"

class UHellWaveDashComponent;
class UInputAction;
class AHellWaveEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FArmorUpdatedDelegate, float, CurrentArmor, float, MaxArmor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDashChargesUpdatedDelegate, int32, CurrentCharges, int32, MaxCharges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChainsawFuelUpdatedDelegate, int32, CurrentFuel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlameBelchCooldownDelegate, float, CooldownPercent);

/**
 *  DOOM Eternal-inspired arena character
 *  Extends AShooterCharacter with armor, dash, double jump,
 *  glory kill, chainsaw, and flame belch abilities
 */
UCLASS(abstract)
class HELLWAVE_API AHellWaveArenaCharacter : public AShooterCharacter
{
	GENERATED_BODY()

	/** Dash ability component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UHellWaveDashComponent* DashComponent;

protected:

	// --- Input Actions ---

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* GloryKillAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ChainsawAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* FlameBelchAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* AltFireAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* WeaponSlot1Action;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* WeaponSlot2Action;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* WeaponSlot3Action;

	// --- Armor ---

	UPROPERTY(EditAnywhere, Category="Armor", meta = (ClampMin = 0, ClampMax = 200))
	float MaxArmor = 100.0f;

	float CurrentArmor = 0.0f;

	/** Fraction of damage absorbed by armor (0-1) */
	UPROPERTY(EditAnywhere, Category="Armor", meta = (ClampMin = 0, ClampMax = 1))
	float ArmorAbsorption = 0.5f;

	// --- Glory Kill ---

	/** Max distance to detect a staggered enemy for glory kill */
	UPROPERTY(EditAnywhere, Category="Glory Kill", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float GloryKillRange = 250.0f;

	/** Duration of invulnerability during glory kill */
	UPROPERTY(EditAnywhere, Category="Glory Kill", meta = (ClampMin = 0, ClampMax = 2, Units = "s"))
	float GloryKillInvulnTime = 0.5f;

	/** Health restored per glory kill */
	UPROPERTY(EditAnywhere, Category="Glory Kill", meta = (ClampMin = 0, ClampMax = 200))
	float GloryKillHealthRestore = 50.0f;

	bool bIsInvulnerable = false;

	FTimerHandle InvulnTimer;

	// --- Chainsaw ---

	/** Max chainsaw fuel charges */
	UPROPERTY(EditAnywhere, Category="Chainsaw", meta = (ClampMin = 1, ClampMax = 5))
	int32 MaxChainsawFuel = 3;

	int32 CurrentChainsawFuel = 3;

	/** Range for chainsaw kill */
	UPROPERTY(EditAnywhere, Category="Chainsaw", meta = (ClampMin = 0, ClampMax = 500, Units = "cm"))
	float ChainsawRange = 200.0f;

	/** Ammo restored per weapon on chainsaw kill */
	UPROPERTY(EditAnywhere, Category="Chainsaw", meta = (ClampMin = 0, ClampMax = 50))
	int32 ChainsawAmmoRestore = 10;

	// --- Flame Belch ---

	/** Cooldown duration for flame belch */
	UPROPERTY(EditAnywhere, Category="Flame Belch", meta = (ClampMin = 0, ClampMax = 30, Units = "s"))
	float FlameBelchCooldown = 15.0f;

	/** Range of the flame belch cone */
	UPROPERTY(EditAnywhere, Category="Flame Belch", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float FlameBelchRange = 600.0f;

	/** Half-angle of the flame belch cone */
	UPROPERTY(EditAnywhere, Category="Flame Belch", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float FlameBelchHalfAngle = 30.0f;

	/** Duration enemies stay on fire */
	UPROPERTY(EditAnywhere, Category="Flame Belch", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float FlameBelchBurnDuration = 5.0f;

	bool bFlameBelchReady = true;

	FTimerHandle FlameBelchTimer;

	// --- Double Jump ---

	/** Number of additional jumps allowed (1 = double jump) */
	UPROPERTY(EditAnywhere, Category="Movement", meta = (ClampMin = 0, ClampMax = 3))
	int32 ExtraJumps = 1;

	int32 JumpsRemaining = 0;

public:

	/** Delegates for UI updates */
	FArmorUpdatedDelegate OnArmorUpdated;
	FDashChargesUpdatedDelegate OnDashChargesUpdated;
	FChainsawFuelUpdatedDelegate OnChainsawFuelUpdated;
	FFlameBelchCooldownDelegate OnFlameBelchCooldownUpdated;

public:

	AHellWaveArenaCharacter();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Landed(const FHitResult& Hit) override;
	virtual void DoJumpStart() override;

	// --- Ability Inputs ---

	void DoDash();
	void DoGloryKill();
	void DoChainsaw();
	void DoFlameBelch();
	void DoAltFire();
	void DoSelectWeapon1();
	void DoSelectWeapon2();
	void DoSelectWeapon3();
	void DoScrollWeapon(const FInputActionValue& Value);

	// --- Resource Modifiers ---

	/** Add health, clamped to MaxHP */
	void AddHealth(float Amount);

	/** Add armor, clamped to MaxArmor */
	void AddArmor(float Amount);

	/** Add reserve ammo to all owned weapons */
	void AddAmmoToAllWeapons(int32 Amount);

	/** Refuel chainsaw by one charge */
	void AddChainsawFuel(int32 Amount);

	// --- Accessors ---

	float GetCurrentArmor() const { return CurrentArmor; }
	float GetMaxArmor() const { return MaxArmor; }
	int32 GetChainsawFuel() const { return CurrentChainsawFuel; }
	bool IsFlameBelchReady() const { return bFlameBelchReady; }
	UHellWaveDashComponent* GetDashComponent() const { return DashComponent; }

protected:

	/** Find the nearest staggered enemy within range */
	AHellWaveEnemy* FindGloryKillTarget() const;

	/** Find the nearest enemy within chainsaw range */
	AHellWaveEnemy* FindChainsawTarget() const;

	/** End invulnerability after glory kill */
	void EndInvulnerability();

	/** Reset flame belch cooldown */
	void OnFlameBelchCooldownExpired();

	/** Select a weapon by index in the owned weapons array */
	void SelectWeaponByIndex(int32 Index);
};
