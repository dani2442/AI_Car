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

void NeuralNetwork::Init(TArray<int> topology)
{
	NN.Empty();

	for (int i = 0; i < topology.Num()-1; i++) {
		TArray<TArray<float>> layer;
		for (int j = 0; j < topology[i]; j++) {
			TArray<float> sublayer;
			for (int k = 0; k < topology[i + 1]; k++) {
				float my = FMath::FRand();
				sublayer.Add(my);
			}
			layer.Add(sublayer);
		}
		NN.Add(layer);
	}
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

void NeuralNetwork::Load(FString path)
{
	NN.Empty();
	FString result;
	FFileHelper::LoadFileToString(result,*path);
	FNN JsonData;

	FJsonObjectConverter::JsonObjectStringToUStruct<FNN>(result, &JsonData, 0, 0);
	
	//UE_LOG(LogTemp, Warning, TEXT("%s"), *result);

	for (FNNLayer& layer : JsonData.NN) {
		TArray<TArray<float>> nnlayer;
		for (int i = 0; i < layer.n_neurons_pre; i++) {
			TArray<float> sublayer;
			for (int j = 0; j < layer.n_neurons_pos; j++) {
				sublayer.Add(layer.weight[j + i * layer.n_neurons_pos]);
			}
			nnlayer.Add(sublayer);
		}
		NN.Add(nnlayer);
	}
}

void NeuralNetwork::Create(TArray<int> topology)
{
	for (int i = 0; i < topology.Num()-1; i++) {
		TArray<TArray<float>> layer;
		for (int j = 0; j < topology[i]; j++) {
			TArray<float> sublayer;
			for (int k = 0; k < topology[i + 1]; k++) {
				sublayer.Add(FMath::FRand()*2-1);
			}
			layer.Add(sublayer);
		}
		NN.Add(layer);
	}
}

void NeuralNetwork::Write(FString path)
{
	
	TSharedPtr<FJsonObject> SaveData = MakeShareable(new FJsonObject);

	TArray< TSharedPtr<FJsonValue> > ObjArray;

	for (auto& a : NN) {
		TSharedPtr< FJsonObject > JsonObj = MakeShareable(new FJsonObject);
		JsonObj->SetNumberField("n_neurons_pre",a.Num());
		JsonObj->SetNumberField("n_neurons_pos",a[0].Num());

		TArray< TSharedPtr<FJsonValue> > ValueArray;
		for (auto&b : a) {
			for (auto&c : b) {
				TSharedPtr<FJsonValue> Value1 = MakeShareable(new FJsonValueNumber(c));  
				ValueArray.Add(Value1); 
			}
		}
		JsonObj->SetArrayField("weight",ValueArray);
		TSharedRef< FJsonValueObject > JsonValue = MakeShareable( new FJsonValueObject( JsonObj) );
		ObjArray.Add(JsonValue);
	}

	SaveData->SetArrayField("NN", ObjArray);


	FString SaveGameStringData;

	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&SaveGameStringData);
	FJsonSerializer::Serialize(SaveData.ToSharedRef(), JsonWriter);

	FFileHelper::SaveStringToFile(*SaveGameStringData, *path);
}
