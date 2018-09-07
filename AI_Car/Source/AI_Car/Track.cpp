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
	
	this->FinishOnStart = this->PathSpline->IsClosedLoop();
	this->n_target = this->PathSpline->GetNumberOfSplinePoints();
	UE_LOG(LogInit,Warning,TEXT("number of spline points: %i"), n_target );

	for (int i = 0; i < n_target; i++)
		this->SplinePoints.Add(PathSpline->GetWorldLocationAtSplinePoint(i));

	InitCircuitDistance();
	InitCenters();
}

// Called every frame
void ATrack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATrack::Init()
{
}

void ATrack::InitCenters() {
	points[0]=SplinePoints[n_target-1];
	points[1]=SplinePoints[0];
	for (int i = 0; i < n_target-1; i++) {
		points[position % 3]=SplinePoints[i+1];
		Centers.Add(CalcCenter());
		position++;
	}
	points[position % 3] = SplinePoints[0];
	Centers.Add(CalcCenter());
}

void ATrack::InitCircuitDistance()
{
	float distance = 0.f;
	FVector target[2];
	bool state = true;

	target[!state]=SplinePoints[0];
	AcumulativeDistance.Add(distance);

	for (int i = 1; i < n_target; i++) {
		target[state]=SplinePoints[i];
		
		distance += sqrt(pow(target[0].X - target[1].X, 2) + pow(target[0].Y - target[1].Y, 2));
		//UE_LOG(LogTemp,Warning,TEXT("acumulative distance %i: %f"),i, distance );
		AcumulativeDistance.Add(distance);

		state = !state;
	}
	
	if (FinishOnStart) { // last -> 0
		target[state] = PathSpline->GetWorldLocationAtSplinePoint(0);
		distance += sqrt(pow(target[0].X - target[1].X, 2) + pow(target[0].Y - target[1].Y, 2));
		//UE_LOG(LogTemp,Warning,TEXT("acumulative distance last->0: %f"), distance );
		AcumulativeDistance.Add(distance);
	}

	this->TotalDistance = distance;
	//GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("distance total:%f"), distance));
}

int ATrack::CalcNearestPoint(const FVector & actorloc)
{
	FVector target;
	float distance = 0.f, distance2 = 1000000000.f,newdistance;
	int point=0;
	bool state = true;

	for (int i = !FinishOnStart; i < n_target; i++) {
		target=PathSpline->GetWorldLocationAtSplinePoint(i);

		newdistance = pow(actorloc.X - target.X, 2) + pow(actorloc.Y - target.Y, 2);
		if (newdistance < distance2) {
			distance2 = newdistance;
			point = i;
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("nearest point:%i"), point));
	return point;
}

void ATrack::UpdatePoint(const FVector & actorloc,int& index)
{
	FVector target[2];
	float distance2 = 10000000.f;
	int nextindex;

	target[0] = SplinePoints[index];
	if (index == n_target-1) {
		target[1] = SplinePoints[0];
		nextindex = 0;
	}
	else {
		target[1] = SplinePoints[index + 1];
		nextindex = index + 1;
	}
	
	if (pow(actorloc.X - target[0].X, 2) + pow(actorloc.Y - target[0].Y, 2) > pow(actorloc.X - target[1].X, 2) + pow(actorloc.Y - target[1].Y, 2)) {
		index = nextindex;
	}	
}


float ATrack::CalcRectPosition(const FVector & actorloc, const int index)
{
	FVector t[3]; 
	t[1]= SplinePoints[index];
	if (index != 0)
		t[0]=SplinePoints[index-1];
	else
		t[0] = SplinePoints[n_target-1];

	if(index!=n_target-1)
		t[2]=SplinePoints[index+1];
	else
		t[2]=SplinePoints[0];

	float m, n;
	m = (t[1].Y - Centers[index].Y) / (t[1].X -  Centers[index].X);
	n = Centers[index].Y - m * Centers[index].X;

	bool t1=(actorloc.X*m + n - actorloc.Y) > 0 ? 1 : 0;
	bool t2 = (t[2].X*m + n - t[2].Y) > 0 ? 1 : 0;

	int indexnext;
	if (t1 == t2)
		indexnext = 2;
	else
		indexnext = 0;
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("indexnext:%i"), indexnext));

	float m1, n1, m2, n2;

	m1=(t[1].Y - t[indexnext].Y) / (t[1].X -  t[indexnext].X);
	n1 = t[1].Y - m1 * t[1].X;

	m2=(actorloc.Y - Centers[index].Y) / (actorloc.X -  Centers[index].X);
	n2 = actorloc.Y - m2 * actorloc.X;

	FVector result;
	result.X = ( n2-n1) / (m1 - m2);
	result.Y = result.X*m1 + n1;
	
	//DrawDebugLine(GetWorld(), t[1], t[indexnext], FColor::Blue, false, 1.f, 0,35);

	float proportion;
	int indexnext2 = indexnext * 0.5 - 1;
	if (t[1].X - t[indexnext].X > 0.1) {
		proportion = abs((result.X - t[1+indexnext2].X) / (t[indexnext].X - t[1].X));
	}
	else {
		proportion = abs((result.Y - t[1+indexnext2].Y) / (t[indexnext].Y - t[1].Y));
	}

	float distance;
	indexnext = indexnext*0.5-1;
	distance=AcumulativeDistance[index + indexnext2] + proportion * (AcumulativeDistance[index+indexnext2+1] - AcumulativeDistance[index + indexnext2]);

	float percentage = distance / TotalDistance;
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("percentage:%f"), percentage));
	return percentage;
}

FVector ATrack::CalcCenter()
{
	//UE_LOG(LogTemp,Warning,TEXT("[0]: %s"), *points[0].ToString() );
	//UE_LOG(LogTemp,Warning,TEXT("[1]: %s"), *points[1].ToString() );
	//UE_LOG(LogTemp,Warning,TEXT("[2]: %s"), *points[2].ToString() );

	//int n_p = position % 3 == 2 ? 1 : -1;

	for (int i = 0; i < 3; i++) {
		h_square[i] = points[i].X*points[i].X + points[i].Y*points[i].Y;
	}
	h_divisor = 2 * (points[0].X*(points[1].Y - points[2].Y) - points[0].Y*(points[1].X - points[2].X) + points[1].X*points[2].Y - points[2].X*points[1].Y);

	//if (h_divisor < 0.0001)
		//h_divisor = 0.0001;

	FVector temp;
	temp.X = (h_square[0] * (points[1].Y - points[2].Y) + h_square[1] * (points[2].Y - points[0].Y) + h_square[2] * (points[0].Y - points[1].Y)) /
		h_divisor;

	temp.Y = (h_square[0] * (points[2].X - points[1].X) + h_square[1] * (points[0].X - points[2].X) + h_square[2] * (points[1].X - points[0].X)) /
		h_divisor;

	temp.Z = 0;
	//UE_LOG(LogTemp,Warning,TEXT("center: %s"), *temp.ToString() );
	return temp;
	
}