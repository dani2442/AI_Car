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

	// Init Score of each population
	this->Score.Init(0, population);

	// Init Neural Network of selected
	int n_selected = (int)(population*population_selection);
	for (int i = 0; i < n_selected; i++)
		selections.Add(NeuralNetwork());

	// Init positions
	for (int i = 0; i < population; i++)
		position.Add(i);

	// Init topology
	topology.Add(InputLayer);
	for (auto&i : HiddenLayer)
		topology.Add(i);
	topology.Add(OutputLayer);

	importance_diversity2 = 1 / (importance_diversity + 1);
	this->Initialize(); // Spawn objects
	Cars[0]->nn.Load(RelativePath+"NNdata/dani.json");
}



// Called every frame
void AAI_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	delta = DeltaTime;
	sumDelta += DeltaTime;

	if (sumDelta > 0.3) {
		RefreshCarPosition();
	}
	CheckHit();
	
	
}

void AAI_Controller::Initialize(bool learn)
{
	FVector Location = GetActorLocation();
	this->init_target = OurTrack->CalcNearestPoint(Location);
	FRotator Rotation(0.f, 0.0f, 0.0f);
	FTransform transform(Rotation, Location);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;
	int newobjects = Cars.Num();
	
	for (int i = 0; i < population; i++) {
		Cars.Add(GetWorld()->SpawnActorDeferred<ACar>(OurSpawningObject,FTransform::Identity,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	}

	for (int i = 0; i < population; i++) {
		Cars[i]->lastTarget=init_target;
		Cars[i]->InitNet(topology);
		if (i >= show_cars) {
			Cars[i]->OurVisibleActor->SetVisibility(false);
		}
		Cars[i]->FinishSpawning(transform);
	}
	//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("first number:%f"), best.NN[0][0][0]));
}

void AAI_Controller::ReInitialize()
{
	Probability();
	GeneticAlgorithm();
	for (int i = 0; i < population; i++) {
		Cars[i]->ResetMovement(GetActorTransform(),init_target);
	}
}

void AAI_Controller::Probability()
{
	CalcFitness();
	CalcDiversity();

	Score[0] = (Cars[0]->fitness + Cars[0]->diversity);
	for (int i = 1; i < Score.Num(); i++) {
		Score[i] = Score[i - 1] + (Cars[i]->fitness +  Cars[i]->diversity);
		UE_LOG(LogTemp,Warning,TEXT("scores[%i]: %f + %f => %f"),i, Cars[i]->fitness,Cars[i]->diversity,Score[i]);
	}
}

void AAI_Controller::CalcPosition()
{
	int k;
	for (int i = 0; i < show_cars; i++) {
		for (int j = i + 1; j < population; j++) {
			if (Cars[position[i]]->percentage < Cars[position[j]]->percentage) {
				//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), j));
				k = position[i];
				position[i] = position[j];
				position[j] = k;
			}
		}
	}
}

void AAI_Controller::CheckHit() {
	int Death=0;
	for (int i = 0; i < Cars.Num() ; i++) {
		if (Cars[i]->hit) {
			Death++;
		}
	}
	if (Death == population) {
		ReInitialize();
	}
}

void AAI_Controller::RefreshCarPosition()
{
	int count=0;
	for (int i = 0; i < Cars.Num(); i++) {
		if (!Cars[i]->hit) {
			OurTrack->UpdatePoint(Cars[i]->GetActorLocation(), Cars[i]->lastTarget);
			GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("Car #%i target: %i"), i, Cars[i]->lastTarget));
			Cars[i]->percentage = OurTrack->CalcRectPosition(Cars[i]->GetActorLocation(), Cars[i]->lastTarget);// last implementation
			GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("distance: %f / %f"), Cars[i]->percentage*OurTrack->TotalDistance, OurTrack->TotalDistance));
		}

		// Draw line and show mesh only if the car is between the 6 firsts
		if (i<show_cars) {
			Cars[position[i]]->OurVisibleActor->SetVisibility(true);
			Cars[position[i]]->drawLine = true;
			//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), position[count]));
		}
		else {
			Cars[position[i]]->drawLine = false;
			Cars[position[i]]->OurVisibleActor->SetVisibility(false);
		}
		
	}
	CalcPosition();
	
}

