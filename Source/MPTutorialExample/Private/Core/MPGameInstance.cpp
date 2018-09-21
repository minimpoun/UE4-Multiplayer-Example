#include "MPGameInstance.h"
#include "MPGameSession.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "MPBaseGameMode.h"

DEFINE_LOG_CATEGORY(LogGameInstance);

namespace FGameNetworkState
{
	const FName None = FName(TEXT("None"));
	const FName MainMenu = FName(TEXT("MainMenu"));
	const FName WelcomeMenu = FName(TEXT("WelcomeMenu"));
	const FName Playing = FName(TEXT("Playing"));
	const FName RetryingConnection = FName(TEXT("RetryingConnection"));
	const FName Transition = FName(TEXT("Transition"));
}

bool UMPGameInstance::HostListenServer(ULocalPlayer* OwningPlayer, const FString& GameMode, const FString& GameOptions)
{
	if (GetConnectionType() == EOnlineConnectionType::SinglePlayer || GetConnectionType() == EOnlineConnectionType::Offline)
	{
		SetState(FGameNetworkState::Playing);
		GetWorld()->ServerTravel(GameOptions);
		return true;
	}

	auto* const _GS = GetGameSession();
	if (_GS)
	{
		OnCreateSessionCompleteDelegateHandle = _GS->OnCreateListenServerComplete().AddUObject(this, &ThisClass::OnCreateSessionComplete);

		GameURL = GameOptions;

		const FString& _MapFolder = "/Game/Maps/";
		const FString& _MapUrl = GameOptions.RightChop(_MapFolder.Len());
		const FString& _MapName = _MapUrl.LeftChop(_MapUrl.Len() - _MapUrl.Find("?game"));
		const bool _bIsLAN = GameOptions.Contains("?bIsLAN");
		const bool _bAllowLateJoin = GameOptions.Contains("?bAllowLateJoin");
		const int32 _MaxPlayers = UGameplayStatics::GetIntOption(GameOptions, "MaxPlayers", 8);

		Map.ToString() = _MapName;

		if (_GS->HostSession(OwningPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, GameMode, _MapName, _bIsLAN, true, _bAllowLateJoin, _MaxPlayers))
		{
			if (IsStateInTransition() || PendingState == FGameNetworkState::None)
			{
				SetState(FGameNetworkState::Playing);
				return true;
			}

			UE_LOG(LogGameInstance, Warning, TEXT("Warning: Failed to host session with URL %s in %s."), *GameOptions, TEXT(__FUNCTION__));
		}
	}

	return false;
}

bool UMPGameInstance::JoinServer(ULocalPlayer* OwningPlayer, int32 SessionIndex)
{
	auto* const _GS = GetGameSession();
	if (_GS)
	{
		OnJoinSessionCompleteDelegateHandle = _GS->OnJoinSessionsComplete().AddUObject(this, &ThisClass::OnJoinSessionComplete);
		if (_GS->JoinSession(OwningPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, SessionIndex))
		{
			if (IsStateInTransition() || PendingState == FGameNetworkState::None)
			{
				SetState(FGameNetworkState::Playing);
				return true;
			}
		}

		UE_LOG(LogGameInstance, Warning, TEXT("Warning: Failed to join session with index %d in %s."), SessionIndex, TEXT(__FUNCTION__));
	}

	return false;
}

bool UMPGameInstance::JoinServer(ULocalPlayer* OwningPlayer, const FOnlineSessionSearchResult& SearchResult)
{
	auto* const _GS = GetGameSession();
	if (_GS)
	{
		OnJoinSessionCompleteDelegateHandle = _GS->OnJoinSessionsComplete().AddUObject(this, &ThisClass::OnJoinSessionComplete);
		if (_GS->JoinSession(OwningPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, SearchResult))
		{
			if (IsStateInTransition() || PendingState == FGameNetworkState::None)
			{
				SetState(FGameNetworkState::Playing);
				return true;
			}
		}

		UE_LOG(LogGameInstance, Warning, TEXT("Warning: Failed to join session with index %s in %s."), *SearchResult.Session.SessionInfo->ToString(), TEXT(__FUNCTION__));
	}

	return false;
}

