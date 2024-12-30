// Fill out your copyright notice in the Description page of Project Settings.


#include "QuizballQuestion.h"
#include "Misc/FileHelper.h"
#include "Engine.h"

// Sets default values
AQuizballQuestion::AQuizballQuestion()
	:CurrentQuestion(FQuizballQuestionData())
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AQuizballQuestion::LoadQuestion()
{
	FString filePath = FPaths::ProjectConfigDir();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("File path: %s"), *filePath));
	}

	filePath.Append(TEXT("QuizballQuestion.txt"));

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> fileLines;

	if (FileManager.FileExists(*filePath))
	{
		if (FFileHelper::LoadFileToStringArray(fileLines, *filePath))
		{
			for (const FString& line : fileLines)
			{
				TArray<FString> parsedData;
				line.ParseIntoArray(parsedData, TEXT(","), true);

				if (parsedData.Num() == 6)
				{
					FQuizballQuestionData newQuestion;
					newQuestion.Category = (EQuestionCategory)FCString::Atoi(*parsedData[1]);
					newQuestion.Difficulty = (EQuestionDifficulty)FCString::Atoi(*parsedData[2]);
					newQuestion.InitialAnswer = parsedData[3];
					if (newQuestion.Category == EQuestionCategory::EQC_TOP5)
					{
						newQuestion.InitialAnswer = parsedData[3];
						newQuestion.InitialAnswer.ParseIntoArray(newQuestion.Answers, TEXT("-"), true);
						newQuestion.Tries = true;
					}
					else
					{
						newQuestion.Answers.Add(parsedData[3]);
						newQuestion.Tries = false;
					}
					newQuestion.Points = (int32)FCString::Atoi(*parsedData[4]); /*Difficulty enums are between 1-3, 1:Easy, 2:Medium, 3:Hard*/
					newQuestion.Answer50_50 = parsedData[5];
					newQuestion.isPlayed = false;


					int32 maxCharacters = SetMaxCharacters(newQuestion.Category);
					char seperateSymbol = SetSeperateSymbol(newQuestion.Category);
					newQuestion.Question = SeperateQuestionIntoLines(parsedData[0], maxCharacters, seperateSymbol);
					RemoveSpacesFromStart(newQuestion.Answers);

					QuizballQuestions.Add(newQuestion);
				}
				else
				{
					if (GEngine)
						GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Wrong question format.There are %d, Question: %s"), 
							parsedData.Num(), *parsedData[0]));
				}
			}
		}
		else
		{
			if (GEngine)
				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, FString::Printf(TEXT("Fail to load the questions")));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, FString::Printf(TEXT("File doesn't exists")));
	}
}

void AQuizballQuestion::SetCurrentQuestion(const EQuestionCategory& category, const EQuestionDifficulty& difficulty)
{
	for (const FQuizballQuestionData& question : QuizballQuestions)
	{
		if (question.Category == category && question.Difficulty == difficulty && !question.isPlayed)
		{
			CurrentQuestion = question;
			break;
		}
	}
}

bool AQuizballQuestion::CheckAnswer(const FString& answer)
{
	if (answer.IsEmpty())
	{
		return false;
	}

	for (const FString& correctAnswer : CurrentQuestion.Answers)
	{
		if (correctAnswer.Contains(answer) || answer.Contains(correctAnswer) && answer.Len() > correctAnswer.Len() / 2)
		{
			return true;
		}
	}

	// If no matches are found, return false
	return false;
}

void AQuizballQuestion::DisableQuestion(const FQuizballQuestionData& currentQuestion)
{
	for (auto& question : QuizballQuestions)
	{
		if (question.Question == currentQuestion.Question)
		{
			question.isPlayed = true;
		}
	}
}

int32 AQuizballQuestion::SetMaxCharacters(const EQuestionCategory& category)
{
	if (category == EQuestionCategory::EQC_MANAGERID || category == EQuestionCategory::EQC_PLAYERID || category == EQuestionCategory::EQC_WHOS_MISSING)
		return 40;
	else
		return 60;
}

char AQuizballQuestion::SetSeperateSymbol(const EQuestionCategory& category)
{
	if (category == EQuestionCategory::EQC_MANAGERID || category == EQuestionCategory::EQC_PLAYERID)
		return '>';
	else if (category == EQuestionCategory::EQC_WHOS_MISSING)
		return '_';
	else
		return ' ';
}

void AQuizballQuestion::RemoveCharacter(FString& question, const char& seperateCharacter)
{
	for (int i = 0; i < question.Len(); i++)
	{
		if (question[i] == seperateCharacter)
		{
			question[i] = ' ';
		}
	}
}

void AQuizballQuestion::SetQuestionHelp(const EQuestionHelp& help)
{
	CurrentQuestion.Help = help;
}

void AQuizballQuestion::SetQuestionExtraHelp(const EQuestionHelp& extraHelp)
{
	CurrentQuestion.ExtraHelp = extraHelp;
}

int AQuizballQuestion::CalculatePoints()
{
	switch (CurrentQuestion.Help)
	{
	case EQuestionHelp::EQH_NONE:
		return CurrentQuestion.Points;
	case EQuestionHelp::EQH_50_50:
		return 1;
	case EQuestionHelp::EQH_DOUBLE_POINTS:
		if (CurrentQuestion.ExtraHelp == EQuestionHelp::EQH_50_50)
			return 2;
		else
			return CurrentQuestion.Points * 2;
	default:
		return -1;
	}
}

// Called when the game starts or when spawned
void AQuizballQuestion::BeginPlay()
{
	Super::BeginPlay();
	
}

FString AQuizballQuestion::SeperateQuestionIntoLines(const FString& question, const int32 maxCharactersALine, const char& seperateSymbol)
{
	FString updatedQuestion;
	int32 currentQuestionLength = 0;

	for (int32 i = 0; i < question.Len(); ++i)
	{
		updatedQuestion.AppendChar(question[i]);
		currentQuestionLength++;

		if (seperateSymbol == ' ')
		{
			if (currentQuestionLength >= maxCharactersALine && question[i] == seperateSymbol)
			{
				updatedQuestion.Append(TEXT("\n"));
				currentQuestionLength = 0;
			}
		}
		else
		{
			if (currentQuestionLength >= maxCharactersALine || question[i] == seperateSymbol)
			{
				updatedQuestion.Append(TEXT("\n"));
				currentQuestionLength = 0;
			}
		}
	}

	// Remove all occurrences of the separator character
	RemoveCharacter(updatedQuestion, seperateSymbol);

	return updatedQuestion;
}

void AQuizballQuestion::RemoveSpacesFromStart(TArray<FString>& answers)
{
	for (FString& answer : answers)
	{
		answer.RemoveFromStart(TEXT(" "));
	}
}


// Called every frame
void AQuizballQuestion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