void AAI_Controller::CalcDiversity()
{
	this->TotalDiversity = 0;
	for(int i=0;i<population;i++)
		Cars[i]->diversity = 0;

	float t = 0;
	for (int i = 0; i < population; i++) {
		for (int j = i+1; j < population; j++) {
			for (int x = 0; x < topology.Num()-1;x++) {
				for (int y = 0; y < topology[x];y++) {
					for (int z = 0; z < topology[x+1];z++) {
						float diversity=abs(Cars[j]->nn.NN[x][y][z]-Cars[i]->nn.NN[x][y][z]);
						Cars[i]->diversity += diversity;
						Cars[j]->diversity += diversity;
					}
				}
			}
		}
		if (Cars[i]->percentage == 0) // Remove the ones that does not reach the deadline
			Cars[i]->diversity = 0;
		TotalDiversity += Cars[i]->diversity;
	}
	TotalDiversity = 1 / TotalDiversity * importance_diversity;
	for (int i = 0; i < population; i++) {
		Cars[i]->diversity *= TotalDiversity;
	}
}

void AAI_Controller::CalcFitness()
{
	RefreshCarPosition();
	this->TotalFitness = 0;
	float avarage=0;
	for (int i = 0; i < population; i++) {
		avarage += Cars[i]->percentage;
	}
	avarage = avarage * deadline / population;
	for (int i = 0; i < population; i++) {
		if (Cars[i]->percentage < avarage)// Remove the ones which are the worst
			Cars[i]->percentage = 0;
		TotalFitness += Cars[i]->percentage;
	}
	TotalFitness = 1 / TotalFitness*(1-importance_diversity);
	for (int i = 0; i < population; i++) {
		Cars[i]->fitness = Cars[i]->percentage * TotalFitness;
	}
}

void AAI_Controller::GeneticAlgorithm()
{
	GA_Selection();
	GA_Crossover();
	GA_Mutation();
}

void AAI_Controller::GA_Selection()
{	
	// Select the best
	float last_best=10000000.f; int last_best_index;
	for (int i = 0; i < Score.Num(); i++) {
		if (Score[i] < last_best) {
			last_best = Score[i];
			last_best_index = i;
		}
	}
	selections[0].NN = Cars[last_best_index]->nn.NN;

	float n;
	for (int i = 1; i < selections.Num();i++) {
		n = FMath::FRand();
		for (int j = 0; j < Score.Num(); j++) {
			if ( n < Score[j]) {
				selections[i].NN = Cars[j]->nn.NN;
				break;
			}
		}
	}

	Cars[0]->nn.NN = selections[0].NN;
	for (int i = 1; i < selections.Num()*2; i++) {
		Cars[i]->nn.NN = selections[i%selections.Num()].NN;
	}
}

void AAI_Controller::GA_Crossover()
{
	if (!crossover)
		return;

	int a, b, c;
	for (int i = selections.Num()*2; i < population; i++) {
		a = FMath::RandRange(0, selections.Num()-1);
		b = FMath::RandRange(0, selections.Num()-1);
		c = FMath::RandRange(0, population - 1);

		for (int x = 0; x < topology.Num()-1;x++) {
			for (int y = 0; y < topology[x];y++) {
				for (int z = 0; z < topology[x+1];z++) {
					if (FMath::FRand()>crossover_rate) {
						Cars[i]->nn.NN[x][y][z] = selections[a].NN[x][y][z];
					}
					else {
						Cars[i]->nn.NN[x][y][z] = selections[b].NN[x][y][z];
					}
				}
			}
		}
	}
}

void AAI_Controller::GA_Mutation()
{
	for (int i = selections.Num(); i < population; i++) {
		for (auto& a : Cars[i]->nn.NN) {
			for (auto&b : a) {
				for (auto&c : b) {
					if(FMath::FRand()<mutation_rate)
						c += (FMath::FRand() * 2 - 1)*mutation_change;
				}
			}
		}
	}
}
