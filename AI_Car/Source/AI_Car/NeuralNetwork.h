// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class AI_CAR_API NeuralNetwork
{
public:
	NeuralNetwork();
	NeuralNetwork(TArray<int> topology);
	~NeuralNetwork();

	TArray<double> forward(TArray<double> data);

private:
	TArray<TArray<TArray<double>>> NN;
};