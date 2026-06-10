// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IInteractable.h"
#include "GameFramework/Actor.h"
#include "CBMachine.generated.h"

class ACBBattery;

UCLASS()
class CRYOBABIES_API ACBMachine : public AActor, public IIInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACBMachine();

	UPROPERTY(EditDefaultsOnly, Category = "Machine | Visuals")
	USkeletalMeshComponent* MeshComp;

	//'Current charge' depletes as time goes by
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Machine | Properties")
	float ChargeAmount = 100.f;

	//The max charge the machine can hold, it will recharge to this once batteries are inserted
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Machine | Properties")
	float MaxCharge = 100.f;

	//The rate of depletion of charge
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Machine | Properties")
	float ChargeDrainSpeed = 1.f;

	//If false-> it's off
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Machine | Properties")
	bool bIsPowered = false;

	//How many batteries can be attached
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Replicated, Category = "Machine | Properties")
	int MaxBatteryCount = 1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	FTimerHandle TimerHandle_DepletionTimer;

	UPROPERTY(BlueprintReadOnly)
	TArray<ACBBattery*> InsertedBatteries;

	void Timer_ApplyDepletion();

public:
	UFUNCTION(NetMulticast, Reliable)
	void AddBatteryRPC(ACBBattery* Battery);
	UFUNCTION(NetMulticast, Reliable)
	void RemoveBatteryRPC();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void AddBattery(ACBBattery* Battery, bool bForce = false);
	UFUNCTION(BlueprintCallable)
	void RemoveBattery();
};
