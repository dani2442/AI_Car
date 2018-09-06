// Fill out your copyright notice in the Description page of Project Settings.

#include "Track.h"


// Sets default values
ATrack::ATrack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	OurTrack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Our mesh"));
	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("Path"));
	
}

// Called when the game starts or when spawned
void ATrack::BeginPlay()
{
	Super::BeginPlay();
	
	this->n_target = this->PathSpline->GetNumberOfSplinePoints();
}

// Called every frame
void ATrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATrack::InitCenters() {	
	points[0]=PathSpline->GetWorldLocationAtSplinePoint(n_target);
	points[1]=PathSpline->GetWorldLocationAtSplinePoint(0);
	for (int i = 0; i < n_target-1; i++) {
		points[position % 3]=PathSpline->GetWorldLocationAtSplinePoint(i+1);
		Centers.Add(CalcCenter());
		position++;
	}
	points[position % 3] = PathSpline->GetWorldLocationAtSplinePoint(n_target);
	Centers.Add(CalcCenter());
}

FVector ATrack::CalcCenter()
{
	for (int i = 0; i < 3; i++) {
		h_square[i] = points[i].X*points[i].X + points[i].Y*points[i].Y;
	}
	h_divisor = 2 * (points[0].X*(points[1].Y - points[2].Y) - points[0].Y*(points[1].X - points[2].X) + points[1].X*points[2].Y - points[2].X*points[1].Y);

	FVector temp;
	temp.X = (h_square[0] * (points[1].Y - points[2].Y) + h_square[1] * (points[2].Y - points[0].Y) + h_square[2] * (points[0].Y - points[1].Y)) /
		h_divisor;

	temp.Y = (h_square[0] * (points[2].X - points[1].X) + h_square[1] * (points[0].X - points[2].X) + h_square[2] * (points[1].X - points[0].X)) /
		h_divisor;

	temp.Z = 0;
	return temp;
}


