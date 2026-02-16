// Copyright Epic Games, Inc. All Rights Reserved.

#include "HellWaveArenaCharacter.h"
#include "HellWaveDashComponent.h"
#include "HellWaveEnemy.h"
#include "HellWaveWeapon.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

AHellWaveArenaCharacter::AHellWaveArenaCharacter()
{
	// Create dash component
	DashComponent = CreateDefaultSubobject<UHellWaveDashComponent>(TEXT("DashComponent"));

	// DOOM-like movement: fast, snappy
	GetCharacterMovement()->MaxWalkSpeed = 900.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;
	GetCharacterMovement()->AirControl = 0.8f;
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->GravityScale = 1.5f;
}

void AHellWaveArenaCharacter::BeginPlay()
{
	Super::BeginPlay();
	CurrentArmor = 0.0f;
	JumpsRemaining = ExtraJumps;
	CurrentChainsawFuel = MaxChainsawFuel;
	bFlameBelchReady = true;
}

void AHellWaveArenaCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(InvulnTimer);
	GetWorld()->GetTimerManager().ClearTimer(FlameBelchTimer);
}

void AHellWaveArenaCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Skip AShooterCharacter::SetupPlayerInputComponent — it binds SwitchWeaponAction
	// to the forward-only DoSwitchWeapon. We rebind fire + scroll ourselves.
	AHellWaveCharacter::SetupPlayerInputComponent(PlayerInputComponent);

	UE_LOG(LogTemp, Warning, TEXT("AHellWaveArenaCharacter::SetupPlayerInputComponent called."));
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Rebind fire (originally in AShooterCharacter)
		EIC->BindAction(FireAction, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoStartFiring);
		EIC->BindAction(FireAction, ETriggerEvent::Completed, this, &AHellWaveArenaCharacter::DoStopFiring);
		UE_LOG(LogTemp, Warning, TEXT("Bound FireAction to DoStartFiring and DoStopFiring."));
		
		// Bidirectional scroll weapon (replaces parent's forward-only switch)
		if (SwitchWeaponAction)
		{
			EIC->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AHellWaveArenaCharacter::DoScrollWeapon);
			UE_LOG(LogTemp, Warning, TEXT("Bound SwitchWeaponAction to DoScrollWeapon for bidirectional weapon switching."));
		}

		if (DashAction)
		{
			EIC->BindAction(DashAction, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoDash);
		}
		if (GloryKillAction)
		{
			EIC->BindAction(GloryKillAction, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoGloryKill);
		}
		if (ChainsawAction)
		{
			EIC->BindAction(ChainsawAction, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoChainsaw);
		}
		if (FlameBelchAction)
		{
			EIC->BindAction(FlameBelchAction, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoFlameBelch);
		}
		if (AltFireAction)
		{
			EIC->BindAction(AltFireAction, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoAltFire);
		}
		if (WeaponSlot1Action)
		{
			EIC->BindAction(WeaponSlot1Action, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoSelectWeapon1);
		}
		if (WeaponSlot2Action)
		{
			EIC->BindAction(WeaponSlot2Action, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoSelectWeapon2);
		}
		if (WeaponSlot3Action)
		{
			EIC->BindAction(WeaponSlot3Action, ETriggerEvent::Started, this, &AHellWaveArenaCharacter::DoSelectWeapon3);
		}
	}
}

float AHellWaveArenaCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsInvulnerable || IsDead())
	{
		return 0.0f;
	}

	// Armor absorbs a fraction of damage
	float ArmorDamage = 0.0f;
	if (CurrentArmor > 0.0f)
	{
		ArmorDamage = Damage * ArmorAbsorption;
		ArmorDamage = FMath::Min(ArmorDamage, CurrentArmor);
		CurrentArmor -= ArmorDamage;
		OnArmorUpdated.Broadcast(CurrentArmor, MaxArmor);
	}

	// Remaining damage goes to health
	const float HealthDamage = Damage - ArmorDamage;
	return Super::TakeDamage(HealthDamage, DamageEvent, EventInstigator, DamageCauser);
}

void AHellWaveArenaCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	JumpsRemaining = ExtraJumps;
}

