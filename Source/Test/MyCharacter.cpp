// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}


// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	GetCharacterMovement()->MaxWalkSpeed = minSpeed;
}

void AMyCharacter::Move(const FInputActionValue& Value)
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
		//LaunchCharacter(ForwardDirection, false, false);
		//LaunchCharacter(RightDirection, false, false);
		float calculatedDot = FMath::Abs(dotProdOfCameraFwdVsPlayerFwd());

		if (MovementVector.X < 0)
		{
			calculatedDot = 1 - calculatedDot;
			calculatedDot *= -1;
			//changeBoardDot(calculatedDot);
		}
		else
		{
			//changeBoardDot(1-calculatedDot);
		}

		GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Cyan, "Dot is: " + FString::SanitizeFloat(calculatedDot), true, FVector2D(2.0f, 2.0f));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, "Move fail", true, FVector2D(2.0f, 2.0f));
	}
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, "Look fail", true, FVector2D(2.0f, 2.0f));
	}
}

void AMyCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	changePlayerBoardState(EPlayerRideState::PS_Landed);
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AddMovementInput(GetActorForwardVector() *.1);
	GEngine->AddOnScreenDebugMessage(0, 10.0f, FColor::Cyan, "SpeedLerpAlpha is: " + FString::SanitizeFloat(speedLerpAlpha), true, FVector2D(2.0f, 2.0f));
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, "Called SetupInput inside binding lines", true, FVector2D(2.0f, 2.0f));
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMyCharacter::initiateJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);

		//Debug Mecjanic used to test mechanic before solidifying it as kept
		EnhancedInputComponent->BindAction(DebugAction, ETriggerEvent::Started, this, &AMyCharacter::debugMechanic);

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, "Set up PLAYER Input fail", true, FVector2D(2.0f, 2.0f));
	}

}

void AMyCharacter::initiateJump()
{
	if (GetCharacterMovement()->IsMovingOnGround())
	{
		GetCharacterMovement()->bNotifyApex = true;
		changePlayerBoardState(EPlayerRideState::PS_Starting_Jump);
	}
	
	
}

float AMyCharacter::dotProdOfCameraFwdVsPlayerFwd()
{
	FVector playerForward = GetActorForwardVector();
	FVector cameraForward = FollowCamera->GetForwardVector();
	return UKismetMathLibrary::Dot_VectorVector(FVector(playerForward.X, playerForward.Y,0),FVector(cameraForward.X, cameraForward.Y, 0));
}

void AMyCharacter::debugMechanic()
{
	
	if (GetCharacterMovement()->MaxWalkSpeed == minSpeed)
	{
		//GetCharacterMovement()->MaxWalkSpeed = 1000.0f;
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, "Speed Up called", true, FVector2D(2.0f, 2.0f));
		speedLerpAlpha = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(speedTransitionHandle);
		GetWorld()->GetTimerManager().SetTimer(speedTransitionHandle, this, &AMyCharacter::speedUp, GetWorld()->GetDeltaSeconds(), true);
	}
	else
	{
		//GetCharacterMovement()->MaxWalkSpeed = 500.0f;
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, "Slow Down called", true, FVector2D(2.0f, 2.0f));
		speedLerpAlpha = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(speedTransitionHandle);
		GetWorld()->GetTimerManager().SetTimer(speedTransitionHandle, this, &AMyCharacter::slowDown, GetWorld()->GetDeltaSeconds(), true);
	}
}

void AMyCharacter::speedUp()
{
	speedLerpAlpha += GetWorld()->GetDeltaSeconds();
	//speedLerpAFMath::Clamp(speedLerpAlpha,0.0f,1.0f);

	GetCharacterMovement()->MaxWalkSpeed= FMath::Lerp(minSpeed, maxSpeed, speedLerpAlpha);

	if (speedLerpAlpha >= 1.0f)
	{
		speedLerpAlpha = 0.0f;
		GetCharacterMovement()->MaxWalkSpeed = maxSpeed;
		GetWorld()->GetTimerManager().ClearTimer(speedTransitionHandle);
	}
}

void AMyCharacter::slowDown()
{
	speedLerpAlpha += GetWorld()->GetDeltaSeconds();
	//FMath::Clamp(speedLerpAlpha, 0.0f, 1.0f);

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(maxSpeed, minSpeed, speedLerpAlpha);

	if (speedLerpAlpha >= 1.0f)
	{
		speedLerpAlpha = 0.0f;
		GetCharacterMovement()->MaxWalkSpeed = minSpeed;
		GetWorld()->GetTimerManager().ClearTimer(speedTransitionHandle);
	}
}


