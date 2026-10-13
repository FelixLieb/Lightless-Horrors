#pragma once

#include "CoreMinimal.h"
#include "LimbTypes.generated.h"

// Player damage types.
// Slashing beats low Hardness, Piercing beats low Thickness, Blunt beats low ShockAbsorption.
UENUM(BlueprintType)
enum class EDamageKind : uint8
{
	Slashing,
	Piercing,
	Blunt
};

UENUM(BlueprintType)
enum class ELimbRole : uint8
{
	Leg,
	Arm,
	Eye,
	Ear,
	Other
};

// Outer protective layer. All values 0..1
USTRUCT(BlueprintType)
struct FLimbArmor
{
	GENERATED_BODY()

	// High = armor wears down slowly when hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float Durability = 0.5f;

	// Resists Piercing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float Thickness = 0.5f;

	// Resists Slashing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float Hardness = 0.5f;

	// Resists Blunt
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "1"))
	float ShockAbsorption = 0.5f;
};

USTRUCT(BlueprintType)
struct FLimb
{
	GENERATED_BODY()

	// Unique name, e.g. "LegL_Thigh"
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;

	// Parent limb name. Leave empty if attached directly to the body.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ParentName;

	// Skeleton bone that represents this limb (used for hit detection)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Bone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELimbRole Role = ELimbRole::Other;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP = 50.f;

	UPROPERTY(BlueprintReadOnly)
	float HP = 50.f;

	UPROPERTY(BlueprintReadOnly)
	bool bDead = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLimbArmor Armor;
};
