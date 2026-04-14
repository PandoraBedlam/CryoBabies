#include "CBCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/VoiceConfig.h"

ACBCharacter::ACBCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(GetMesh());
	CameraComponent->bUsePawnControlRotation = true;

	VOIPTalker = CreateDefaultSubobject<UVOIPTalker>(TEXT("VOIPTalker"));
	VOIPTalker->SetIsReplicated(true);
}

void ACBCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACBCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACBCharacter::Look);
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ACBCharacter::Move);
	}
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
	if (GetPlayerState() && IsLocallyControlled()) SetupVOIP();
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

void ACBCharacter::SetupVOIP()
{
	APlayerState* LocalPlayerState = GetPlayerState();

	//UVOIPTalker* VOIPTalker = UVOIPTalker::CreateTalkerForPlayer(LocalPlayerState);

	if (VOIPTalker)
	{
		VOIPTalker->RegisterWithPlayerState(LocalPlayerState);
	}

	FVoiceSettings VoiceSettings;

	if (AttenuationSettings) VoiceSettings.AttenuationSettings = AttenuationSettings;
	if (SourceEffectChain) VoiceSettings.SourceEffectChain = SourceEffectChain;
	VoiceSettings.ComponentToAttachTo = CameraComponent;

	VOIPTalker->Settings = VoiceSettings;
}