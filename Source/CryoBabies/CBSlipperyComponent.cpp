// Fill out your copyright notice in the Description page of Project Settings.


#include "CBSlipperyComponent.h"

#include "CBCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"


// Sets default values for this component's properties
UCBSlipperyComponent::UCBSlipperyComponent()
{
	SphereOverlapComponent = CreateDefaultSubobject<USphereComponent>("OverlapSphere");
}


// Called when the game starts
void UCBSlipperyComponent::BeginPlay()
{
	Super::BeginPlay();

	SphereOverlapComponent->SetWorldLocation(GetOwner()->GetActorLocation());
	SphereOverlapComponent->SetupAttachment(GetOwner()->GetRootComponent());
	
	SphereOverlapComponent->OnComponentBeginOverlap.AddDynamic(this, &UCBSlipperyComponent::OnOverlapBegin);
}


void UCBSlipperyComponent::Recover(ACBCharacter* RagdolledCharacter)
{
	RagdolledCharacter->GetMesh()->SetSimulatePhysics(false);

	RagdolledCharacter->SetActorLocation(FVector(RagdolledCharacter->GetActorLocation().X,
		RagdolledCharacter->GetActorLocation().Y,
		(RagdolledCharacter->GetActorLocation().Z + RagdolledCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2)));
	RagdolledCharacter->GetCapsuleComponent()->SetCollisionProfileName("Pawn");
	RagdolledCharacter->GetMesh()->SetCollisionProfileName("CharacterMesh");

	//@TODO actually fix standing up

}

void UCBSlipperyComponent::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto localPlayer = Cast<ACBCharacter>(OtherActor);
	if(localPlayer)
	{
		//Storing locally in case simulating physics changes them
		auto playerVelocity = localPlayer->GetVelocity();
		
		localPlayer->GetMesh()->SetSimulatePhysics(true);
		
		localPlayer->GetMesh()->SetCollisionProfileName("Pawn");
		localPlayer->GetCapsuleComponent()->SetCollisionProfileName("NoCollision");

		localPlayer->GetMesh()->AddImpulse(FVector(playerVelocity.X, playerVelocity.Y, 2500.f), "Hips", true);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle_RagdollRecovery, FTimerDelegate::CreateUObject(this, &UCBSlipperyComponent::Recover, localPlayer), 3.f, false);

		//@TODO Drop held items?
	}
}

