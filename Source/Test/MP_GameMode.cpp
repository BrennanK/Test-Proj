// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/KismetMathLibrary.h"
#include "TestCharacter.h"

void AMP_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	AllPlayerControllers.Add(NewPlayer);
}

void AMP_GameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerController* ExitingPlayer = Cast<APlayerController>(Exiting);

	AllPlayerControllers.Remove(ExitingPlayer);
}

void AMP_GameMode::SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC)
{
	Super::SwapPlayerControllers(OldPC, NewPC);

	AllPlayerControllers.Add(NewPC);
}

FTransform AMP_GameMode::FindRandomPlayerStart(APlayerController* PC)
{
	TArray <AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(PC->GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	int randomIndex = UKismetMathLibrary::RandomIntegerInRange(0, PlayerStarts.Num() - 1);

	return PlayerStarts[randomIndex]->GetActorTransform();
}

void AMP_GameMode::SpawnPlayer(APlayerController* PC)
{
	if (IsValid(PC->Super::GetPawn()))
	{
		PC->Super::GetPawn()->Destroy();
	}

	FActorSpawnParameters spawnInfo;

	
	if (PC->GetLocalRole() == ROLE_Authority)
	{
		FString length = UEnum::GetValueAsString(PC->GetLocalRole());

		GEngine->AddOnScreenDebugMessage(-1, 14.0f, FColor::Green, length);
	}
	else
	{
		FString role = UEnum::GetValueAsString(PC->GetLocalRole());
		//GEngine->AddOnScreenDebugMessage(-1, 14.0f, FColor::Red, role);
	}
	

	TArray <AActor*> PlayerStarts;
	if (PC->GetWorld() != nullptr && (PC->GetLocalRole()==ROLE_Authority || PC->GetLocalRole() == ROLE_AutonomousProxy))
	{
		UGameplayStatics::GetAllActorsOfClass(PC->GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

		FString numPlayerStarts = "Number of Player Starts: ";
		numPlayerStarts.AppendInt(PlayerStarts.Num());

		GEngine->AddOnScreenDebugMessage(-1, 14.0f, FColor::Red, numPlayerStarts);
	}
	

	//ATestCharacter* temp = GetWorld()->SpawnActor<ATestCharacter>(ATestCharacter::StaticClass(), FindRandomPlayerStart(), spawnInfo);

	//GEngine->AddOnScreenDebugMessage(-1, 14.0f, FColor::Red, FindRandomPlayerStart().ToString());

	if (PC->GetLocalRole() == ROLE_Authority)
	{
	    if (PC != nullptr)
		{
			
			UWorld* world = PC->GetWorld();
			ATestCharacter* temp = world->SpawnActor<ATestCharacter>(ATestCharacter::StaticClass(), FindRandomPlayerStart(PC), spawnInfo);
			PC->Possess(temp);
			temp->overridePossesion(PC);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 14.0f, FColor::Red, "No controller found for GAME MODE");
		}
		
	}

	
}




