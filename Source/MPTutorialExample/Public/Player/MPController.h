#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MPController.generated.h"

UCLASS()
class AMPController : public APlayerController
{
	GENERATED_BODY()
	
public:

	AMPController(){}
	virtual ~AMPController(){}
	

	UFUNCTION(Reliable, Client)
	void ClientStartGame();

protected:
private:
	
	FTimerHandle WaitForPlayerState_TimerHandle;

};