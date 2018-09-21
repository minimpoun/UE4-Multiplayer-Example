#include "MPController.h"
#include "MPPlayerState.h"
#include "Online.h"
#include "Net/UnrealNetwork.h"

void AMPController::ClientStartGame_Implementation()
{
	AMPPlayerState* _PlayerState = Cast<AMPPlayerState>(PlayerState);
	if (_PlayerState)
	{
		GetWorld()->GetTimerManager().ClearTimer(WaitForPlayerState_TimerHandle);
		IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
		if (_Subsystem)
		{
			IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
			if (_Session.IsValid() && (_Session->GetNamedSession(_PlayerState->SessionName)))
			{
				_Session->StartSession(_PlayerState->SessionName);
			}
		}
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(WaitForPlayerState_TimerHandle, this, &ThisClass::ClientStartGame_Implementation, 0.1f, false);
	}
}

void AMPController::ClientEndGame_Implementation()
{
	if (IsPrimaryPlayer())
	{
		auto* _PlayerState = Cast<AMPPlayerState>(PlayerState);
		if (_PlayerState)
		{
			IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
			if (_Subsystem)
			{
				IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
				if (_Session.IsValid() && _Session->GetNamedSession(_PlayerState->SessionName))
				{
					_Session->EndSession(_PlayerState->SessionName);
				}
			}
		}
	}
}

void AMPController::ClientNotifyGameStarted_Implementation()
{

}

void AMPController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
