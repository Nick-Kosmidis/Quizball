// Fill out your copyright notice in the Description page of Project Settings.


#include "QuizballQuestion.h"
#include "Misc/FileHelper.h"
#include "QuizballGameInstance.h"
#include "Engine.h"
#include "Algo/RandomShuffle.h"

// Sets default values
AQuizballQuestion::AQuizballQuestion()
	:m_CurrentQuestion(FQuizballQuestionData()), m_Top5Counter(0), m_WrongAnswerTries(0)
{
	m_CorrectAnswerSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/CorrectAnswer_Cue"));
	m_WrongAnswerSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Sounds/WrongAnswer_Cue"));

 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AQuizballQuestion::LoadQuestion()
{
	FString filePath = FPaths::ProjectContentDir();
	filePath.Append(TEXT("Data/QuizballQuestions.csv"));

	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	TArray<FString> fileLines;

	if (FileManager.FileExists(*filePath))
	{
		if (FFileHelper::LoadFileToStringArray(fileLines, *filePath))
		{
			for (FString line : fileLines)
			{
				TArray<FString> parsedData;
				line.ParseIntoArray(parsedData, TEXT(","), true);

				if (parsedData.Num() == 6)
				{
					FQuizballQuestionData newQuestion;
					newQuestion.Category = (EQuestionCategory)FCString::Atoi(*parsedData[1]);
					newQuestion.Difficulty = (EQuestionDifficulty)FCString::Atoi(*parsedData[2]);

					if (newQuestion.Category == EQuestionCategory::EQC_TOP5)
					{
						parsedData[3].ParseIntoArray(newQuestion.Answers, TEXT("-"), true);
						newQuestion.Tries = true;
					}
					else if (newQuestion.Category == EQuestionCategory::EQC_GUESS_THE_SCORE)
					{
						TArray<FString> scoreAndScorers;
						parsedData[3].ParseIntoArray(scoreAndScorers, TEXT(">"), true);

						if (scoreAndScorers.Num() >= 2)
						{
							FString score = scoreAndScorers[0].Replace(TEXT("Score:"), TEXT("")).TrimStartAndEnd();
							newQuestion.Answers.Add(score);
							FString scorersStr = scoreAndScorers[1].Replace(TEXT("Score:"), TEXT("")).TrimStartAndEnd();
							TArray<FString> scorers;
							scorersStr.ParseIntoArray(scorers, TEXT("-"), true);

							for (FString& scorer : scorers)
							{
								newQuestion.Answers.Add(scorer);
							}
						}
						newQuestion.Tries = false;
					}
					else
					{
						newQuestion.Answers.Add(parsedData[3]);
						newQuestion.Tries = false;
					}

					newQuestion.InitialAnswer = parsedData[3];
					newQuestion.Points = FCString::Atoi(*parsedData[4]);
					newQuestion.Answer50_50 = parsedData[5];
					newQuestion.isPlayed = false;

					int32 maxCharacters = SetMaxCharacters(newQuestion.Category);
					char separateSymbol = SetSeperateSymbol(newQuestion.Category);
					newQuestion.Question = SeperateQuestionIntoLines(parsedData[0], maxCharacters, separateSymbol);
					RemoveSpacesFromStart(newQuestion.Answers);

					m_QuizballQuestions.Add(newQuestion);
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
						FString::Printf(TEXT("Wrong format: %d fields. Line: %s"), parsedData.Num(), *line));
				}
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, TEXT("Failed to load file."));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT("File does not exist."));
	}
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black,
			FString::Printf(TEXT("Total questions: %d "), m_QuizballQuestions.Num()));
}


