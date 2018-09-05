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

	this->Initialize();
	Cars[0]->nn.Load(RelativePath+"NNdata/dani.json");
}



// Called every frame
void AAI_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
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


