#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LimbTypes.h"
#include "LimbComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLimbDied, FName, LimbName, ELimbRole, Role);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBodyDied);

// Put this on any enemy. Fill the Limbs array in the Details panel.
UCLASS(ClassGroup = (Combat), meta = (BlueprintSpawnableComponent))
class ADVANCEDENEMY_API ULimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULimbComponent();

	// ---- Setup (edit per enemy) ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limbs")
	TArray<FLimb> Limbs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limbs")
	float BodyMaxHP = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limbs")
	FLimbArmor BodyArmor;

	// Time the body takes to lose HP after a limb dies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limbs")
	float BleedSeconds = 5.f;

	// How much armor stats drop per hit (scaled by 1 - Durability)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limbs")
	float ArmorWearPerHit = 0.05f;

	UPROPERTY(BlueprintReadOnly, Category = "Limbs")
	float BodyHP = 0.f;

	// ---- Events ----
	UPROPERTY(BlueprintAssignable)
	FOnLimbDied OnLimbDied;

	UPROPERTY(BlueprintAssignable)
	FOnBodyDied OnBodyDied;

	// ---- Damage ----
	// Main entry point. Pass the hit result from your weapon trace.
	UFUNCTION(BlueprintCallable, Category = "Limbs")
	void DamageByHit(const FHitResult& Hit, float Damage, EDamageKind Kind);

	UFUNCTION(BlueprintCallable, Category = "Limbs")
	float DamageLimb(FName LimbName, float Damage, EDamageKind Kind);

	UFUNCTION(BlueprintCallable, Category = "Limbs")
	void DamageBody(float Damage, EDamageKind Kind);

	// ---- Queries for AI ----
	UFUNCTION(BlueprintPure, Category = "Limbs")
	int32 CountAlive(ELimbRole Role) const;

	// 0 = cannot walk, 1 = all leg parts alive
	UFUNCTION(BlueprintPure, Category = "Limbs")
	float GetMoveSpeedMultiplier() const;

	UFUNCTION(BlueprintPure, Category = "Limbs")
	bool CanWalk() const { return GetMoveSpeedMultiplier() > 0.f; }

	UFUNCTION(BlueprintPure, Category = "Limbs")
	bool CanAttack() const { return CountAlive(ELimbRole::Arm) > 0; }

	// Bones of the furthest ALIVE part of each leg (use these for foot IK / ground contact)
	UFUNCTION(BlueprintCallable, Category = "Limbs")
	void GetLegContactBones(TArray<FName>& OutBones) const;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	int32 IndexByName(FName Name) const;
	static float CalcMultiplier(const FLimbArmor& Armor, EDamageKind Kind);
	void WearArmor(FLimbArmor& Armor) const;
	void KillLimb(int32 Index);
	float KillChildren(FName ParentName); // returns total HP of killed children
	void AddBleed(float Amount);
	void ApplyBodyHP(float Amount);

	float BleedPool = 0.f;
	float BleedRate = 0.f;
	bool bBodyDead = false;
};