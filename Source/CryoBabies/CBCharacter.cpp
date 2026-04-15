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
	//if (GetPlayerState() /* && IsLocallyControlled()*/) SetupVOIP();

	if (GetPlayerState() && IsLocallyControlled()) RequestSetupVOIPRPC(this);
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