void AQuizballQuestion::SetCurrentQuestion(const EQuestionCategory& category, const EQuestionDifficulty& difficulty)
{
	for (FQuizballQuestionData& question : m_SelectedQuestions)
	{
		if (question.Category == category && question.Difficulty == difficulty && !question.isPlayed)
		{
			m_CurrentQuestion = question;
			question.isPlayed = true;
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

	for (const FString& correctAnswer : m_CurrentQuestion.Answers)
	{
		if (m_CurrentQuestion.Category != EQuestionCategory::EQC_GUESS_THE_SCORE)
		{
			if (answer.Len() >= correctAnswer.Len() / 3 && answer.Len() <= correctAnswer.Len())
			{
				if (correctAnswer.Contains(answer) || answer.Contains(correctAnswer))
				{
					return true;
				}
			}
		}
		else
		{
			if (correctAnswer.Contains(answer) || answer.Contains(correctAnswer))
			{
				return true;
			}
		}
	}

	// If no matches are found, return false
	return false;
}

bool AQuizballQuestion::CheckGuessAnswer(const FString& answer1, const FString& answer2, bool& correctScore, bool& correctScorers)
{
	if (answer1.IsEmpty() || !m_CurrentQuestion.Answers[0].Equals(answer1, ESearchCase::IgnoreCase))
	{
		correctScore = false;
	}
	else
	{
		correctScore = true;
	}

	TArray<FString> scorers;
	answer2.ParseIntoArray(scorers, TEXT("-"));

	for (FString& scorer : scorers)
	{
		scorer = scorer.TrimStartAndEnd();
	}

	TSet<FString> correctAnswers;
	TSet<FString> givenScorers;

	for (int32 i = 1; i < m_CurrentQuestion.Answers.Num(); i++)
	{
		correctAnswers.Add(m_CurrentQuestion.Answers[i].ToLower());
	}

	for (const FString& scorer : scorers)
	{
		givenScorers.Add(scorer.ToLower());
	}

	if (correctAnswers.Num() != givenScorers.Num())
	{
		correctScorers = false;
	}

	correctScorers = correctAnswers.Includes(givenScorers);

	return correctScore || correctScorers;
}

void AQuizballQuestion::DisableQuestion(const FQuizballQuestionData& currentQuestion)
{
	for (auto& question : m_QuizballQuestions)
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
		return 50;
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

void AQuizballQuestion::SelectRandomQuestions()
{
	TArray<FQuizballQuestionData> selectedQuestions;
	TSet<FString> alreadySelectedQuestions;

	// Λίστα με όλες τις απαιτήσεις ανά κατηγορία και δυσκολία
	struct FSelectionCriteria
	{
		EQuestionCategory Category;
		EQuestionDifficulty Difficulty;
		int32 Count;
	};

	TArray<FSelectionCriteria> criteria = 
	{
		{EQuestionCategory::EQC_HISTORY, EQuestionDifficulty::EQD_EASY, 1},
		{EQuestionCategory::EQC_HISTORY, EQuestionDifficulty::EQD_MEDIUM, 1},
		{EQuestionCategory::EQC_HISTORY, EQuestionDifficulty::EQD_HARD, 1},

		{EQuestionCategory::EQC_GEOGRAPHY, EQuestionDifficulty::EQD_EASY, 1},
		{EQuestionCategory::EQC_GEOGRAPHY, EQuestionDifficulty::EQD_MEDIUM, 1},
		{EQuestionCategory::EQC_GEOGRAPHY, EQuestionDifficulty::EQD_HARD, 1},

		{EQuestionCategory::EQC_TOP5, EQuestionDifficulty::EQD_HARD, 2},
		{EQuestionCategory::EQC_WHOS_MISSING, EQuestionDifficulty::EQD_HARD, 2},

		{EQuestionCategory::EQC_PLAYERID, EQuestionDifficulty::EQD_MEDIUM, 2},

		{EQuestionCategory::EQC_GOSSIP, EQuestionDifficulty::EQD_EASY, 1},
		{EQuestionCategory::EQC_GOSSIP, EQuestionDifficulty::EQD_MEDIUM, 1},

		{EQuestionCategory::EQC_MANAGERID, EQuestionDifficulty::EQD_EASY, 2},

		{EQuestionCategory::EQC_GUESS_THE_SCORE, EQuestionDifficulty::EQD_EASY, 2},
	};

	for(const FSelectionCriteria& criterion : criteria)
	{
		TArray<FQuizballQuestionData> FilteredQuestions;

		for (const FQuizballQuestionData& question : m_QuizballQuestions)
		{
			if (question.Category == criterion.Category &&
				question.Difficulty == criterion.Difficulty &&
				!question.isPlayed && !alreadySelectedQuestions.Contains(question.Question))
			{
				FilteredQuestions.Add(question);
			}
		}

		/*FilteredQuestions.Sort([](const FQuizballQuestionData&, const FQuizballQuestionData&)
		{
			return FMath::RandBool(); // Random sort
		});*/

		Algo::RandomShuffle(FilteredQuestions);

		for (int32 i = 0; i < criterion.Count && i < FilteredQuestions.Num(); i++)
		{
			const FQuizballQuestionData& Selected = FilteredQuestions[i];
			selectedQuestions.Add(Selected);
			alreadySelectedQuestions.Add(Selected.Question);

			for (FQuizballQuestionData& q : m_QuizballQuestions)
			{
				if (q.Question == FilteredQuestions[i].Question)
				{
					q.isPlayed = true;
					break;
				}
			}
		}
	}

	m_SelectedQuestions = selectedQuestions;

}

void AQuizballQuestion::SetQuestionHelp(const EQuestionHelp& help)
{
	m_CurrentQuestion.Help = help;
}

void AQuizballQuestion::SetQuestionExtraHelp(const EQuestionHelp& extraHelp)
{
	m_CurrentQuestion.ExtraHelp = extraHelp;
}

int AQuizballQuestion::CalculatePoints()
{
	if (m_CurrentQuestion.Category != EQuestionCategory::EQC_TOP5)
	{
		switch (m_CurrentQuestion.Help)
		{
		case EQuestionHelp::EQH_NONE:
			return m_CurrentQuestion.Points;
		case EQuestionHelp::EQH_50_50:
			return 1;
		case EQuestionHelp::EQH_DOUBLE_POINTS:
			if (m_CurrentQuestion.ExtraHelp == EQuestionHelp::EQH_50_50)
				return 2;
			else
				return m_CurrentQuestion.Points * 2;
		default:
			return -1;
		}
	}
	else
	{
		if (m_Top5Counter == 5)
		{
			ResetTop5Properties();
			switch (m_CurrentQuestion.Help)
			{
			case EQuestionHelp::EQH_NONE:
				return m_CurrentQuestion.Points;
			case EQuestionHelp::EQH_DOUBLE_POINTS:
					return m_CurrentQuestion.Points * 2;
			default:
				return -1;
			}
		}
		else if (m_Top5Counter == 4)
		{
			ResetTop5Properties();
			switch (m_CurrentQuestion.Help)
			{
			case EQuestionHelp::EQH_NONE:
				return 2;
			case EQuestionHelp::EQH_DOUBLE_POINTS:
				return 4;
			default:
				return -1;
			}
		}
	}

	return -1;
}

int AQuizballQuestion::CalculateGuessTheScorePoints(bool correctScore, bool correctScorers)
{
	if (!correctScore)
		return 0;
	else if (correctScore && !correctScorers)
		return 1;
	else
		return 2;
}

bool AQuizballQuestion::CheckGameEnd()
{
	for (const auto& question : m_QuizballQuestions)
	{
		if (!question.isPlayed)
			return false;
	}
	return true;
}

void AQuizballQuestion::HandleQuestions(const FString& answer)
{
	switch (m_CurrentQuestion.Category)
	{
	case EQuestionCategory::EQC_TOP5:
		HandleTop5Question(answer);
		break;
	default:
		HandleSimpleQuestion(answer);
		break;
	}
}

int AQuizballQuestion::HandleSimpleQuestion(FString answer)
{
	if (CheckAnswer(answer))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_CorrectAnswerSound);
		int32 points = CalculatePoints();

		return points;
	}
	else
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_WrongAnswerSound);
		return 0;
	}

	return 0;
}

