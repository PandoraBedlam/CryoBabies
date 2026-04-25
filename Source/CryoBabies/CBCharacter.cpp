#include "CBCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "IInteractable.h"
#include "Components/SphereComponent.h"
#include "Net/VoiceConfig.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"

ACBCharacter::ACBCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetMesh());
	CameraComponent->bUsePawnControlRotation = true;
	//CameraComponent->AddRelativeLocation(FVector(0.0f, 0.0f, -CameraHeightCrouching));

	OverlapSphereComponent = CreateDefaultSubobject<USphereComponent>("SphereOverlapComp");
	OverlapSphereComponent->SetupAttachment(RootComponent);
	OverlapSphereComponent->SetCollisionProfileName("InteractorSphere");
	OverlapSphereComponent->SetGenerateOverlapEvents(true);

	//VOIPTalker = CreateDefaultSubobject<UVOIPTalker>(TEXT("VOIPTalker"));
	//VOIPTalker->SetIsReplicated(true);
}

void ACBCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//if (GEngine)
	//{
	//	if (VOIPTalker)
	//	{
	//		float VoiceLevel = VOIPTalker->GetVoiceLevel();
	//		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, FString::Printf(TEXT("Voice level: %f"), VoiceLevel));
	//	}
	//}
}

void ACBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACBCharacter::Look);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ACBCharacter::Move);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ACBCharacter::ActivateSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ACBCharacter::DeactivateSprint);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ACBCharacter::ActivateCrouch);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &ACBCharacter::DeactivateCrouch);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACBCharacter::Jump);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &ACBCharacter::ActivateInteract);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ACBCharacter::DeactivateInteract);
		EnhancedInputComponent->BindAction(DropAction, ETriggerEvent::Triggered, this, &ACBCharacter::DropPickup);
	}
}

void ACBCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACBCharacter, bIsCrouching);
	DOREPLIFETIME(ACBCharacter, bIsInteracting);
}

void ACBCharacter::TryPickUp(AActor* Pickup, bool bForce)
{
	// Check current pickup
	if (m_HeldPickup)
	{
		if (bForce) DropPickupRPC();
		else return;
	}

	PickUpRPC(Pickup);
}

void ACBCharacter::PickUpRPC_Implementation(AActor* Pickup)
{
	m_HeldPickup = Pickup;

	// Disable physics on the pickup
	m_HeldPickup->SetActorEnableCollision(false);
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(m_HeldPickup->GetRootComponent()))
	{
		RootPrimitive->SetSimulatePhysics(false);
	}

	// Attach pickup
	bool bAttached = m_HeldPickup->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("PickupSocket"));
}

void ACBCharacter::DropPickup()
{
	DropPickupRPC();
}

void ACBCharacter::DropPickupRPC_Implementation()
{
	if (!m_HeldPickup) return;

	// Enable physics on the pickup
	m_HeldPickup->SetActorEnableCollision(true);
	if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(m_HeldPickup->GetRootComponent()))
	{
		RootPrimitive->SetSimulatePhysics(true);
	}

	// Detach pickup
	m_HeldPickup->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	m_HeldPickup = nullptr;
}

void ACBCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Input
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultInputMapping, 0);
		}
	}

	// VOIP
	//if (GetPlayerState() /* && IsLocallyControlled()*/) SetupVOIP();

	if (GetPlayerState() && IsLocallyControlled()) RequestSetupVOIPRPC(this);

	OverlapSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ACBCharacter::OnOverlapBegin);
	OverlapSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ACBCharacter::OnOverlapEnd);
}

void ACBCharacter::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this
		&& OtherActor->GetClass()->ImplementsInterface(UIInteractable::StaticClass()))
	{
		m_OverlappedInteractable = OtherActor;
	}
}

void ACBCharacter::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor,
	class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == m_OverlappedInteractable)
	{
		m_OverlappedInteractable = nullptr;
	}
}

void ACBCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(LookAxisVector.Y);
	AddControllerYawInput(LookAxisVector.X);
}

void ACBCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FVector Forward = GetActorForwardVector();
	AddMovementInput(Forward, MovementVector.Y);
	const FVector Right = GetActorRightVector();
	AddMovementInput(Right, MovementVector.X);
}

void ACBCharacter::ActivateSprint(const FInputActionValue& Value)
{
	UnCrouch(true);
	bIsCrouching = false;

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
}

