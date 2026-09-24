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
					else if (newQuestion.Category == EQuestionCategory::EQC_CLUB_COMBO)
					{
						parsedData[3].ParseIntoArray(newQuestion.Answers, TEXT("&"), true);
						newQuestion.Tries = false;
					}
					else if (newQuestion.Category == EQuestionCategory::EQC_GUESS_THE_SCORE)
					{
						TArray<FString> scoreAndScorers;
						parsedData[3].ParseIntoArray(scoreAndScorers, TEXT(">"), true);

						if (scoreAndScorers.Num() >= 2)
						{
							FString score = scoreAndScorers[0].Replace(TEXT("Score:"), TEXT("")).TrimStartAndEnd();
							newQuestion.Answers.Add(score);
							FString scorersStr = scoreAndScorers[1].Replace(TEXT("Scorers:"), TEXT("")).TrimStartAndEnd();
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

					if (newQuestion.Category == EQuestionCategory::EQC_WHOS_MISSING)
					{
						newQuestion.Question = parsedData[0].TrimStartAndEnd();
					}
					else
					{
						int32 maxCharacters = SetMaxCharacters(newQuestion.Category);
						char separateSymbol = SetSeperateSymbol(newQuestion.Category);

						if (newQuestion.Category == EQuestionCategory::EQC_PLAYERID || newQuestion.Category == EQuestionCategory::EQC_MANAGERID)
						{
							newQuestion.Question = parsedData[0];
						}
						else
						{
							newQuestion.Question = SeperateQuestionIntoLines(parsedData[0], maxCharacters, separateSymbol);
						}

					}
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
			if (question.Category == EQuestionCategory::EQC_WHOS_MISSING)
			{
				TArray<FString> matchAndScore;
				question.Question.ParseIntoArray(matchAndScore, TEXT("Lineups:"));

				question.Question = matchAndScore[0].TrimStartAndEnd();
				m_WhosMissingPlayers = matchAndScore[1].TrimStartAndEnd();
			}
			else if (question.Category == EQuestionCategory::EQC_LOGO_QUIZ)
			{
				if (difficulty == EQuestionDifficulty::EQD_EASY)
				{
					question.Image = LoadObject<UTexture2D>(nullptr, *(GetLogoPath(question)));
				}
				else
				{
					question.Image = LoadObject<UTexture2D>(nullptr, *(GetLogoPath(question)));
				}
			}
			m_CurrentQuestion = question;
			question.isPlayed = true;
			break;
		}
	}
}

bool AQuizballQuestion::CheckAnswer(const FString& answer)
{
	if (m_CurrentQuestion.Category == EQuestionCategory::EQC_LOGO_QUIZ)
	{
		if (m_CurrentQuestion.Difficulty == EQuestionDifficulty::EQD_EASY)

			m_CurrentQuestion.Image = LoadObject<UTexture2D>(nullptr, *(GetLogoPath(m_CurrentQuestion) + "Correct"));
		else
			m_CurrentQuestion.Image = LoadObject<UTexture2D>(nullptr, *(GetLogoPath(m_CurrentQuestion) + "Correct"));
	}

	if (answer.IsEmpty())
	{
		return false;
	}

	/*if (m_CurrentQuestion.Category == EQuestionCategory::EQC_HIGHER_LOWER)
	{
		return true;
	}*/

	for (const FString& correctAnswer : m_CurrentQuestion.Answers)
	{
		int seperatorIndex;

		if (correctAnswer.FindChar(' ', seperatorIndex))
		{
			FString name = correctAnswer.Left(seperatorIndex);
			FString surname = correctAnswer.Mid(seperatorIndex+1);

			if (answer == name || answer == surname || answer == correctAnswer)
			{
				return true;
			}
		}
		else if (answer == correctAnswer)
		{
			return true;
		}
	}

	// If no matches are found, return false
	return false;
}

