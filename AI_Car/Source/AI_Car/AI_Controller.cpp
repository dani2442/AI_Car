// Fill out your copyright notice in the Description page of Project Settings.


#include "AI_Controller.h"


// Sets default values
AAI_Controller::AAI_Controller()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

// Called when the game starts or when spawned
void AAI_Controller::BeginPlay()
{
	Super::BeginPlay();

	InitLine();

	this->Initialize(); // Spawn objects
	Cars[0]->nn.Load(RelativePath+"NNdata/dani.json");


}



// Called every frame
void AAI_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	CheckHit();
	
}

void AAI_Controller::Initialize(bool learn)
{
	FVector Location = GetActorLocation();
	FRotator Rotation(0.f, 0.0f, 0.0f);
	FTransform transform(Rotation, Location);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;
	int newobjects = Cars.Num();
	
	for (int i = 0; i < population; i++) {
		Cars.Add(GetWorld()->SpawnActorDeferred<ACar>(OurSpawningObject,FTransform::Identity,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	}

	if (learn) {
		Learning();
	}

	for (int i = 0; i < population; i++) {
		if (i >= show_cars) {
			Cars[i]->OurVisibleActor->SetVisibility(false);
		}
		Cars[i]->FinishSpawning(transform);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("first number:%f"), best.NN[0][0][0]));
}

void AAI_Controller::Learning()
{
	Cars[0]->nn = best;
	for (int i = 1; i < population;i++) { // TODO first part
		Cars[i]->nn = best;
		Cars[i]->Change();
	}
}


void AAI_Controller::CheckHit() {
	for (int i = 0; i < Cars.Num() ; i++) {
		if (Cars[i]->hit) {
			if (Cars.Num() == 1) {
				Cars[i]->best = true;
				Cars[i]->nn.Write(RelativePath+"NNdata/dani.json");
				best = Cars[i]->nn;
				Cars[i]->Destroy();
				Cars.RemoveAt(i);
				Initialize(true);
				//break;
			}
			else {

				Cars[i]->Destroy();
				Cars.RemoveAt(i);
				//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("number:%i"), Cars.Num()));
			}
		}
	}
}

void AAI_Controller::InitLine()
{
	FVector carloc = GetActorLocation();
	FVector target[2];
	float distance = 0.f, distance2 = 1000000000.f,newdistance;
	int point=0;
	bool state = true;

	if (OurTrack->FinishOnStart) {
		target[!state]=OurTrack->PathSpline->GetWorldLocationAtSplinePoint(OurTrack->n_target);
	}
	else {
		target[!state]=OurTrack->PathSpline->GetWorldLocationAtSplinePoint(0);
	}
	for (int i = !OurTrack->FinishOnStart; i < OurTrack->n_target; i++) {
		target[state]=OurTrack->PathSpline->GetWorldLocationAtSplinePoint(i);

		newdistance = pow(carloc.X - target[state].X, 2) + pow(carloc.Y - target[state].Y, 2);
		if (newdistance < distance2) {
			distance2 = newdistance;
			point = i;
		}

		distance += sqrt(pow(target[0].X - target[1].X, 2) + pow(target[0].Y - target[1].Y, 2));
		state = !state;
	}
	this->current_target = point;
	this->circuit_ditance = distance;

	GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Green, FString::Printf(TEXT("distance total:%f"), distance));
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("nearest point:%i"), point));
	
}

void AAI_Controller::PercentageRace()
{

}
