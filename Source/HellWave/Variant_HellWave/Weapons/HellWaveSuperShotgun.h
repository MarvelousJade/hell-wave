// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HellWaveWeapon.h"
#include "HellWaveSuperShotgun.generated.h"

/**
 *  Super Shotgun weapon — hitscan shotgun that fires multiple pellet traces in a cone
 *  Each trigger pull fires all pellets simultaneously
 *  Alt-fire launches the Meat Hook (Phase 2)
 */
UCLASS(abstract)
class HELLWAVE_API AHellWaveSuperShotgun : public AHellWaveWeapon
{
	GENERATED_BODY()

protected:

	/** Number of pellets (line traces) fired per shot */
	UPROPERTY(EditAnywhere, Category="Shotgun", meta = (ClampMin = 1, ClampMax = 20))
	int32 PelletCount = 8;

	/** Half-angle of the pellet spread cone in degrees */
	UPROPERTY(EditAnywhere, Category="Shotgun", meta = (ClampMin = 0, ClampMax = 45, Units = "Degrees"))
	float SpreadHalfAngle = 8.0f;

public:

	AHellWaveSuperShotgun();

protected:

	/** Override to fire multiple hitscan pellets in a spread pattern */
	virtual void FireHitscan(const FVector& TargetLocation) override;
};
