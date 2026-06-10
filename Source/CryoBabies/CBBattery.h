// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CBBattery.generated.h"

UCLASS()
class CRYOBABIES_API ACBBattery : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACBBattery();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Battery|Visuals")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category="Battery|Properties")
	float ChargeAmount = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category="Battery|Properties")
	float MaxChargeAmount = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="Battery|Properties")
	float ChargePercentage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category="Battery|Properties")
	bool bIsInserted = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	//Used to initialise ChangeCharge
	float m_deltaChargeSpeed = 0;

	FTimerHandle TimerHandle_Charge;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	float GetChargeAmount() const { return ChargeAmount; }
	void SetChargeAmount(float deltaCharge) { ChargeAmount = deltaCharge; }

	//These should be used to begin/end timers that will happen locally to reduce bandwidth usage
	//Supports positive and negative for charging or depletion
	UFUNCTION(Reliable, Server)
	void StartCharge(float deltaChargeSpeed);

	UFUNCTION()
	void ChangeCharge();

	UFUNCTION(Reliable, Server)
	void EndCharge();
};
