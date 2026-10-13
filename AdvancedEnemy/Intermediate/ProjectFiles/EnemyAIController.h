#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LimbTypes.h"
#include "EnemyAIController.generated.h"

class ULimbComponent;
class UAIPerceptionComponent;

UCLASS()
class ADVANCEDENEMY_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	float BaseWalkSpeed = 300.f;

	// Behavior Tree / Blackboard can read this before starting an attack
	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool CanAttack() const;

protected:
	virtual void OnPossess(APawn* InPawn) override;

private:
	UFUNCTION()
	void HandleLimbDied(FName LimbName, ELimbRole Role);

	void RefreshSenses();
	void RefreshMovement();

	UPROPERTY()
	TObjectPtr<UAIPerceptionComponent> Perception;

	UPROPERTY()
	TObjectPtr<ULimbComponent> Limbs;
};