int AQuizballQuestion::HandleTop5Question(FString answer)
{
	int32 correctAnswerIndex = -1;
	if (CheckAnswer(answer))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_CorrectAnswerSound);
		m_Top5Counter++;
		correctAnswerIndex = FindAnswerByIndex(answer);
	}
	else
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_WrongAnswerSound);
		m_WrongAnswerTries++;
		if (m_WrongAnswerTries > 1)
		{
			m_CurrentQuestion.Tries = false;
		}
	}

	return correctAnswerIndex;
}

int AQuizballQuestion::HandleGuessTheScoreQuestion(const FString& score, const FString& scorers)
{
	if (CheckGuessAnswer(score, scorers, bCorrectScore, bCorrectScorers))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_CorrectAnswerSound);
		int32 points = CalculateGuessTheScorePoints(bCorrectScore, bCorrectScorers);

		return points;
	}
	else
	{
		UGameplayStatics::PlaySound2D(GetWorld(), m_WrongAnswerSound);
		return 0;
	}
}

// Called when the game starts or when spawned
void AQuizballQuestion::BeginPlay()
{
	Super::BeginPlay();

	LoadQuestion();
	SelectRandomQuestions();
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

int32 AQuizballQuestion::FindAnswerByIndex(const FString& answer)
{
	for (int32 i = 0; i < m_CurrentQuestion.Answers.Num(); i++)
	{
		if (m_CurrentQuestion.Answers[i].Contains(answer) || answer.Contains(m_CurrentQuestion.Answers[i]))
		{
			return i;
		}
	}
	return -1;
}

void AQuizballQuestion::ResetTop5Properties()
{
	m_Top5Counter = 0;
	m_WrongAnswerTries = 0;
}


// Called every frame
void AQuizballQuestion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

