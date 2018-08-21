// Fill out your copyright notice in the Description page of Project Settings.

#include "Car.h"


// Sets default values
ACar::ACar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	OurVisibleActor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Our mesh"));
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Our collision"));
	RootComponent = OurVisibleActor;

	BoxComponent->SetupAttachment(OurVisibleActor);

	OurVisibleActor->SetSimulatePhysics(true);
	CollisionParams.AddIgnoredActor(this);

	TArray<int> topology = { StickNumber,3,2 };
	nn = NeuralNetwork(topology);

	proportion =  4.f/MaxDistance;

}

// Called when the game starts or when spawned
void ACar::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ACar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<float> Input;
	Input.Reserve(StickNumber);
	FHitResult OutHit;
	FVector start = this->GetActorLocation();
	FRotator actorRot(this->GetActorRotation());
	actorRot.Yaw -= amplitude * 0.5;
	
	float rotation = amplitude / (StickNumber-1);
	for (int i = 0; i < StickNumber; ++i) {
		FVector end(actorRot.Vector() * 2000 + start);
		DrawDebugLine(GetWorld(), start, end, FColor::Green, false, DeltaTime+0.01, 0, 1);
		bool isHit = GetWorld()->LineTraceSingleByObjectType(OutHit, start, end, ECC_WorldStatic, CollisionParams);
		if (isHit && OutHit.bBlockingHit) {
			if (GEngine) {
				float distance = (OutHit.ImpactPoint - start).Size()*proportion;
				Input.Add(distance);
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Distance: %f"), distance));
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Actor: %s"), *OutHit.GetActor()->GetName()));
			}
		}
		else {
			Input.Add(2000*proportion);
		}
		actorRot.Yaw += rotation;
		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::Printf(TEXT("result:%f"),  Input[i]));
	}
	TArray<float> result=nn.forward(Input);
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::Printf(TEXT("right: %f || left: %f"), result[0],result[1]));
	
	SetActorLocation(GetActorLocation() + FVector(DeltaTime*VelocityX, 0.f, 0.f));
	SetActorRotation(GetActorRotation() + FRotator(0.f, 0.2f*(result[0] - result[1]), 0.f));
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

