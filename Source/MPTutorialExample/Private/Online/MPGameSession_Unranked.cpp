#include "MPGameSession_Unranked.h"

void AMPGameSession_Unranked::StartServer()
{
	const IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem && GetWorld()->GetNetMode() == NM_DedicatedServer || GetWorld()->GetNetMode() == NM_ListenServer) /** @note(Chrisr): A unranked (casual) match doesn't have to be on a dedicated server */
	{
		const IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{

		}
	}

	Super::StartServer();
}

void AMPGameSession_Unranked::EndServer()
{
	Super::EndServer();
}