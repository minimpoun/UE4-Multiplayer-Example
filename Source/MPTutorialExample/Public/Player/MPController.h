#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MPController.generated.h"

class UMPGameMenu;

UCLASS()
class AMPController : public APlayerController
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true, DisplayName = "GameMenuWidget"))
	TSubclassOf<UMPGameMenu> W_GameMenuWidget;

	UMPGameMenu* GameMenuWidget;

public:

	AMPController(){}
	virtual ~AMPController(){}
	

	/**
	 *	Called when a game starts after a player has already joined
	 **/
	UFUNCTION(Reliable, Client)
	void ClientStartGame();

	/**
	 *	Called when a player is in a game that ends
	 **/
	UFUNCTION(Reliable, Client)
	void ClientEndGame();

	/**
	 *	Called when a player joins a game that has already started
	 **/
	UFUNCTION(Reliable, Client)
	void ClientNotifyGameStarted();

protected:

	/** 
	 * Used to disable all input outside of basic movement.
	 * Ex: Overwatch post-match
	 **/
	UPROPERTY(BlueprintReadWrite)
	uint8 bAllowGameplay:1;

	/** 
	 * The location of the player when they died. 
	 * Used for when the player kills someone after death, and for setting up the KillCam
	 **/
	UPROPERTY(BlueprintReadOnly)
	FVector DeathLocation;

	/** Begin AActor Interface */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const override;
	/** End AActor Interface */

protected:

	/************************************************************************/
	/*								Cheats                                  */
	/*vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

	UPROPERTY(Replicated, Transient)
	uint8 bNoReload:1;

	UPROPERTY(Replicated, Transient)
	uint8 bUnlimitedAmmo:1;

	UPROPERTY(Replicated, Transient)
	uint8 bGodMode:1;

	UPROPERTY(Replicated, Transient)
	uint8 bNoRecoil:1;

	/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
	/*								Cheats                                  */
	/************************************************************************/

private:
	
	FTimerHandle WaitForPlayerState_TimerHandle;

};