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

private:
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
