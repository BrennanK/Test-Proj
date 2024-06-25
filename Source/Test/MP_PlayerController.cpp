// Fill out your copyright notice in the Description page of Project Settings.


#include "MP_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "MP_GameMode.h"
void AMP_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		Server_SpawnPlayerCharacter();
	}
}

void AMP_PlayerController::Server_SpawnPlayerCharacter()
{
	AMP_GameMode* gm = Cast<AMP_GameMode>(UGameplayStatics::GetGameMode(GetWorld()));

	gm->SpawnPlayer(this);
}

void AMP_PlayerController::Server_Interact_Implementation()
{
	// Interact with a door or so!
}



bool AMP_PlayerController::Server_Interact_Validate()
{
	return true;
}



