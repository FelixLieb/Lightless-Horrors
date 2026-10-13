#include "EnemyAIController.h"
#include "LimbComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyAIController::AEnemyAIController()
{
	Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
	SetPerceptionComponent(*Perception);

	UAISenseConfig_Sight* Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	Sight->SightRadius = 2000.f;
	Sight->LoseSightRadius = 2300.f;
	Sight->PeripheralVisionAngleDegrees = 70.f;
	Sight->DetectionByAffiliation.bDetectEnemies = true;
	Sight->DetectionByAffiliation.bDetectNeutrals = true;
	Sight->DetectionByAffiliation.bDetectFriendlies = true;

	UAISenseConfig_Hearing* Hearing = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	Hearing->HearingRange = 1500.f;
	Hearing->DetectionByAffiliation.bDetectEnemies = true;
	Hearing->DetectionByAffiliation.bDetectNeutrals = true;
	Hearing->DetectionByAffiliation.bDetectFriendlies = true;

	Perception->ConfigureSense(*Sight);
	Perception->ConfigureSense(*Hearing);
	Perception->SetDominantSense(Sight->GetSenseImplementation());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	Limbs = InPawn ? InPawn->FindComponentByClass<ULimbComponent>() : nullptr;
	if (!Limbs) return;

	Limbs->OnLimbDied.AddDynamic(this, &AEnemyAIController::HandleLimbDied);
	RefreshSenses();
	RefreshMovement();
}

void AEnemyAIController::HandleLimbDied(FName LimbName, ELimbRole Role)
{
	switch (Role)
	{
	case ELimbRole::Eye:
	case ELimbRole::Ear: RefreshSenses();   break;
	case ELimbRole::Leg: RefreshMovement(); break;
	default: break;
	}
}

void AEnemyAIController::RefreshSenses()
{
	if (!Limbs || !Perception) return;

	// No eyes alive = blind. No ears alive = deaf.
	Perception->UpdatePerceptionAllowList(UAISense::GetSenseID<UAISense_Sight>(),
		Limbs->CountAlive(ELimbRole::Eye) > 0);
	Perception->UpdatePerceptionAllowList(UAISense::GetSenseID<UAISense_Hearing>(),
		Limbs->CountAlive(ELimbRole::Ear) > 0);
}

void AEnemyAIController::RefreshMovement()
{
	ACharacter* Char = Cast<ACharacter>(GetPawn());
	if (!Char || !Limbs) return;

	const float Mult = Limbs->GetMoveSpeedMultiplier();
	Char->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * Mult;
	if (Mult <= 0.f)
	{
		StopMovement(); // no legs left
	}
}

bool AEnemyAIController::CanAttack() const
{
	return Limbs && Limbs->CanAttack();
}