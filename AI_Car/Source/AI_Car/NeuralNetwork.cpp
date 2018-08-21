// Fill out your copyright notice in the Description page of Project Settings.

#include "NeuralNetwork.h"

NeuralNetwork::NeuralNetwork()
{
}

NeuralNetwork::NeuralNetwork(TArray<int> topology)
{
	for (int i = 0; i < topology.Num()-1; i++) {
		TArray<TArray<float>> layer;
		for (int j = 0; j < topology[i]; j++) {
			TArray<float> sublayer;
			for (int k = 0; k < topology[i + 1]; k++) {
				sublayer.Add(FMath::FRand());
			}
			layer.Add(sublayer);
		}
		NN.Add(layer);
	}
}

NeuralNetwork::~NeuralNetwork()
{
}

TArray<float> NeuralNetwork::forward(TArray<float> data)
{
	for (int i = 0; i < NN.Num(); ++i) {
		TArray<float> output;
		for (int j = 0; j < NN[i].Num(); ++j) {
			output.Add(0);
			for (int k = 0; k < NN[i][0].Num(); ++k) {
				output[j] += data[j] * NN[i][j][k];
			}
		}
		data = output;
	}
	return data;
}