void ACBCharacter::DeactivateSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ACBCharacter::ActivateCrouch(const FInputActionValue& Value)
{
	bIsSprinting = false;

	Crouch(true);
	bIsCrouching = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;

	CameraComponent->AddRelativeLocation(CameraOffsetCrouching);
}

void ACBCharacter::DeactivateCrouch(const FInputActionValue& Value)
{
	UnCrouch(true);
	bIsCrouching = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CameraComponent->AddRelativeLocation(-CameraOffsetCrouching);
}

void ACBCharacter::ActivateInteract(const FInputActionValue& Value)
{
	bIsInteracting = true;

	if(HasAuthority())
	{
		if (m_OverlappedInteractable
			&& m_OverlappedInteractable->GetClass()->ImplementsInterface(UIInteractable::StaticClass()))
		{
			IIInteractable::Execute_Interact(m_OverlappedInteractable, this, true);
		}
	}
	else
	{
		ServerInteractButtonPressed();
	}
}

void ACBCharacter::DeactivateInteract(const FInputActionValue& Value)
{
	bIsInteracting = false;

	if(HasAuthority())
	{
		if (m_OverlappedInteractable
			&& m_OverlappedInteractable->GetClass()->ImplementsInterface(UIInteractable::StaticClass()))
		{
			IIInteractable::Execute_Interact(m_OverlappedInteractable, this, false);
		}
	}
	else
	{
		ServerInteractButtonReleased();
	}
}

void ACBCharacter::ServerInteractButtonPressed_Implementation()
{
	if (m_OverlappedInteractable
		&& m_OverlappedInteractable->GetClass()->ImplementsInterface(UIInteractable::StaticClass()))
	{
		IIInteractable::Execute_Interact(m_OverlappedInteractable, this, true);
	}
}

void ACBCharacter::ServerInteractButtonReleased_Implementation()
{
	if (m_OverlappedInteractable
			&& m_OverlappedInteractable->GetClass()->ImplementsInterface(UIInteractable::StaticClass()))
	{
		IIInteractable::Execute_Interact(m_OverlappedInteractable, this, false);
	}
}

void ACBCharacter::RequestSetupVOIPRPC_Implementation(ACBCharacter* character)
{
	SetupVOIPRPC(character);
}

void ACBCharacter::SetupVOIPRPC_Implementation(ACBCharacter* character)
{
	APlayerState* player = character->GetPlayerState();

	UVOIPTalker* VOIPTalker = UVOIPTalker::CreateTalkerForPlayer(player);

	if (VOIPTalker)
	{
		VOIPTalker->RegisterWithPlayerState(player);

		VOIPTalker->Settings.ComponentToAttachTo = character->GetRootComponent();

		USoundAttenuation* ClientAttenuationSettings = character->GetAttenuationSettings();
		if (ClientAttenuationSettings)
		{
			VOIPTalker->Settings.AttenuationSettings = ClientAttenuationSettings;
		}

		USoundEffectSourcePresetChain* ClientSourceEffectChain = character->GetSourceEffectChain();
		if (ClientSourceEffectChain)
		{
			VOIPTalker->Settings.SourceEffectChain = ClientSourceEffectChain;
		}
	}
}

//void ACBCharacter::SetupVOIP()
//{
//	APlayerState* LocalPlayerState = GetPlayerState();
//
//	//VOIPTalker = UVOIPTalker::CreateTalkerForPlayer(LocalPlayerState);
//
//	if (VOIPTalker)
//	{
//		VOIPTalker->RegisterWithPlayerState(LocalPlayerState);
//	}
//
//	FVoiceSettings VoiceSettings;
//
//	if (AttenuationSettings) VoiceSettings.AttenuationSettings = AttenuationSettings;
//	if (SourceEffectChain) VoiceSettings.SourceEffectChain = SourceEffectChain;
//	VoiceSettings.ComponentToAttachTo = RootComponent;
//
//	VOIPTalker->Settings = VoiceSettings;
//}

//void ACBCharacter::SetupVOIP()
//{
//	APlayerState* LocalPlayerState = GetPlayerState();
//
//	VOIPTalker = UVOIPTalker::CreateTalkerForPlayer(LocalPlayerState);
//
//	if (VOIPTalker)
//	{
//		if (AttenuationSettings) VOIPTalker->Settings.AttenuationSettings = AttenuationSettings;
//		if (SourceEffectChain) VOIPTalker->Settings.SourceEffectChain = SourceEffectChain;
//		VOIPTalker->Settings.ComponentToAttachTo = RootComponent;
//
//		VOIPTalker->RegisterWithPlayerState(LocalPlayerState);
//	}
//}