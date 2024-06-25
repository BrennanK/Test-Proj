// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Engine.h"
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MP_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TEST_API AMP_PlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void Server_SpawnPlayerCharacter();

	void Server_Interact_Implementation();

	UFUNCTION(Server, unreliable, WithValidation)
	void Server_Interact();

	bool Server_Interact_Validate();

};
