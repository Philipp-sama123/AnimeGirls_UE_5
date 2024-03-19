// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Anime3rdPersonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AAnime3rdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AAnime3rdPersonCharacter();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Sprint(const FInputActionValue& Value);
	void Dodge(const FInputActionValue& Value);
	void AttackPrimary(const FInputActionValue& Value);
	void AttackSecondary(const FInputActionValue& Value);

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float DefaultMaxWalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float SprintMaxWalkSpeed = 500.f;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/**Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DefenseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CombatPrimaryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CombatSecondaryAction;

	/* ComboStuff */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo_Primary, meta = (AllowPrivateAccess = "true"))
	int32 CurrentComboStage_Primary = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo_Primary, meta = (AllowPrivateAccess = "true"))
	float ComboCooldownTime_Primary = 0.5f; // Time between combo inputs

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo_Primary, meta = (AllowPrivateAccess = "true"))
	float LastComboInputTime_Primary = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo_Secondary, meta = (AllowPrivateAccess = "true"))
	float ComboCooldownTime_Secondary = 0.5f; // Time between combo inputs

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo_Secondary, meta = (AllowPrivateAccess = "true"))
	float LastComboInputTime_Secondary = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo_Secondary, meta = (AllowPrivateAccess = "true"))
	int32 CurrentComboStage_Secondary = 0;


	/* AnimationMontages */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> PrimaryComboMontage_1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> PrimaryComboMontage_2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> PrimaryComboMontage_3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> PrimaryComboMontage_4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SecondaryComboMontage_1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SecondaryComboMontage_2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SecondaryComboMontage_3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> SecondaryComboMontage_4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DodgeForwardMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = AttackCombo, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimMontage> DodgeBackwardMontage;

	void PlayAnimationMontageFromSection(UAnimMontage* MontageToPlay, FName SectionName, float InPlayRate);
	void PrimaryAttackCombo();
	void SecondaryAttackCombo();
	void PlayMontage(TObjectPtr<UAnimMontage> Montage) const;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
