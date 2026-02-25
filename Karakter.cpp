#include "Karakter.h"

AKarakter::AKarakter()
{
	PrimaryActorTick.bCanEverTick = true;

	hiz = 500;
}

void AKarakter::BeginPlay()
{
	Super::BeginPlay();
}

void AKarakter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

