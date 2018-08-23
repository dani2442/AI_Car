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
	/*
	for (int i = 0; i < population; i++) {
		FVector Location = GetActorLocation();
		FRotator Rotation(0.f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = Instigator;

		Cars.Add(GetWorld()->SpawnActor<ACar>(Location, Rotation, SpawnInfo));

	}
	*/
	this->Initialize();
}

// Called every frame
void AAI_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	for (int i = 0; i < Cars.Num() ; i++) {
		if (Cars[i]->hit) {
			if (Cars.Num() == 1) {
				Initialize(true);
			}
			Cars[i]->K2_DestroyActor();
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("number:%i"), Cars.Num()));
			Cars.RemoveAt(i);
		}
	}
	
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
	for (int i = newobjects; i < population; i++) {
		Cars.Add(GetWorld()->SpawnActorDeferred<ACar>(ACar::StaticClass(),FTransform::Identity,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	}

	if (learn) {
		Learning();
	}

	for (int i = newobjects; i < population; i++) {
		Cars[i]->FinishSpawning(transform);
	}
	Cars[0]->SetActorTransform(transform);
}

void AAI_Controller::Learning()
{
	for (int i = 1; i < population;i++) { // TODO first part
		Cars[i]->nn = Cars[0]->nn;
		Cars[i]->Change();
	}
}


