// Fill out your copyright notice in the Description page of Project Settings.


#include "CBMachine.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ACBMachine::ACBMachine()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
	RootComponent = MeshComp;
	MeshComp->SetCollisionProfileName("InteractableObject");	
}

// Called when the game starts or when spawned
void ACBMachine::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_DepletionTimer, this, &ACBMachine::Timer_ApplyDepletion, 1.f, true);
}

void ACBMachine::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACBMachine, ChargeAmount);
	DOREPLIFETIME_CONDITION(ACBMachine, MaxCharge, COND_InitialOnly);
	DOREPLIFETIME(ACBMachine, ChargeDrainSpeed);
	DOREPLIFETIME(ACBMachine, bIsPowered);
}

void ACBMachine::Timer_ApplyDepletion()
{
	ChargeAmount = FMath::Clamp(ChargeAmount-ChargeDrainSpeed, 0.f, MaxCharge);
	if(ChargeAmount <= 0.f)
	{
		bIsPowered = false;
	}
}

// Called every frame
void ACBMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

