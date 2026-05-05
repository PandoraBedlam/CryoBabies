// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CBSlipperyComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CRYOBABIES_API UCBSlipperyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCBSlipperyComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components")
	class USphereComponent* SphereOverlapComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	FTimerHandle TimerHandle_RagdollRecovery;

	UFUNCTION()
	void Recover(class ACBCharacter* RagdolledCharacter);

public:

	UFUNCTION()
	void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
