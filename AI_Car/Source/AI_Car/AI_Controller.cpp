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
	OurPlayer = UGameplayStatics::GetPlayerController(this, 0);

	if (show_cars > population) {
		show_cars = population;
	}

	// Init Score of each population
	this->Score.Init(0, population);

	// Init Neural Network of selected
	int n_selected = (int)(population*population_selection);
	for (int i = 0; i < n_selected; i++)
		selections.Add(NeuralNetwork());

	// Init positions
	for (int i = 0; i < population+player; i++)
		position.Add(i);

	// Init topology
	topology.Add(InputLayer);
	for (auto&i : HiddenLayer)
		topology.Add(i);
	topology.Add(OutputLayer);

	importance_diversity2 = 1 / (importance_diversity + 1);
	this->Initialize(); // Spawn objects

	Cars[0]->nn=JSON_Handler::Load_NN(RelativePath + "NNdata/dani.json");
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("first number:%f"), Cars[0]->nn.NN[0][0][0]));

	if (!player) {
		OurPlayer->UnPossess();
		OurPlayer->Possess(Cars[0]);
	}
}



// Called every frame
void AAI_Controller::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	delta = DeltaTime;
	sumDelta += DeltaTime;

	if (sumDelta > refresh_frecuency) {
		RefreshCarPosition();
		CheckHit();
		if (player) {
			FTrainingData temp;
			TArray<float> output{ Cars[population]->RotationPlayer };
			TArray<float> input = Cars[population]->Input;
			temp.input = input;
			temp.output = output;
			PlayerTrace.Add(temp);
		}
		sumDelta = 0;
	}
	
	
}

void AAI_Controller::Initialize(bool learn)
{
	FVector Location = GetActorLocation();
	this->init_target = OurTrack->CalcNearestPoint(Location);
	FRotator Rotation = GetActorRotation();
	FTransform transform(Rotation, Location);
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = Instigator;

	
	for (int i = 0; i < population; i++) {
		Cars.Add(GetWorld()->SpawnActorDeferred<ACar>(OurSpawningObject, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
	}
	if (player) {
		Cars.Add(GetWorld()->SpawnActorDeferred<ACar>(OurSpawningObject, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn));
		Cars[population]->isPossessed = true;
		OurPlayer->UnPossess();
		OurPlayer->Possess(Cars[population]);
		Cars[population]->lastTarget = init_target;
		Cars[population]->InitNet(topology);
		Cars[population]->OurVisibleActor->SetVisibility(true);
		Cars[population]->FinishSpawning(transform);
	}

	for (int i = 0; i < population; i++) {
		Cars[i]->lastTarget = init_target;
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
	for (int i = 0; i < population+player; i++) {
		Cars[i]->ResetMovement(GetActorTransform(), init_target);
	}
	if (player) {
		JSON_Handler::Write_Training(PlayerTrace, RelativePath + "NNdata/training.json");
	}
}

void AAI_Controller::Probability()
{
	CalcFitness();
	CalcDiversity();
	if (population == 0)
		return;
	Score[0] = (Cars[0]->fitness + Cars[0]->diversity);
	for (int i = 1; i < Score.Num(); i++) {
		Score[i] = Score[i - 1] + (Cars[i]->fitness +  Cars[i]->diversity);
		UE_LOG(LogTemp,Warning,TEXT("scores[%i]: %f + %f => %f"),i, Cars[i]->fitness,Cars[i]->diversity,Score[i]);
	}
}

void AAI_Controller::CalcPosition()
{
	if (Cars.Num() == 0)
		return;
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
	int Death = 0;
	int finished=0;
	for (int i = 0; i < population+player; i++) {
		if (Cars[i]->hit) {
			Death++;
		}
		else {
			if (Cars[i]->laps > this->laps) {
				finished++;
			}
		}
	}
	if (finished == population + (int)player - Death) {
		ReInitialize();
		GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("Winner: #%i"), position[0]));
		GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("population: %i || death: %i || finished: %i"), population,Death,finished));
		
		// TODO winner and position
	}
	if (Death == population+player) {
		ReInitialize();
	}
}

void AAI_Controller::RefreshCarPosition()
{
	int count=0;
	for (int i = 0; i < population+player; i++) {
		if (!Cars[i]->hit) {
			if (OurTrack->UpdatePoint(Cars[i]->GetActorLocation(), Cars[i]->lastTarget)) {
				Cars[i]->laps++;
				if (Cars[i]->laps > this->laps)
					this->laps++;
			}
			GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("Car #%i target: %i"), i, Cars[i]->lastTarget));
			Cars[i]->percentage =Cars[i]->laps+ OurTrack->CalcRectPosition(Cars[i]->GetActorLocation(), Cars[i]->lastTarget);// last implementation
			//GEngine->AddOnScreenDebugMessage(-1, refresh_frecuency, FColor::Green, FString::Printf(TEXT("distance: %f / %f"), Cars[i]->percentage*OurTrack->TotalDistance, OurTrack->TotalDistance));
		}

		// Draw line and show mesh only if the car is between the 6 firsts
		if(i!=population || player!=1){
			if (i < show_cars) {
				Cars[position[i]]->OurVisibleActor->SetVisibility(true);
				Cars[position[i]]->drawLine = true;
				//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), position[count]));
			}
			else {

				Cars[position[i]]->drawLine = false;
				Cars[position[i]]->OurVisibleActor->SetVisibility(false);
			}
		}
	}
	CalcPosition();
	if (!player) {
		if (last_best != position[count]) {
			OurPlayer->SetViewTargetWithBlend(Cars[position[count]], 0.4f);
			OurPlayer->UnPossess();
			OurPlayer->Possess(Cars[position[count]]);
			last_best = position[count];

			//Cars[position[count]]->OurCamera->Activate();
			//Cars[last_best]->StopPossessing();
			//Cars[position[count]]->StartPossessing();
		}
	}
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
	if (selections.Num() == 0)
		return;
	GA_Selection();
	GA_Crossover();
	GA_Mutation();
	UE_LOG(LogTemp,Warning,TEXT("first number:%f"),Cars[0]->nn.NN[0][0][0]);
}

void AAI_Controller::GA_Selection()
{
	if (population == 0)
		return;
	// Select the best
	int k = 0;
	for (int j =  1; j < population; j++) {
		if (Cars[position[0]]->percentage < Cars[position[j]]->percentage) {
			//GEngine->AddOnScreenDebugMessage(-1, delta, FColor::Green, FString::Printf(TEXT("shown %i"), j));
			k = position[0];
			position[0] = position[j];
			position[j] = k;
		}
	}
	
	if (position[0] == population) {
		selections[0].NN = Cars[position[1]]->nn.NN;
		UE_LOG(LogTemp,Warning,TEXT("car_selected__:%i"),position[1]);
		JSON_Handler::Write_NN(Cars[position[1]]->nn, RelativePath + "NNdata/dani.json");
	}else {
		selections[0].NN = Cars[position[0]]->nn.NN;
		UE_LOG(LogTemp,Warning,TEXT("car_selected:%i"),position[0]);
		JSON_Handler::Write_NN(Cars[position[0]]->nn,RelativePath+"NNdata/dani.json");
	}

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
