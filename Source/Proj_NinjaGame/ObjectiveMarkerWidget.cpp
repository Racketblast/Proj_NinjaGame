// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectiveMarkerWidget.h"

#include "Animation/WidgetAnimation.h"

void UObjectiveMarkerWidget::PlayFadeIn()
{
	//UE_LOG(LogTemp, Display, TEXT("PlayFadeIn"));
    const float Time = GetCurrentFadeTime();
	
	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, Time, 1, EUMGSequencePlayMode::Forward);
	}
}

void UObjectiveMarkerWidget::PlayFadeOut()
{
	//UE_LOG(LogTemp, Display, TEXT("PlayFadeOut"));
	float Time = FadeAnimation->GetEndTime() - GetCurrentFadeTime();
	if (!IsAnimationPlaying(FadeAnimation))
	{
		Time = FadeAnimation->GetStartTime();
	}
	
	if (FadeAnimation)
	{
		PlayAnimation(FadeAnimation, Time, 1, EUMGSequencePlayMode::Reverse);
	}
}

float UObjectiveMarkerWidget::GetCurrentFadeTime() const
{
		return GetAnimationCurrentTime(FadeAnimation);
}
