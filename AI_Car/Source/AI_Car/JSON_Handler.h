// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonVariables.h"
#include "Engine.h"
#include "NeuralNetwork.h"
#include "Templates/SharedPointer.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "JsonUtilities/Public/JsonObjectConverter.h"
/**
 * 
 */
class AI_CAR_API JSON_Handler
{
public:
	JSON_Handler();
	~JSON_Handler();

	// Neural Network
	static void Write_NN(const NeuralNetwork& nn, FString path);
	static NeuralNetwork Load_NN(FString path);

	// Training Data
	static void Write_Training(const TArray<FTrainingData>& data,FString path);
};
