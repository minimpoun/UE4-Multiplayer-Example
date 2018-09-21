#include "MPBaseGameMode.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Engine/PlayerStartPIE.h"

bool AMPBaseGameMode::RemovePlayer(APlayerController* PlayerToRemove)
{
	return false;
}

AActor* AMPBaseGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> _PreferredSpawns, _DefaultSpawns;
	APlayerStart* _ChosenStart = nullptr;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* _Spawn = *It;
		if (_Spawn)
		{
			if (_Spawn->IsA<APlayerStartPIE>())
			{
				_ChosenStart = _Spawn;
				break;
			}
			else
			{
				if (IsValidSpawn(_Spawn, Player))
				{
					if (IsPreferredSpawn(_Spawn, Player))
					{
						_PreferredSpawns.Add(_Spawn);
					}
					else
					{
						_DefaultSpawns.Add(_Spawn);
					}
				}
			}
		}
	}

	if (!_ChosenStart)
	{
		if (_PreferredSpawns.Num())
		{
			_ChosenStart = _PreferredSpawns[FMath::RandHelper(_PreferredSpawns.Num())];
		}
		else if (_DefaultSpawns.Num())
		{
			_ChosenStart = _DefaultSpawns[FMath::RandHelper(_DefaultSpawns.Num())];
		}
	}

	return _ChosenStart ? _ChosenStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool AMPBaseGameMode::IsValidSpawn(APlayerStart* Spawn, AController* Player) const
{
	return true;
}

bool AMPBaseGameMode::IsPreferredSpawn(APlayerStart* Spawn, AController* Player) const
{
	return true;
}
