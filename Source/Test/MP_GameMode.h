// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MP_GameMode.generated.h"

/**
 * 
 */
UCLASS()
class TEST_API AMP_GameMode : public AGameMode
{
	GENERATED_BODY()
	
	TArray <APlayerController*> AllPlayerControllers;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

public:

	UFUNCTION()
	FTransform FindRandomPlayerStart(APlayerController* PC);

	UFUNCTION()
	void SpawnPlayer(APlayerController* PC);
};
