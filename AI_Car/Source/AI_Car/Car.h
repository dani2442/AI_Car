// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Array.h"
#include "NeuralNetwork.h"
#include "Engine.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Car.generated.h"

UCLASS()
class AI_CAR_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* OurVisibleActor;

	UPROPERTY(EditAnywhere)
		UBoxComponent* BoxComponent;

	UPROPERTY(EditAnywhere)
		UBoxComponent* Component;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		int StickNumber = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		bool best = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float amplitude = 90.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float VelocityX = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float RotationVelocityPawn = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Car")
		float MaxDistance = 2000.f;

	FCollisionQueryParams CollisionParams;

	UFUNCTION()
		void OnCompHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	NeuralNetwork nn;
	bool hit = false;

	void Change();

	int lastTarget=0;

	float percentage;
	int laps;

private:
	float NNproportion;
};

