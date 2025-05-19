// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerPawn.generated.h"

UENUM(BlueprintType)
enum class EPlayerLocation : uint8
{
	EPL_NONE UMETA(DisplayName = "None"),
	EPL_MAIN_MENU UMETA(DisplayName = "Main Menu"),
	EPL_IN_QUESTION UMETA(DisplayName = "In question"),

	EPL_MAX UMETA(DisplayName = "Max")
};

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPlayerLocation PlayerLocation;

	FPlayerProperties()
		:Name(TEXT("")), Points(0), Use50_50(false), UseDoublePoints(false), PlayerLocation(EPlayerLocation::EPL_MAIN_MENU)
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
