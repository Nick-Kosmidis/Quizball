// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuizballQuestion.generated.h"

UENUM(BlueprintType)
enum class EQuestionCategory : uint8
{
	EQC_NONE UMETA(DisplayName = "None"),
	EQC_HISTORY UMETA(DisplayName = "History"),
	EQC_GEOGRAPHY UMETA(DisplayName = "Geography"),
	EQC_TOP5 UMETA(DisplayName = "Top5"),
	EQC_WHOS_MISSING UMETA(DisplayName = "WhosMissing"),
	EQC_PLAYERID UMETA(DisplayName = "PlayerID"),
	EQC_GOSSIP UMETA(DisplayName = "Gossip"),
	EQC_GUESS_THE_SCORE UMETA(DisplayName = "GuessTheScore"),
	EQC_MANAGERID UMETA(DisplayName = "ManagerID"),

	EQC_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class EQuestionDifficulty : uint8
{
	EQD_NONE UMETA(DisplayName = "None"),
	EQD_EASY UMETA(DisplayName = "Easy"),
	EQD_MEDIUM UMETA(DisplayName = "Medium"),
	EQD_HARD UMETA(DisplayName = "Hard"),

	EQD_MAX UMETA(DisplayName = "MAX")
};

UENUM(BlueprintType)
enum class EQuestionHelp : uint8
{
	EQH_NONE UMETA(DisplayName = "None"),
	EQH_50_50 UMETA(DisplayName = "50-50"),
	EQH_DOUBLE_POINTS UMETA(DisplayName = "Double Points"),
	EQH_TELEPHONE UMETA(DisplayName = "Telephone"),

	EQH_MAX UMETA(DisplayName = "MAX")
};

USTRUCT(BlueprintType)
struct FQuizballQuestionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Question;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQuestionCategory Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQuestionDifficulty Difficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InitialAnswer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Answers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Answer50_50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool isPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Tries;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EQuestionHelp Help;

	FQuizballQuestionData()
		:Question(TEXT("")), Category(EQuestionCategory::EQC_NONE), Difficulty(EQuestionDifficulty::EQD_NONE), InitialAnswer(TEXT("")), Answers(),
		Points(0), Answer50_50(TEXT("")), isPlayed(false), Tries(false), Help(EQuestionHelp::EQH_NONE)
	{

	}
};


UCLASS()
class QUIZBALL_API AQuizballQuestion : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AQuizballQuestion();

	UFUNCTION(BlueprintCallable)
	void LoadQuestion();

	UFUNCTION(BlueprintCallable)
	void SetCurrentQuestion(const EQuestionCategory& category, const EQuestionDifficulty& difficulty);

	UFUNCTION(BlueprintCallable)
	bool CheckAnswer(const FString& answer);

	UFUNCTION(BlueprintCallable)
	void DisableQuestion(const FQuizballQuestionData& currentQuestion);

	int32 SetMaxCharacters(const EQuestionCategory& category);
	char SetSeperateSymbol(const EQuestionCategory& category);
	void RemoveCharacter(FString& question, const char& seperateCharacter);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FString SeperateQuestionIntoLines(const FString& question, const int32 maxCharactersALine, const char& seperateSymbol);
	void RemoveSpacesFromStart(TArray<FString>& answers);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Question", meta = (AllowPrivateAccess = "true"))
	TArray<FQuizballQuestionData> QuizballQuestions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Question", meta = (AllowPrivateAccess = "true"))
	FQuizballQuestionData CurrentQuestion;
};
