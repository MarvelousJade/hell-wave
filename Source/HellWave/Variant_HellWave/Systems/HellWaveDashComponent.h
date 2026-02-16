// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HellWaveDashComponent.generated.h"

/**
 *  Dash ability component with charge-based cooldown
 *  Attach to a character to give them a multi-charge dash
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HELLWAVE_API UHellWaveDashComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	/** Impulse strength applied during dash */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0, ClampMax = 10000))
	float DashImpulse = 2000.0f;

	/** Maximum number of dash charges */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 1, ClampMax = 5))
	int32 MaxCharges = 2;

	/** Time in seconds to recharge one dash charge */
	UPROPERTY(EditAnywhere, Category="Dash", meta = (ClampMin = 0.1, ClampMax = 10, Units = "s"))
	float RechargeTime = 1.5f;

	/** Current available dash charges */
	int32 CurrentCharges = 0;

	/** Timer for recharging a single charge */
	FTimerHandle RechargeTimer;

public:

	UHellWaveDashComponent();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

	/** Attempt a dash in the given world direction. Returns true if dash was performed. */
	bool TryDash(const FVector& Direction);

	/** Returns current charge count */
	int32 GetCurrentCharges() const { return CurrentCharges; }

	/** Returns max charge count */
	int32 GetMaxCharges() const { return MaxCharges; }

protected:

	/** Called when a charge finishes recharging */
	void OnRechargeComplete();

	/** Starts the recharge timer if charges are below max */
	void StartRechargeIfNeeded();
};
