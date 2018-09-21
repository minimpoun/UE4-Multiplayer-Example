#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MPBaseGameMode.generated.h"

class APlayerStart;
class APlayerController;

UCLASS(minimalapi, Abstract)
class AMPBaseGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	
	/** 
	 *	This can only be called by the server, so in the case of a dedicated server Instigator would be nullptr
	 *	
	 *	@Param PlayerToRemove: The player controller owned by the user we want to remove from the server.
	 *	@Param Instigator: The player controller owned by the server
	 **/
	UFUNCTION(BlueprintCallable, Category = "Admin Functions")
	bool RemovePlayer(APlayerController* PlayerToRemove);

	AActor* ChoosePlayerStart_Implementation(AController* Player) override;

protected:

	bool IsValidSpawn(APlayerStart* Spawn, AController* Player) const;
	bool IsPreferredSpawn(APlayerStart* Spawn, AController* Player) const;
};