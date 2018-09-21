// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonVariables.generated.h"

/**
 * 
 */
UCLASS()
class AI_CAR_API UJsonVariables : public UObject
{
	GENERATED_BODY()
	
	
	
	
};

USTRUCT()
struct FTrainingData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
		TArray<float> input;

	UPROPERTY()
		TArray<float> output;
};


USTRUCT()
struct FNNLayer
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		int32 n_neurons_pre;

	UPROPERTY()
		int32 n_neurons_pos;

	UPROPERTY()
		TArray<float> weight;
};

USTRUCT()
struct FNN
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY()
		TArray<FNNLayer> NN;
};
