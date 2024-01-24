// Fill out your copyright notice in the Description page of Project Settings.


#include "Mole_Character.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
// Sets default values
AMole_Character::AMole_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
void AMole_Character::BeginPlay()
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
}

void AMole_Character::Move(const FInputActionValue& Value)
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

void AMole_Character::Look(const FInputActionValue& Value)
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

// Called every frame
void AMole_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector forwardLineEnd = GetActorLocation() + (GetActorForwardVector() * lineLength);

	DrawDebugLine(GetWorld(), GetActorLocation(), forwardLineEnd, FColor::Red);

	FHitResult wallDetectHit;
	FCollisionQueryParams queryParams;
	//queryParams.AddIgnoredActor(this);

	//FVector StartPoint = FollowCamera->GetComponentLocation();
	//FVector CameraForwardVector = FollowCamera->GetForwardVector();
	//FVector EndPoint = StartPoint + (CameraForwardVector * shotLockTraceDistance);

	GetWorld()->LineTraceSingleByChannel(wallDetectHit, GetActorLocation(), forwardLineEnd, ECollisionChannel::ECC_Visibility);

	if (wallDetectHit.bBlockingHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Cyan, "We hit something!!", true);

		FVector normalLineEnd = GetActorLocation() + (wallDetectHit.Normal*lineLength);
		
		DrawDebugLine(GetWorld(), GetActorLocation(), normalLineEnd, FColor::Blue);
		
		FVector forwardDebugVector = forwardLineEnd - GetActorLocation();
		FVector normalDebugVector = normalLineEnd - GetActorLocation();

		FVector firstCross=FVector::CrossProduct(normalDebugVector, forwardDebugVector);
		firstCross.Z = FMath::Abs(firstCross.Z);
		FVector firstCrossEnd = GetActorLocation() + firstCross;

		DrawDebugLine(GetWorld(), GetActorLocation(), firstCrossEnd, FColor::Green);

		float newPitch=calculateAngBetweenVectors(GetActorUpVector(), normalDebugVector);

		GetMesh()->SetWorldRotation(FRotator(newPitch, GetActorRotation().Yaw,0));
		//SetActorRotation(FRotator(newPitch, 0, 0));
	}
	else
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			GetMesh()->SetWorldRotation(FRotator(0, GetActorRotation().Yaw, 0));
		}
		
	}
}

// Called to bind functionality to input
void AMole_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AMole_Character::moleJummp);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMole_Character::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMole_Character::Look);

	}

}

void AMole_Character::moleJummp()
{
	FVector displacement = FVector(0, 0, GetCharacterMovement()->JumpZVelocity);
	FVector accelrationDueToGravity = FVector(0, 0, GetCharacterMovement()->GetGravityZ());

	FVector initialJumpVelocity = (displacement - .5 * accelrationDueToGravity * FMath::Square(jumpTime)) / jumpTime;

	LaunchCharacter(initialJumpVelocity, false, false);

	GetWorld()->GetTimerManager().SetTimer(jumpRotHandle,this, &AMole_Character::rotateMoleMesh,GetWorld()->GetDeltaSeconds(),true,0);
}

void AMole_Character::rotateMoleMesh()
{
	rotAmount += GetWorld()->GetDeltaSeconds()*2;
	rotAmount = FMath::Clamp(rotAmount, 0, 1);
	FRotator meshRotation=FMath::Lerp(FRotator(0, GetControlRotation().Yaw, 0),FRotator(180, GetActorRotation().Yaw,0), rotAmount);
	GetMesh()->SetWorldRotation(meshRotation);
	if (rotAmount == 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(jumpRotHandle);
		rotAmount = 0;
		GetCharacterMovement()->GravityScale = 2.0f;
	}
}

void AMole_Character::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	GetWorld()->GetTimerManager().ClearTimer(jumpRotHandle);
	rotAmount = 0;
	GetMesh()->SetWorldRotation(FRotator(0,GetActorRotation().Yaw,0));
	GetCharacterMovement()->GravityScale = 1.0f;

}

float AMole_Character::calculateAngBetweenVectors(FVector firstVector, FVector secondVector)
{
	
	float vectoralDot = FVector::DotProduct(firstVector, secondVector);
	
	float firstMag = firstVector.Length();
	float secondMag = secondVector.Length();

	float multipliedMag = firstMag * secondMag;

	float angle = FMath::RadiansToDegrees(FMath::Acos(vectoralDot / multipliedMag));

	GEngine->AddOnScreenDebugMessage(0, 1.0f, FColor::Cyan, FString::SanitizeFloat(angle), true, FVector2D(2.0f, 2.0f));

	return angle;

	
}

