#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CBCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class CRYOBABIES_API ACBCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category="Camera")
	class UCameraComponent* CameraComponent;

	//UPROPERTY(EditDefaultsOnly, Category = VOIP)
	//class UVOIPTalker* VOIPTalker;

public:
	ACBCharacter();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputMappingContext* DefaultInputMapping;

	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* LookAction;
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* MovementAction;
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* SprintAction;
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* CrouchAction;
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* JumpAction;
	UPROPERTY(EditDefaultsOnly, Category = Input)
	UInputAction* InteractAction;

	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float WalkSpeed = 400.0f;
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float RunSpeed = 600.0f;
	UPROPERTY(EditDefaultsOnly, Category = Movement)
	float CrouchSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = Camera)
	FVector CameraOffsetCrouching = FVector(0.0f, 0.0f, -75.0f);

	UPROPERTY(EditDefaultsOnly, Category = VOIP)
	USoundAttenuation* AttenuationSettings;
	UPROPERTY(EditDefaultsOnly, Category = VOIP)
	USoundEffectSourcePresetChain* SourceEffectChain;

private:
	// Input
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void ActivateSprint(const FInputActionValue& Value);
	void DeactivateSprint(const FInputActionValue& Value);
	void ActivateCrouch(const FInputActionValue& Value);
	void DeactivateCrouch(const FInputActionValue& Value);
	void ActivateInteract(const FInputActionValue& Value);
	void DeactivateInteract(const FInputActionValue& Value);

	// Voice Chat
	//void SetupVOIP();

public:
	UFUNCTION(Server, Reliable)
	void RequestSetupVOIPRPC(ACBCharacter* character);
	UFUNCTION(NetMulticast, Reliable)
	void SetupVOIPRPC(ACBCharacter* character);

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	USoundAttenuation* GetAttenuationSettings() const { return AttenuationSettings; }
	USoundEffectSourcePresetChain* GetSourceEffectChain() const { return SourceEffectChain; }

	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsSprinting = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsCrouching = false;
	UPROPERTY(BlueprintReadOnly, Replicated)
	bool bIsInteracting = false;
};
