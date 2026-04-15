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

	UPROPERTY(EditDefaultsOnly, Category = VOIP)
	USoundAttenuation* AttenuationSettings;
	UPROPERTY(EditDefaultsOnly, Category = VOIP)
	USoundEffectSourcePresetChain* SourceEffectChain;

private:
	// Input
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);

	// Voice Chat
	//void SetupVOIP();

public:
	UFUNCTION(Server, Reliable)
	void RequestSetupVOIPRPC(ACBCharacter* character);
	UFUNCTION(NetMulticast, Reliable)
	void SetupVOIPRPC(ACBCharacter* character);

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	USoundAttenuation* GetAttenuationSettings() const { return AttenuationSettings; }
	USoundEffectSourcePresetChain* GetSourceEffectChain() const { return SourceEffectChain; }
};