bool AQuizballQuestion::CheckGuessAnswer(const FString& answer1, const FString& answer2)
{

	bCorrectScore = !answer1.IsEmpty() && m_CurrentQuestion.Answers[0].Equals(answer1, ESearchCase::IgnoreCase);

	TArray<FString> correctScorers;
	for (int32 i = 1; i < m_CurrentQuestion.Answers.Num(); i++)
	{
		FString scorer = m_CurrentQuestion.Answers[i].TrimStartAndEnd().ToLower();
		correctScorers.Add(scorer);
	}
	correctScorers.Sort();

	TArray<FString> playerScorers;
	answer2.ParseIntoArray(playerScorers, TEXT(","));
	for (FString& s : playerScorers)
	{
		s = s.TrimStartAndEnd().ToLower();
	}
	playerScorers.Sort();

	// Check if all correct scorers are guessed
	bCorrectScorers = (correctScorers.Num() > 0) && (correctScorers.Num() == playerScorers.Num());

	if (bCorrectScorers)
	{
		for (FString& scorer : playerScorers)
		{
			bool scorerFound = false;
			for (FString& correctScorer : correctScorers)
			{
				if (correctScorer.Contains(scorer) || scorer.Contains(correctScorer))
				{
					scorerFound = true;
					break;
				}
			}
			if (!scorerFound)
			{
				bCorrectScorers = false;
				break;
			}
		}
	}

	return bCorrectScore || bCorrectScorers;
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

		{EQuestionCategory::EQC_HIGHER_LOWER, EQuestionDifficulty::EQD_EASY, 2}, 

		{EQuestionCategory::EQC_CLUB_COMBO, EQuestionDifficulty::EQD_EASY, 2},

		{EQuestionCategory::EQC_LOGO_QUIZ, EQuestionDifficulty::EQD_EASY, 1},
		{EQuestionCategory::EQC_LOGO_QUIZ, EQuestionDifficulty::EQD_MEDIUM, 1},
	};

	for (const FSelectionCriteria& criterion : criteria)
	{
		TArray<FQuizballQuestionData> FilteredQuestions;

		for (const FQuizballQuestionData& question : m_QuizballQuestions)
		{
			if (question.Category == criterion.Category &&
				question.Difficulty == criterion.Difficulty &&
				!question.isPlayed) // && (!alreadySelectedQuestions.Contains(question.Question) && question.Category != EQuestionCategory::EQC_LOGO_QUIZ))
			{
				FilteredQuestions.Add(question);
			}
		}

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

FString AQuizballQuestion::GetPlayerPosition(const FString& player)
{
	int32 OpenIndex, CloseIndex;

	if (player.FindChar('(', OpenIndex) && player.FindChar(')', CloseIndex))
	{
		return player.Mid(OpenIndex + 1, CloseIndex - OpenIndex - 1);
	}

	return TEXT("Unknown");
}

FIDInfo AQuizballQuestion::GetIDInfo(const FString& question)
{
	TArray<FString> infos, teams, periods;
	FIDInfo playerBiography;
	question.ParseIntoArray(infos, TEXT(">"), true);

	for (const FString& info : infos)
	{
		FString trimmedInfo = info.TrimStartAndEnd();
		int32 seperatorIndex;
		if (trimmedInfo.FindChar(']', seperatorIndex))
		{
			FString period = trimmedInfo.Left(seperatorIndex);
			period.RemoveAt(0);
			FString team = trimmedInfo.Mid(seperatorIndex + 1);

			teams.Add(team.TrimStartAndEnd());
			periods.Add(period.TrimStartAndEnd());
		}
	}

	for (size_t i = 0; i < teams.Num(); i++)
	{
		playerBiography.Teams.Append(teams[i] + "\n");
		playerBiography.Periods.Append(periods[i] + "\n");
	}

	return playerBiography;
}

FString AQuizballQuestion::RevealAnswer()
{
	if (m_CurrentQuestion.Category != EQuestionCategory::EQC_CLUB_COMBO)
	{
		return m_CurrentQuestion.Answers[0];
	}
	else
	{
		FString clubComboPlayers;
		if (m_CurrentQuestion.Answers.Num() >= 5)
		{
			for (int32 i = 0; i < m_CurrentQuestion.Answers.Num(); ++i)
			{
				if (i == m_CurrentQuestion.Answers.Num() / 2)
				{
					clubComboPlayers.AppendChar('\n');
				}

				clubComboPlayers.Append(m_CurrentQuestion.Answers[i].TrimStartAndEnd());

				if (i < m_CurrentQuestion.Answers.Num() - 1)
				{
					clubComboPlayers.Append(", ");
				}
			}

		}
		else
		{
			for (int32 i = 0; i < m_CurrentQuestion.Answers.Num(); ++i)
			{

				clubComboPlayers.Append(m_CurrentQuestion.Answers[i].TrimStartAndEnd());

				if (i < m_CurrentQuestion.Answers.Num() - 1)
				{
					clubComboPlayers.Append(", ");
				}
			}
		}
		return "Players: " + clubComboPlayers;
	}
}

TArray<FString> AQuizballQuestion::SeperateHigherLower()
{
	TArray<FString> HigherLower;
	m_CurrentQuestion.Question.ParseIntoArray(HigherLower, TEXT("."));

	TArray<FString> playerPairs;
	HigherLower[0].ParseIntoArray(playerPairs, TEXT("-"));

	for (FString& pair : playerPairs)
	{
		pair = pair.TrimStartAndEnd();
	}
	return playerPairs;
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
				return 1;
			case EQuestionHelp::EQH_DOUBLE_POINTS:
				return 2;
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
	else
	{
		int32 guessPoints = 0;
		if (correctScorers)
			guessPoints = 2;
		else
			guessPoints = 1;

		if(m_CurrentQuestion.Help == EQuestionHelp::EQH_DOUBLE_POINTS)
		{
			return 2 * guessPoints;
		}
		else
		{
			return guessPoints;
		}
	}
}

bool AQuizballQuestion::CheckGameEnd()
{
	size_t playedQuestions = 0U;
	for (const auto& question : m_SelectedQuestions)
	{
		if (question.isPlayed)
			playedQuestions++;
	}
	
	if (playedQuestions == m_SelectedQuestions.Num() - 2)
	{
		return true;
	}

	return false;
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
	if (CheckGuessAnswer(score, scorers))
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

FString AQuizballQuestion::CleanSeparators(const FString& input, const char& separator)
{
	TArray<FString> parts;
	FString cleaned;

	input.ParseIntoArray(parts, *FString::Chr(separator), true);

	for (int32 i = 0; i < parts.Num(); i++)
	{
		cleaned.Append(parts[i].TrimStartAndEnd());

		if (i < parts.Num() - 1)
		{
			cleaned.AppendChar(separator);
		}
	}

	return cleaned;
}

FString AQuizballQuestion::GetLogoPath(const FQuizballQuestionData& question)
{
	FString logo = question.Answers[0].Replace(TEXT(" "), TEXT(""));
	FString logoPath = question.Answers[0].Replace(TEXT(" "), TEXT("_"));
	return "/Game/Data/Logos/" + logoPath + "/" + logo;
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

