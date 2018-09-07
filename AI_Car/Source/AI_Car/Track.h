// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"
#include "Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Track.generated.h"

UCLASS()
class AI_CAR_API ATrack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MyCategory",meta = (AllowPrivateAccess = "true"))
         USplineComponent* PathSpline;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "MyCategory")
		UStaticMeshComponent* OurTrack;

	bool FinishOnStart;

	TArray<FVector> SplinePoints;

	TArray<float> AcumulativeDistance;

	TArray<FVector> Centers;
	int n_target;

	float TotalDistance;

	void Init();
	void InitCenters();
	void InitCircuitDistance();

	int CalcNearestPoint(const FVector& actorloc);
	void UpdatePoint(const FVector& actorloc,int &index);

	float CalcRectPosition(const FVector& actorloc,const int index);

private:

	FVector CalcCenter(); float h_square[3], h_divisor;

	int position = 2;
	FVector points[3];
};
