#include "LimbComponent.h"

ULimbComponent::ULimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // only ticks while bleeding
}

void ULimbComponent::BeginPlay()
{
	Super::BeginPlay();
	BodyHP = BodyMaxHP;
	for (FLimb& L : Limbs)
	{
		L.HP = L.MaxHP;
		L.bDead = false;
	}
}

int32 ULimbComponent::IndexByName(FName Name) const
{
	return Limbs.IndexOfByPredicate([Name](const FLimb& L) { return L.Name == Name; });
}

// ---------------- Damage ----------------

float ULimbComponent::CalcMultiplier(const FLimbArmor& A, EDamageKind Kind)
{
	float Resist = 0.f;
	switch (Kind)
	{
	case EDamageKind::Slashing: Resist = A.Hardness;        break;
	case EDamageKind::Piercing: Resist = A.Thickness;       break;
	case EDamageKind::Blunt:    Resist = A.ShockAbsorption; break;
	}
	// Resist 0 -> x1.5 damage, Resist 1 -> x0.5 damage
	return FMath::Lerp(1.5f, 0.5f, Resist);
}

void ULimbComponent::WearArmor(FLimbArmor& A) const
{
	const float Loss = ArmorWearPerHit * (1.f - A.Durability);
	A.Hardness = FMath::Max(0.f, A.Hardness - Loss);
	A.Thickness = FMath::Max(0.f, A.Thickness - Loss);
	A.ShockAbsorption = FMath::Max(0.f, A.ShockAbsorption - Loss);
}

void ULimbComponent::DamageByHit(const FHitResult& Hit, float Damage, EDamageKind Kind)
{
	const int32 Idx = Limbs.IndexOfByPredicate([&Hit](const FLimb& L) { return L.Bone == Hit.BoneName; });
	if (Idx != INDEX_NONE)
	{
		DamageLimb(Limbs[Idx].Name, Damage, Kind);
	}
	else
	{
		DamageBody(Damage, Kind); // bone not a limb -> torso
	}
}

float ULimbComponent::DamageLimb(FName LimbName, float Damage, EDamageKind Kind)
{
	const int32 Idx = IndexByName(LimbName);
	if (Idx == INDEX_NONE || Limbs[Idx].bDead) return 0.f;

	FLimb& L = Limbs[Idx];
	const float Final = Damage * CalcMultiplier(L.Armor, Kind);
	WearArmor(L.Armor);
	L.HP = FMath::Max(0.f, L.HP - Final);

	if (L.HP <= 0.f)
	{
		KillLimb(Idx);
	}
	return Final;
}

void ULimbComponent::DamageBody(float Damage, EDamageKind Kind)
{
	const float Final = Damage * CalcMultiplier(BodyArmor, Kind);
	WearArmor(BodyArmor);
	ApplyBodyHP(Final);
}

void ULimbComponent::ApplyBodyHP(float Amount)
{
	if (bBodyDead) return;
	BodyHP -= Amount;
	if (BodyHP <= 0.f)
	{
		BodyHP = 0.f;
		bBodyDead = true;
		OnBodyDied.Broadcast();
	}
}

// ---------------- Limb death + bleed ----------------

void ULimbComponent::KillLimb(int32 Index)
{
	FLimb& L = Limbs[Index];
	L.bDead = true;
	L.HP = 0.f;

	// Body takes: this limb's max HP + remaining HP of every child
	const float Pool = L.MaxHP + KillChildren(L.Name);
	AddBleed(Pool);
	OnLimbDied.Broadcast(L.Name, L.Role);
}

float ULimbComponent::KillChildren(FName ParentName)
{
	float Pool = 0.f;
	for (FLimb& C : Limbs)
	{
		if (C.bDead || C.ParentName != ParentName) continue;

		Pool += C.HP;
		C.bDead = true;
		C.HP = 0.f;
		Pool += KillChildren(C.Name);
		OnLimbDied.Broadcast(C.Name, C.Role);
	}
	return Pool;
}

void ULimbComponent::AddBleed(float Amount)
{
	BleedPool += Amount;
	BleedRate = BleedPool / FMath::Max(BleedSeconds, 0.1f);
	SetComponentTickEnabled(true);
}

void ULimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const float Step = FMath::Min(BleedPool, BleedRate * DeltaTime);
	BleedPool -= Step;
	ApplyBodyHP(Step); // bleed ignores armor

	if (BleedPool <= KINDA_SMALL_NUMBER)
	{
		BleedPool = 0.f;
		SetComponentTickEnabled(false);
	}
}

// ---------------- Queries ----------------

int32 ULimbComponent::CountAlive(ELimbRole Role) const
{
	int32 Count = 0;
	for (const FLimb& L : Limbs)
	{
		if (L.Role == Role && !L.bDead) ++Count;
	}
	return Count;
}

float ULimbComponent::GetMoveSpeedMultiplier() const
{
	int32 Total = 0, Alive = 0;
	for (const FLimb& L : Limbs)
	{
		if (L.Role != ELimbRole::Leg) continue;
		++Total;
		if (!L.bDead) ++Alive;
	}
	return Total > 0 ? (float)Alive / (float)Total : 0.f;
}

void ULimbComponent::GetLegContactBones(TArray<FName>& OutBones) const
{
	OutBones.Reset();
	for (const FLimb& L : Limbs)
	{
		if (L.Role != ELimbRole::Leg || L.bDead) continue;

		// "Tip" = alive leg part with no alive leg child. Children die with parents,
		// so this is always the furthest remaining part of that leg.
		const bool bHasAliveChild = Limbs.ContainsByPredicate([&L](const FLimb& C)
			{
				return C.Role == ELimbRole::Leg && !C.bDead && C.ParentName == L.Name;
			});
		if (!bHasAliveChild) OutBones.Add(L.Bone);
	}
}