void AHellWaveArenaCharacter::DoJumpStart()
{
	if (IsDead()) return;

	if (GetCharacterMovement()->IsMovingOnGround())
	{
		// Normal ground jump
		Super::DoJumpStart();
		JumpsRemaining = ExtraJumps;
	}
	else if (JumpsRemaining > 0)
	{
		// Air jump (double jump)
		--JumpsRemaining;
		LaunchCharacter(FVector(0.0f, 0.0f, GetCharacterMovement()->JumpZVelocity), false, true);
	}
}

// --- Ability Inputs ---

void AHellWaveArenaCharacter::DoDash()
{
	if (IsDead()) return;

	// Determine dash direction from movement input, or forward if no input
	FVector DashDir = GetLastMovementInputVector();
	if (DashDir.IsNearlyZero())
	{
		DashDir = GetActorForwardVector();
	}

	DashComponent->TryDash(DashDir);
	OnDashChargesUpdated.Broadcast(DashComponent->GetCurrentCharges(), DashComponent->GetMaxCharges());
}

void AHellWaveArenaCharacter::DoGloryKill()
{
	if (IsDead()) return;

	AHellWaveEnemy* Target = FindGloryKillTarget();
	if (!Target) return;

	// Become briefly invulnerable
	bIsInvulnerable = true;
	GetWorld()->GetTimerManager().SetTimer(InvulnTimer, this, &AHellWaveArenaCharacter::EndInvulnerability, GloryKillInvulnTime, false);

	// Kill the enemy via glory kill
	Target->OnGloryKilled();

	// Restore health
	AddHealth(GloryKillHealthRestore);
}

void AHellWaveArenaCharacter::DoChainsaw()
{
	if (IsDead() || CurrentChainsawFuel <= 0) return;

	AHellWaveEnemy* Target = FindChainsawTarget();
	if (!Target) return;

	// Consume fuel
	--CurrentChainsawFuel;
	OnChainsawFuelUpdated.Broadcast(CurrentChainsawFuel);

	// Kill the enemy via chainsaw
	Target->OnChainsawKilled();

	// Restore ammo to all weapons
	AddAmmoToAllWeapons(ChainsawAmmoRestore);
}

void AHellWaveArenaCharacter::DoFlameBelch()
{
	if (IsDead() || !bFlameBelchReady) return;

	// Put on cooldown
	bFlameBelchReady = false;
	GetWorld()->GetTimerManager().SetTimer(FlameBelchTimer, this, &AHellWaveArenaCharacter::OnFlameBelchCooldownExpired, FlameBelchCooldown, false);
	OnFlameBelchCooldownUpdated.Broadcast(0.0f);

	// Find enemies in cone and apply burning
	const FVector Origin = GetActorLocation();
	const FVector Forward = GetFirstPersonCameraComponent()->GetForwardVector();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHellWaveEnemy::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AHellWaveEnemy* Enemy = Cast<AHellWaveEnemy>(Actor);
		if (!Enemy || Enemy->IsEnemyDead()) continue;

		const FVector ToEnemy = Enemy->GetActorLocation() - Origin;
		const float Distance = ToEnemy.Size();
		if (Distance > FlameBelchRange) continue;

		// Check cone angle
		const float DotProduct = FVector::DotProduct(Forward, ToEnemy.GetSafeNormal());
		const float ConeThreshold = FMath::Cos(FMath::DegreesToRadians(FlameBelchHalfAngle));
		if (DotProduct >= ConeThreshold)
		{
			Enemy->ApplyBurning(FlameBelchBurnDuration);
		}
	}
}

void AHellWaveArenaCharacter::DoAltFire()
{
	// Will be used by Super Shotgun for Meat Hook — delegated to weapon
	// For now, stub
}

void AHellWaveArenaCharacter::DoSelectWeapon1() { SelectWeaponByIndex(0); }
void AHellWaveArenaCharacter::DoSelectWeapon2() { SelectWeaponByIndex(1); }
void AHellWaveArenaCharacter::DoSelectWeapon3() { SelectWeaponByIndex(2); }

