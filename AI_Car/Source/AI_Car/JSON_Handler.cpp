// Fill out your copyright notice in the Description page of Project Settings.

#include "JSON_Handler.h"

JSON_Handler::JSON_Handler()
{
}

JSON_Handler::~JSON_Handler()
{
}

void JSON_Handler::Write_NN(const NeuralNetwork & nn, FString path)
{
	TSharedPtr<FJsonObject> SaveData = MakeShareable(new FJsonObject);

	TArray< TSharedPtr<FJsonValue> > ObjArray;

	for (auto& a : nn.NN) {
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

NeuralNetwork JSON_Handler::Load_NN(FString path)
{
	NeuralNetwork nn;
	FString result;
	FFileHelper::LoadFileToString(result, *path);
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
		nn.NN.Add(nnlayer);
	}
	return nn;
}

void JSON_Handler::Write_Training(const TArray<FTrainingData>& data,FString path)
{
	TSharedPtr<FJsonObject> SaveData = MakeShareable(new FJsonObject);

	SaveData->SetNumberField("n_input", data[0].input.Num());
	SaveData->SetNumberField("n_output", data[0].output.Num());

	TArray< TSharedPtr<FJsonValue> > ValueArray;
	TArray< TSharedPtr<FJsonValue> > ValueArray2;
	for (auto& a : data) {

		for (auto&b : a.input) {
			TSharedPtr<FJsonValue> Value1 = MakeShareable(new FJsonValueNumber(b));
			ValueArray.Add(Value1);
		}

		for (auto&b : a.output) {
			TSharedPtr<FJsonValue> Value1 = MakeShareable(new FJsonValueNumber(b));  
			ValueArray2.Add(Value1); 
		}

	}
	SaveData->SetArrayField("input",ValueArray);
	SaveData->SetArrayField("output",ValueArray2);

	FString SaveGameStringData;

	TSharedRef< TJsonWriter<> > JsonWriter = TJsonWriterFactory<>::Create(&SaveGameStringData);
	FJsonSerializer::Serialize(SaveData.ToSharedRef(), JsonWriter);

	FFileHelper::SaveStringToFile(*SaveGameStringData, *path);
}
