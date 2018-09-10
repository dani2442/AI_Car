// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Classes/Components/SplineComponent.h"
#include "Track.h"
#include "NeuralNetwork.h"
#include "Car.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI_Controller.generated.h"

UCLASS()
class AI_CAR_API AAI_Controller : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAI_Controller();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		int show_cars = 6; // Number of cars' mesh that are shown

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		int population = 30; // Number of individuals

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		float importance_diversity = 0.2; // Probability to be choosen = fitness * (1 - importance_diversity) + diversity * importance_diversity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm")
		bool crossover = true; // Do we use crossover?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm",meta =(UIMin = "0.0", UIMax = "1.0"))
		float crossover_rate = 0.2; // the probability to experience crossover

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm",meta =(UIMin = "0.0", UIMax = "1.0"))
		float deadline = 0.2; // Avarage percentage that we select

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm",meta =(UIMin = "0.0", UIMax = "0.49"))
		float population_selection = 0.1; // Number of indivuals that we select to create the next generation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm",meta =(UIMin = "0.0", UIMax = "1.0"))
		float mutation_rate = 0.5; // the probability to be mutated

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Genetic Algorithm",meta =(UIMin = "0.0", UIMax = "1.0"))
		float mutation_change = 0.2; // Maximum change that can experience a weight during mutation



	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Network")
		int InputLayer=4; // Number of input neurons

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Neural Network")
		TArray<int> HiddenLayer = { 3 }; // Topology of hidden layer

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Neural Network")
		int OutputLayer= 2; // Number of outputs (fixed) turn right or left



	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category = "AI Controller")
		TSubclassOf<class ACar> OurSpawningObject;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "AI Controller")
		ATrack *OurTrack;

	APlayerController* OurPlayer;

	void Initialize(bool learn=false);
	void ReInitialize();

	TArray<ACar*> Cars;
	TArray<float>Score;

	void Probability();

	TArray<int> position;
	int last_best=0;
	void CalcPosition();

	float refresh_frecuency=0.2;
private:

	float importance_diversity2;

	FString RelativePath = FPaths::ProjectContentDir();

	TArray<int> topology;

	float delta;
	float sumDelta=0;

	int init_target;
	int current_target;

	void CheckHit();
	void RefreshCarPosition();

	float TotalDiversity=0;
	void CalcDiversity();
	float TotalFitness = 0;
	void CalcFitness();

	void GeneticAlgorithm();
		void GA_Selection(); TArray<NeuralNetwork> selections;
		void GA_Crossover();
		void GA_Mutation();
};