void AHellWaveArenaCharacter::DoScrollWeapon(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("DoScrollWeapon triggered with value: %f"), Value.Get<float>());
	if (IsDead() || OwnedWeapons.Num() <= 1) return;

	const float Direction = Value.Get<float>();
	if (FMath::IsNearlyZero(Direction)) return;

	int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);
	const int32 NumWeapons = OwnedWeapons.Num();

	if (Direction > 0)
	{
		WeaponIndex = (WeaponIndex - 1 + NumWeapons) % NumWeapons;
	}
	else
	{
		WeaponIndex = (WeaponIndex + 1) % NumWeapons;
	}

	SelectWeaponByIndex(WeaponIndex);
}

// --- Resource Modifiers ---

void AHellWaveArenaCharacter::AddHealth(float Amount)
{
	CurrentHP = FMath::Min(CurrentHP + Amount, MaxHP);
	OnDamaged.Broadcast(CurrentHP / MaxHP);
}

void AHellWaveArenaCharacter::AddArmor(float Amount)
{
	CurrentArmor = FMath::Min(CurrentArmor + Amount, MaxArmor);
	OnArmorUpdated.Broadcast(CurrentArmor, MaxArmor);
}

void AHellWaveArenaCharacter::AddAmmoToAllWeapons(int32 Amount)
{
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (AHellWaveWeapon* HWWeapon = Cast<AHellWaveWeapon>(Weapon))
		{
			HWWeapon->AddReserveAmmo(Amount);
		}
	}

	// Update HUD for current weapon
	if (CurrentWeapon)
	{
		UpdateWeaponHUD(CurrentWeapon->GetBulletCount(), CurrentWeapon->GetMagazineSize());
	}
}

void AHellWaveArenaCharacter::AddChainsawFuel(int32 Amount)
{
	CurrentChainsawFuel = FMath::Min(CurrentChainsawFuel + Amount, MaxChainsawFuel);
	OnChainsawFuelUpdated.Broadcast(CurrentChainsawFuel);
}

// --- Helpers ---

AHellWaveEnemy* AHellWaveArenaCharacter::FindGloryKillTarget() const
{
	const FVector Origin = GetActorLocation();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHellWaveEnemy::StaticClass(), FoundActors);

	AHellWaveEnemy* ClosestTarget = nullptr;
	float ClosestDistance = GloryKillRange;

	for (AActor* Actor : FoundActors)
	{
		AHellWaveEnemy* Enemy = Cast<AHellWaveEnemy>(Actor);
		if (!Enemy || !Enemy->IsStaggered()) continue;

		const float Distance = FVector::Dist(Origin, Enemy->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestTarget = Enemy;
		}
	}

	return ClosestTarget;
}

AHellWaveEnemy* AHellWaveArenaCharacter::FindChainsawTarget() const
{
	const FVector Origin = GetActorLocation();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AHellWaveEnemy::StaticClass(), FoundActors);

	AHellWaveEnemy* ClosestTarget = nullptr;
	float ClosestDistance = ChainsawRange;

	for (AActor* Actor : FoundActors)
	{
		AHellWaveEnemy* Enemy = Cast<AHellWaveEnemy>(Actor);
		if (!Enemy || Enemy->IsEnemyDead()) continue;

		const float Distance = FVector::Dist(Origin, Enemy->GetActorLocation());
		if (Distance < ClosestDistance)
		{
			ClosestDistance = Distance;
			ClosestTarget = Enemy;
		}
	}

	return ClosestTarget;
}

void AHellWaveArenaCharacter::EndInvulnerability()
{
	bIsInvulnerable = false;
}

void AHellWaveArenaCharacter::OnFlameBelchCooldownExpired()
{
	bFlameBelchReady = true;
	OnFlameBelchCooldownUpdated.Broadcast(1.0f);
}

void AHellWaveArenaCharacter::SelectWeaponByIndex(int32 Index)
{
	if (IsDead() || OwnedWeapons.Num() <= Index || Index < 0) return;
	if (OwnedWeapons[Index] == CurrentWeapon) return;

	CurrentWeapon->DeactivateWeapon();
	CurrentWeapon = OwnedWeapons[Index];
	CurrentWeapon->ActivateWeapon();
}
