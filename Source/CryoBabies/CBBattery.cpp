// Fill out your copyright notice in the Description page of Project Settings.


#include "CBBattery.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ACBBattery::ACBBattery()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ChargePercentage = (ChargeAmount/MaxChargeAmount)*100.f;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
}

// Called when the game starts or when spawned
void ACBBattery::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACBBattery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ChargePercentage = (ChargeAmount/MaxChargeAmount)*100.f;
}

void ACBBattery::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACBBattery, ChargeAmount);
	DOREPLIFETIME_CONDITION(ACBBattery, MaxChargeAmount, COND_InitialOnly);
	DOREPLIFETIME(ACBBattery, bIsInserted);
}

void ACBBattery::StartCharge_Implementation(float deltaChargeSpeed)
{
	m_deltaChargeSpeed = deltaChargeSpeed;
	//Should we pause and unpause timer instead of setting/clearing? This may happen a lot
	GetWorldTimerManager().SetTimer(TimerHandle_Charge, this, &ACBBattery::ChangeCharge, 1.f, true);
}

void ACBBattery::ChangeCharge()
{
	ChargeAmount = FMath::Clamp(ChargeAmount+m_deltaChargeSpeed, 0.f, MaxChargeAmount);
}

void ACBBattery::EndCharge_Implementation()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Charge);
}

