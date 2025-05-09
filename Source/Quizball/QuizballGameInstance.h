// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "QuizballGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class QUIZBALL_API UQuizballGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UQuizballGameInstance();

	UFUNCTION(BlueprintCallable)
	void AddPlayer(FPlayerProperties player);
	
	UFUNCTION(BlueprintCallable)
	int32 GetPlayerIndexOnTurn();
	
	UFUNCTION(BlueprintCallable)
	void ChangeTurn();

	UFUNCTION(BlueprintCallable)
	void AddPoints(int32 points);
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player properties", meta = (AllowPrivateAccess = "true"))
	TArray<struct FPlayerProperties> m_PlayerProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player properties", meta = (AllowPrivateAccess = "true"))
	bool m_Player1Turn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player properties", meta = (AllowPrivateAccess = "true"))
	int32 m_PlayerIndex;
};
