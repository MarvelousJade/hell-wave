// Copyright Epic Games, Inc. All Rights Reserved.

#include "HellWaveDashComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"

UHellWaveDashComponent::UHellWaveDashComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHellWaveDashComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentCharges = MaxCharges;
}

void UHellWaveDashComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(RechargeTimer);
}

bool UHellWaveDashComponent::TryDash(const FVector& Direction)
{
	if (CurrentCharges <= 0 || Direction.IsNearlyZero())
	{
		return false;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return false;
	}

	// Consume a charge
	--CurrentCharges;

	// Launch the character in the desired direction
	const FVector DashVelocity = Direction.GetSafeNormal() * DashImpulse;
	OwnerCharacter->LaunchCharacter(DashVelocity, true, true);

	// Start recharging
	StartRechargeIfNeeded();

	return true;
}

void UHellWaveDashComponent::OnRechargeComplete()
{
	if (CurrentCharges < MaxCharges)
	{
		++CurrentCharges;
	}

	// Continue recharging if still below max
	StartRechargeIfNeeded();
}

void UHellWaveDashComponent::StartRechargeIfNeeded()
{
	if (CurrentCharges < MaxCharges && !GetWorld()->GetTimerManager().IsTimerActive(RechargeTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(RechargeTimer, this, &UHellWaveDashComponent::OnRechargeComplete, RechargeTime, false);
	}
}
