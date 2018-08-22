// Fill out your copyright notice in the Description page of Project Settings.

#include "AI_Controller.h"


// Sets default values
AAI_Controller::AAI_Controller()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	for (int i = 0; i < population; i++) {
		ACar *mycar;
		Cars.Add(mycar);
	}
}

// Called when the game starts or when spawned
void AAI_Controller::BeginPlay()
{
	Super::BeginPlay();
	
	for (int i = 0; i < population; i++) {
		FVector Location = GetActorLocation();
		FRotator Rotation(0.0f, 0.0f, 0.0f);
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		SpawnInfo.Instigator = Instigator;

		Cars[i]=GetWorld()->SpawnActor<ACar>(OurSpawningObject,Location, Rotation, SpawnInfo);

	}
 
}

// Called every frame
void AAI_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	for (int i = 0; i < Cars.Num() ; i++) {
		if (Cars[i]->hit) {
			//Cars[i]->Destroy();
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Destroyable")));
			//Cars.RemoveAt(i);
		}
	}
}

