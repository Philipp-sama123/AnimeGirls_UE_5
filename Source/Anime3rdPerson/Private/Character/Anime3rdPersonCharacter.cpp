// Copyright Epic Games, Inc. All Rights Reserved.

#include "Anime3rdPerson/Public/Character/Anime3rdPersonCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AAnime3rdPersonCharacter

AAnime3rdPersonCharacter::AAnime3rdPersonCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AAnime3rdPersonCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AAnime3rdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAnime3rdPersonCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAnime3rdPersonCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAnime3rdPersonCharacter::Look);
		EnhancedInputComponent->BindAction(CombatPrimaryAction, ETriggerEvent::Triggered, this, &AAnime3rdPersonCharacter::AttackPrimary);
		EnhancedInputComponent->BindAction(CombatSecondaryAction, ETriggerEvent::Triggered, this,
		                                   &AAnime3rdPersonCharacter::AttackSecondary);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AAnime3rdPersonCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AAnime3rdPersonCharacter::Sprint);

		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &AAnime3rdPersonCharacter::Dodge);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT(
			       "'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
		       ), *GetNameSafe(this));
	}
}

void AAnime3rdPersonCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AAnime3rdPersonCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AAnime3rdPersonCharacter::Sprint(const FInputActionValue& Value)
{
	const bool bIsSprinting = Value.Get<bool>();
	GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintMaxWalkSpeed : DefaultMaxWalkSpeed;
}

void AAnime3rdPersonCharacter::Dodge(const FInputActionValue& Value)
{
	// Check if the DodgeMontage is valid and the character is not already dodging
	if (DodgeForwardMontage && DodgeBackwardMontage)
	{
		// Calculate dot product between character's forward vector and velocity vector
		FVector Velocity = GetCharacterMovement()->Velocity;
		FVector ForwardVector = GetActorForwardVector();
		float DotProduct = FVector::DotProduct(Velocity.GetSafeNormal(), ForwardVector);

		// Select the DodgeMontage based on the dot product
		UAnimMontage* SelectedMontage = (DotProduct > 0) ? DodgeForwardMontage : DodgeBackwardMontage;

		// Play the selected dodge montage
		if (SelectedMontage)
		{
			PlayMontage(SelectedMontage);
		}
	}
}

void AAnime3rdPersonCharacter::AttackPrimary(const FInputActionValue& Value)
{
	PrimaryAttackCombo();
}

void AAnime3rdPersonCharacter::AttackSecondary(const FInputActionValue& Value)
{
	SecondaryAttackCombo();
}

void AAnime3rdPersonCharacter::PlayAnimationMontageFromSection(UAnimMontage* MontageToPlay, FName SectionName, float InPlayRate)
{
	if (MontageToPlay && GetMesh() && GetMesh()->GetAnimInstance())
	{
		// Jump to the specified section and play the montage
		GetMesh()->GetAnimInstance()->Montage_JumpToSection(SectionName, MontageToPlay);
		GetMesh()->GetAnimInstance()->Montage_Play(MontageToPlay, InPlayRate);
	}
}

void AAnime3rdPersonCharacter::PrimaryAttackCombo()
{
	// Check if enough time has passed since last combo input
	if (GetWorld()->GetTimeSeconds() - LastComboInputTime_Primary > ComboCooldownTime_Primary)
	{
		// Reset combo if it's been too long since last input
		if (GetWorld()->GetTimeSeconds() - LastComboInputTime_Primary > ComboCooldownTime_Primary * 2)
		{
			CurrentComboStage_Primary = 0;
		}

		// Execute the current combo stage
		switch (CurrentComboStage_Primary)
		{
		case 0:
			PlayMontage(PrimaryComboMontage_1);
			break;
		case 1:
			PlayMontage(PrimaryComboMontage_2);
			break;
		case 2:
			PlayMontage(PrimaryComboMontage_3);
			break;
		case 3:
			PlayMontage(PrimaryComboMontage_4);
			break;
		default:
			break;
		}

		// Increment combo stage and update last input time
		CurrentComboStage_Primary = (CurrentComboStage_Primary + 1) % 4;
		LastComboInputTime_Primary = GetWorld()->GetTimeSeconds();
	}
}

void AAnime3rdPersonCharacter::SecondaryAttackCombo()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();

	// Check if enough time has passed since last combo input
	if (CurrentTime - LastComboInputTime_Secondary > ComboCooldownTime_Secondary)
	{
		// Reset combo if it's been too long since last input
		if (CurrentTime - LastComboInputTime_Secondary > ComboCooldownTime_Secondary * 2)
		{
			CurrentComboStage_Secondary = 0;
		}

		// Execute the current combo stage directly without using switch-case
		if (CurrentComboStage_Secondary >= 0 && CurrentComboStage_Secondary < 4)
		{
			// Use an array of montages to avoid repetitive code
			TArray<TObjectPtr<UAnimMontage>> ComboMontages;
			ComboMontages.Add(SecondaryComboMontage_1);
			ComboMontages.Add(SecondaryComboMontage_2);
			ComboMontages.Add(SecondaryComboMontage_3);
			ComboMontages.Add(SecondaryComboMontage_4);

			// Play the montage for the current combo stage
			PlayMontage(ComboMontages[CurrentComboStage_Secondary]);

			// Increment combo stage and update last input time
			CurrentComboStage_Secondary = (CurrentComboStage_Secondary + 1) % 4;
			LastComboInputTime_Secondary = CurrentTime;
		}
	}
}

void AAnime3rdPersonCharacter::PlayMontage(TObjectPtr<UAnimMontage> Montage) const
{
	if (GetMesh() && GetMesh()->GetAnimInstance() && Montage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage);
	}
}
