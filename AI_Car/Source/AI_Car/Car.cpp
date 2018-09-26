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

	OurCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	OurCameraSpringArm->SetupAttachment(RootComponent);
	OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
	OurCameraSpringArm->TargetArmLength = 400.f;
	OurCameraSpringArm->bEnableCameraLag = true;
	OurCameraSpringArm->CameraLagSpeed = 0.0f;
	
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	OurCamera->SetupAttachment(OurCameraSpringArm, USpringArmComponent::SocketName);

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

	NNproportion =  4.f/MaxDistance;

	//AutoPossessPlayer = EAutoReceiveInput::Player0;
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

	if (ActualVelocity<VelocityX) {
		time += DeltaTime;
		ActualVelocity = time * Aceleration;
	}
		
	if (this->isPossessed) {
		result[0] = RotationPlayer;// *SmoothInput + (1.f - SmoothInput)*result[!actual][0];
	}
	else {
		result = nn.forward(Input);
	}

	
	UpdateStick();
	UpdateRotation();
	UpdateLocation();
	UpdateCameraRotation();
	UpdateStickRotation();
	UpdateStickLength();
	

}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	InputComponent->BindAxis("CameraPitch", this, &ACar::Input_CameraPitch);
	InputComponent->BindAxis("CameraYaw", this, &ACar::Input_CameraYaw);
	InputComponent->BindAxis("StickPitch", this, &ACar::Input_StickPitch);
	InputComponent->BindAxis("Turn", this, &ACar::Input_StickYaw);
	InputComponent->BindAxis("Approach", this, &ACar::Input_Approach);
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

void ACar::ResetMovement(FTransform transform,int initTarget)
{
	this->SetActorTransform(transform);
	this->hit = false;
	this->lastTarget = initTarget;
	this->laps = 0;
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

void ACar::InitNet(TArray<int> topology)
{

	result.Init(0, topology.Last());
	nn.Init(topology);
	Input.Init(0, topology[0]);
}

void ACar::StartPossessing()
{
	GetController()->Possess(this);
}

void ACar::StopPossessing()
{
	GetController()->UnPossess();
}

void ACar::UpdateStick()
{
	if (Input.Num() == 0)
		return;
	FHitResult OutHit;
	FVector start = this->GetActorLocation();
	FRotator actorRot(this->GetActorRotation());
	actorRot.Yaw -= amplitude * 0.5;
	
	float stickrotation = amplitude / (StickNumber-1);
	for (int i = 0; i < StickNumber; ++i) {
		FVector end(actorRot.Vector() * 2000 + start);
		if(drawLine)
			DrawDebugLine(GetWorld(), start, end, FColor::Green, false, deltatime+0.01, 0, 1);
		bool isHit = GetWorld()->LineTraceSingleByObjectType(OutHit, start, end, ECC_WorldStatic, CollisionParams);
		if (isHit && OutHit.bBlockingHit) {
			if (GEngine) {
				float distance = (OutHit.ImpactPoint - start).Size();
				Input[i]=distance;
				//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Distance: %f"), distance));
				//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("Actor: %s"), *OutHit.GetActor()->GetName()));
			}
		}
		else {
			Input[i]=MaxDistance;
		}
		if (!isPossessed)
			Input[i] *= NNproportion;
		actorRot.Yaw += stickrotation;
		//GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Blue, FString::Printf(TEXT("result: %f"),  Input[i]));
	}
	
	
}

void ACar::UpdateLocation()
{
	SetActorLocation(GetActorForwardVector()* (deltatime*ActualVelocity) + GetActorLocation());
}

void ACar::UpdateRotation()
{
	
	//GEngine->AddOnScreenDebugMessage(-1, deltatime, FColor::Blue, FString::Printf(TEXT("rotation: %f"), result[0]));
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw + RotationVelocityPawn * deltatime*(result[0]), 0.f));
}

void ACar::UpdateCameraRotation()
{
	FRotator CameraRotation = OurCamera->GetComponentRotation();
	CameraRotation.Pitch += CameraInput.Y*camera_velocity;
	CameraRotation.Yaw += CameraInput.X*camera_velocity;
	OurCamera->SetWorldRotation(CameraRotation);
}

void ACar::UpdateStickRotation()
{
	FRotator StickRotation = OurCameraSpringArm->GetComponentRotation();
	StickRotation.Pitch += StickInput.Y*stick_velocity;
	StickRotation.Yaw += StickInput.X*stick_velocity;
	OurCameraSpringArm->SetWorldRotation(StickRotation);
}

void ACar::UpdateStickLength()
{
	OurCameraSpringArm->TargetArmLength += approach_velocity*StickDistance;
}

void ACar::Input_Approach(float AxisValue)
{
	StickDistance = FMath::Clamp<float>(AxisValue,-1.0f,1.f);
}

void ACar::Input_CameraPitch(float AxisValue) 
{
	CameraInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACar::Input_CameraYaw(float AxisValue)
{
	CameraInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACar::Input_StickYaw(float AxisValue)
{
	RotationPlayer = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACar::Input_StickPitch(float AxisValue)
{
	StickInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

