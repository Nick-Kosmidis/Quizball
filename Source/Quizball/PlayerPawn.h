// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"


USTRUCT(BlueprintType)
struct FPlayerProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Use50_50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool UseDoublePoints;

	FPlayerProperties()
		:Name(TEXT("")), Points(0), Use50_50(false), UseDoublePoints(false)
	{

	}
};
UCLASS()
class QUIZBALL_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();
	
	UFUNCTION(BlueprintCallable)
	void AddPoints(FPlayerProperties player, const int& points);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	APlayerController* GetPlayerController() const;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Properties", meta = (AllowPrivateAccess = "true"))
	FPlayerProperties playerProperties;
};
