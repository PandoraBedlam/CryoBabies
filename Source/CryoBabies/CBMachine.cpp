// Fill out your copyright notice in the Description page of Project Settings.


#include "CBMachine.h"

#include "Net/UnrealNetwork.h"
#include "CBBattery.h"


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

void ACBMachine::AddBattery(ACBBattery* Battery, bool bForce)
{
	if (InsertedBatteries.Num() >= MaxBatteryCount)
	{
		if (bForce) RemoveBatteryRPC();
		else return;
	}

	AddBatteryRPC(Battery);
}

void ACBMachine::AddBatteryRPC_Implementation(ACBBattery* Battery)
{
	USkeletalMeshComponent* Mesh = GetComponentByClass<USkeletalMeshComponent>();

	if(!Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Mesh component not found on machine"));
		return;
	}

	FName NextSocketName = FName(*FString::Printf(TEXT("BatterySocket%d"), InsertedBatteries.Num()));

	if (!Mesh->DoesSocketExist(NextSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("Socket 'BatterySocket%d' does not exist on mesh"), InsertedBatteries.Num());
		return;
	}

	InsertedBatteries.Add(Battery);

	// Disable physics on the pickup
	Battery->SetActorEnableCollision(false);
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Battery->GetRootComponent()))
	{
		RootPrimitive->SetSimulatePhysics(false);
	}

	// Attach pickup
	bool bAttached = Battery->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NextSocketName);
}

void ACBMachine::RemoveBattery()
{
	RemoveBatteryRPC();
}

void ACBMachine::RemoveBatteryRPC_Implementation()
{
	if (InsertedBatteries.Num() <= 0) return;

	int LastIndex = InsertedBatteries.Num() - 1;
	ACBBattery* BatteryToRemove = InsertedBatteries[LastIndex];

	// Enable physics on the pickup
	BatteryToRemove->SetActorEnableCollision(true);
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(BatteryToRemove->GetRootComponent()))
	{
		RootPrimitive->SetSimulatePhysics(true);
	}

	// Detach pickup
	BatteryToRemove->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	InsertedBatteries.RemoveAt(LastIndex);
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

