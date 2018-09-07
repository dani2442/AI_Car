// Fill out your copyright notice in the Description page of Project Settings.

#include "Car.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"

// Sets default values
ACar::ACar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	OurVisibleActor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Our mesh"));
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Our collision"));
	Component = CreateDefaultSubobject<UBoxComponent>(TEXT("Our box"));
	RootComponent = Component;

	OurVisibleActor->SetupAttachment(RootComponent);
	//Component->SetupAttachment(RootComponent);
	BoxComponent->SetupAttachment(RootComponent);

	BoxComponent->SetNotifyRigidBodyCollision(true);
	BoxComponent->BodyInstance.SetCollisionProfileName("Car");
	BoxComponent->OnComponentHit.AddDynamic(this, &ACar::OnCompHit);
	BoxComponent->SetRelativeScale3D(FVector(1.75, 1.5, 0.5));
	BoxComponent->SetSimulatePhysics(false);

	CollisionParams.AddIgnoredActor(this);

	Component->SetSimulatePhysics(true);
	Component->BodyInstance.SetCollisionProfileName("Car");
	OurVisibleActor->BodyInstance.SetCollisionProfileName("NoCollision");
	//auto MeshAsset= ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	//UStaticMesh* Asset = MeshAsset.Object;
	//OurVisibleActor->SetStaticMesh(Asset);

	TArray<int> topology = { StickNumber,3,2 };
	nn = NeuralNetwork(topology);

	NNproportion =  4.f/MaxDistance;

	Input.Init(0, StickNumber);

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

	deltatime = DeltaTime;
	UpdateStick();
	UpdateRotation();
	UpdateLocation();

	
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACar::OnCompHit(UPrimitiveComponent * HitComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, FVector NormalImpulse, const FHitResult & Hit)
{
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL)) {
		if (GEngine) {
			//GEngine->AddOnScreenDebugMessage(-1,1.f, FColor::Green, FString::Printf(TEXT("I Just hit:%s"), *OtherActor->GetName()));
			//this->Destroy();
			hit = true;
			this->SetActorTickEnabled(false);
		}
	}
}

void ACar::Reset(FTransform transform,int initTarget)
{
	this->SetActorTransform(transform);
	this->hit = false;
	this->lastTarget = initTarget;
	this->SetActorTickEnabled(true);
}

void ACar::Change() {
	for (auto& a : nn.NN) {
		for (auto&b : a) {
			for (auto&c : b) {
				c += (FMath::FRand() * 2 - 1)*0.4;
			}
		}
	}
}

void ACar::UpdateStick()
{
	Input.Reserve(StickNumber);
	FHitResult OutHit;
	FVector start = this->GetActorLocation();
	FRotator actorRot(this->GetActorRotation());
	actorRot.Yaw -= amplitude * 0.5;
	
	float stickrotation = amplitude / (StickNumber-1);
	for (int i = 0; i < StickNumber; ++i) {
		FVector end(actorRot.Vector() * 2000 + start);
		DrawDebugLine(GetWorld(), start, end, FColor::Green, false, deltatime+0.01, 0, 1);
		bool isHit = GetWorld()->LineTraceSingleByObjectType(OutHit, start, end, ECC_WorldStatic, CollisionParams);
		if (isHit && OutHit.bBlockingHit) {
			if (GEngine) {
				float distance = (OutHit.ImpactPoint - start).Size()*NNproportion;
				Input[i]=distance;
				//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Distance: %f"), distance));
				//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Actor: %s"), *OutHit.GetActor()->GetName()));
			}
		}
		else {
			Input[i]=MaxDistance*NNproportion;
		}
		actorRot.Yaw += stickrotation;
		//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::Printf(TEXT("result: %f"),  Input[i]));
	}
	
	//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::Printf(TEXT("rotation: %f"), result[0] - result[1]));
}

void ACar::UpdateLocation()
{
	SetActorLocation(GetActorForwardVector()* (deltatime*VelocityX) + GetActorLocation());
}

void ACar::UpdateRotation()
{
	TArray<float> result=nn.forward(Input);
	SetActorRotation(FRotator(0.f,GetActorRotation().Yaw + RotationVelocityPawn * deltatime*(result[0] - result[1]),0.f));
}
