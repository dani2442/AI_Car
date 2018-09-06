// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
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

	void Initialize(bool learn=false);
	void Learning();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Controller")
		int show_cars = 6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Controller")
		int population = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite,Category = "AI Controller")
		TSubclassOf<class ACar> OurSpawningObject;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "AI Controller")
		ATrack *OurTrack;

	TArray<ACar*> Cars;

	NeuralNetwork best;
private:
	FString RelativePath = FPaths::ProjectContentDir();


	int current_target;
	float circuit_ditance;

	void CheckHit();
	void InitLine();
	void PercentageRace();
};