bool UMPGameInstance::FindServers(ULocalPlayer* OwningPlayer, const FServerFilter& ServerFilter)
{
	if (OwningPlayer)
	{
		auto* const _GS = GetGameSession();
		if (_GS)
		{
			_GS->OnFindSessionsComplete().RemoveAll(this);
			_GS->FindSessions(OwningPlayer->GetPreferredUniqueNetId().GetUniqueNetId(), GameSessionName, ServerFilter.bSearchLAN, true, ServerFilter.bSearchDedicated, ServerFilter.bAllowCustomGames);

			return true;
		}
	}

	return false;
}

void UMPGameInstance::TravelToSession(const FName& SessionName)
{
	DeferredTravelToSession(SessionName);
}

void UMPGameInstance::BeginQuickHost()
{
	
}

void UMPGameInstance::SetMultiplayerFeaturesForType(EOnlineConnectionType InType)
{

}

void UMPGameInstance::ReturnToMenuWithErrorToast(const FText& Error, const FText& Button)
{
	SwitchStateWithToast(FGameNetworkState::MainMenu, Error, Button, FText::GetEmpty(), true);
}

void UMPGameInstance::ReturnToMenu()
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			FName _SessionName(GameSessionName);
			EOnlineSessionState::Type _SessionState = _Session->GetSessionState(_SessionName);

			switch (_SessionState)
			{				
				case EOnlineSessionState::Ending:
				{
					_Session->EndSession(_SessionName);
				} break;

				case EOnlineSessionState::Ended:
				{
					_Session->DestroySession(_SessionName);
				} break;
			}
		}
	}
}

void UMPGameInstance::ResetState()
{
	CurrentState = FGameNetworkState::WelcomeMenu;
}

void UMPGameInstance::SetState(FName NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	PendingState = NewState;

	if (PendingState == FGameNetworkState::MainMenu)
	{

	}
}

void UMPGameInstance::SwitchStateWithToast(const FName& NewState, const FText& ToastMessage, const FText& ConextButtonLeft, const FText& ContextButtonRight, const bool bOverride /*= false*/, TWeakObjectPtr<ULocalPlayer> OwningPlayer /*= nullptr*/)
{
	
}

void UMPGameInstance::StartGameInstance()
{
	ResetState();
}

void UMPGameInstance::OnCreateSessionComplete(FName SessionName, bool bSuccess)
{
	auto* const _GS = GetGameSession();
	if (_GS)
	{
		_GS->OnCreateListenServerComplete().Remove(OnCreateSessionCompleteDelegateHandle);
		FinalizeServerCreation(bSuccess ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMPGameInstance::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	auto* const _GS = GetGameSession();
	if (_GS)
	{
		_GS->OnJoinSessionsComplete().Remove(OnJoinSessionCompleteDelegateHandle);
	}

	/** @todo(Chrisr): network handling */

	DeferredTravelToSession(GameSessionName);
}

void UMPGameInstance::FinalizeServerCreation(EOnJoinSessionCompleteResult::Type Result)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		GetWorld()->ServerTravel(GameURL);
		return;
	}

	UE_LOG(LogGameInstance, Warning, TEXT(__FUNCTION__));
}

AMPGameSession* UMPGameInstance::GetGameSession() const
{
	if (GetWorld())
	{
		const auto* _GM = GetWorld()->GetAuthGameMode<AMPBaseGameMode>(); check(_GM);
		auto* _GS = Cast<AMPGameSession>(_GM->GameSession);
		return _GS;
	}

	return nullptr;
}

void UMPGameInstance::DeferredTravelToSession(const FName& SessionName)
{
	FText _OkayButton = NSLOCTEXT("DialogOption", "Okay", "Okay");

	auto* _Controller = GetFirstLocalPlayerController();
	if (_Controller)
	{
		IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
		if (_Subsystem)
		{
			IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
			if (_Session.IsValid() && _Session->GetResolvedConnectString(SessionName, GameURL))
			{
				_Controller->ClientTravel(GameURL, TRAVEL_Absolute);
			}

			FText _Error = NSLOCTEXT("NetworkError", "NetworkError_Session", "Failed to travel to session");			
			ReturnToMenuWithErrorToast(_Error, _OkayButton);
			return;
		}

		FText _Error = NSLOCTEXT("NetworkError", "NetworkError_Subsystem", "Failed to connect to online subsystem");
		ReturnToMenuWithErrorToast(_Error, _OkayButton);
		return;
	}

	FText _Error = NSLOCTEXT("NetworkError", "NetworkError_Client", "Failed to initialize client");
	ReturnToMenuWithErrorToast(_Error, _OkayButton);
}