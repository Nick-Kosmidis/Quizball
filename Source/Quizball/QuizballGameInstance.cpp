// Fill out your copyright notice in the Description page of Project Settings.


#include "QuizballGameInstance.h"
#include "PlayerPawn.h"

UQuizballGameInstance::UQuizballGameInstance()
	:m_Player1Turn(true), m_PlayerIndex(0)
{

}

void UQuizballGameInstance::AddPlayer(FPlayerProperties player)
{
	FPlayerProperties playerToAdd;
	playerToAdd.Name = player.Name;
	playerToAdd.Points = player.Points;
	playerToAdd.Use50_50 = player.Use50_50;
	playerToAdd.UseDoublePoints = player.UseDoublePoints;
	m_PlayerProperties.Add(playerToAdd);
}

int32 UQuizballGameInstance::GetPlayerIndexOnTurn()
{
	if (m_PlayerProperties.Num() == 1)
		return 0;

	if (m_Player1Turn)
	{
		m_PlayerIndex = 0;
	}
	else
	{
		m_PlayerIndex = 1;
	}
	return m_PlayerIndex;
}

void UQuizballGameInstance::ChangeTurn()
{
	if (m_PlayerProperties.Num() > 1)
		m_Player1Turn = !m_Player1Turn;
}

void UQuizballGameInstance::AddPoints(int32 points)
{
	if (m_Player1Turn)
	{
		m_PlayerProperties[0].Points += points;
	}
	else
	{
		m_PlayerProperties[1].Points += points;
	}
}
