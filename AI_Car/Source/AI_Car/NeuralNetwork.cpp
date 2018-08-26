// Fill out your copyright notice in the Description page of Project Settings.



#include "NeuralNetwork.h"

NeuralNetwork::NeuralNetwork()
{
}

NeuralNetwork::NeuralNetwork(FString path)
{
	Load(path);
}

NeuralNetwork::NeuralNetwork(TArray<int> topology)
{
	Create(topology);
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
			output[j] = Sigmoid(output[j]);
		}
		data = output;
	}
	return data;
}

void NeuralNetwork::Load(FString)
{

}

void NeuralNetwork::Create(TArray<int> topology)
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

void NeuralNetwork::Write(FString path)
{
	
	TSharedPtr<FJsonObject> SaveData = MakeShareable(new FJsonObject);

	 TArray< TSharedPtr<FJsonValue> > ValueArray;

	 TArray< TSharedPtr<FJsonValue> > ValueArray2;
 
	 // Create two values and add them to the array
	 TSharedPtr<FJsonValue> Value1 = MakeShareable(new FJsonValueNumber(4.34));    
	 ValueArray2.Add(Value1);    
	 TSharedPtr<FJsonValue> Value2 = MakeShareable(new FJsonValueNumber(4.34));    
	 ValueArray2.Add(Value2);

	 ValueArray.Add(ValueArray2);
	 ValueArray.Add(ValueArray2);
 
 // Add the array to the Json object
 SaveData->SetArrayField("NN", ValueArray);

	FString SaveGameStringData;

	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&SaveGameStringData);
	FJsonSerializer::Serialize(SaveData.ToSharedRef(), JsonWriter);

	FFileHelper::SaveStringToFile(*SaveGameStringData, *path);
}
