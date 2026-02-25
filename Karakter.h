#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Karakter.generated.h"

UCLASS()
class AKarakter : public AActor
{
	GENERATED_BODY()

public:
	AKarakter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KokGenerated")
	int32 hiz = 500;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

};
