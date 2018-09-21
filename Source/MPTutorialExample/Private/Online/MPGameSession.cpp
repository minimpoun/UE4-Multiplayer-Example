#include "MPGameSession.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "MPBaseGameMode.h"
#include "MPController.h"
#include "OnlineSubsystemSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "OnlineSessionSettings.h"

DEFINE_LOG_CATEGORY(LogSessions);

#define MATCH_TIMEOUT 120.f

AMPGameSession::AMPGameSession()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OnCreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete);
		OnStartSessionCompleteDelegate = FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete);
		OnDestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete);
		OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete);
		OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete);
	}

	MaxPlayers_Dedicated = 16;
}

void AMPGameSession::RequestSafeShutdown(int32 ExitCode)
{
	UE_LOG(LogSessions, Log, TEXT("%s has been flagged for shutdown. Exiting with code: %i"), TEXT(__FUNCTION__), ExitCode);

	const UWorld* _World = GetWorld();
	AGameModeBase* _GM = _World->GetAuthGameMode(); check(_GM)

	UE_LOG(LogSessions, Log, TEXT("Attempting to shutdown"));
	
	if (_GM->GetNumPlayers() == 0)
	{
		UE_LOG(LogSessions, Log, TEXT("No active players in session, and none waiting to join. Shutting down..."));

	}
}

void AMPGameSession::StartMatchmaking()
{

}

void AMPGameSession::CancelMatchmaking()
{

}

int32 AMPGameSession::GetBestSession()
{
	for (int32 _Index = CurrentSession.SessionIndex + 1; _Index < SearchSettings->SearchResults.Num(); ++_Index)
	{
		
	}

	return NULL;
}

bool AMPGameSession::HostSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, const FString& Gamemode, const FString& Map, bool bIsLAN, bool bIsPresence, bool bAllowJoinInProgress, int32 MaxPlayers)
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && UserID.IsValid())
		{
			CurrentSession.User = UserID;
			CurrentSession.bIsPresence = bIsPresence;
			CurrentSession.bIsLAN = bIsLAN;
			CurrentSession.bIsDedicated = false;
			CurrentSession.SessionName = SessionName;

			HostSettings = MakeShareable(new FMPOnlineSessionSettings(bIsLAN, bIsPresence, bAllowJoinInProgress, MaxPlayers));
			HostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			HostSettings->Set(SETTING_GAMEMODE, Gamemode, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MAPNAME, Map, EOnlineDataAdvertisementType::ViaOnlineService);
			HostSettings->Set(SETTING_MATCHING_TIMEOUT, MATCH_TIMEOUT, EOnlineDataAdvertisementType::ViaOnlineService);

			OnCreateSessionCompleteDelegateHandle = _Session->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			return _Session->CreateSession(*UserID, CurrentSession.SessionName, *HostSettings);
		}
	}
	else
	{
#if !UE_BUILD_SHIPPING

		OnCreateListenServerComplete().Broadcast(NAME_GameSession, true);

#endif
	}

	return false;
}

bool AMPGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, int32 _SessionIndex)
{
	bool _bSuccess = false;

	if (_SessionIndex >= 0 && _SessionIndex < SearchSettings->SearchResults.Num())
	{
		_bSuccess = JoinSession(UserID, SessionName, SearchSettings->SearchResults[_SessionIndex]);
	}

	return _bSuccess;
}

bool AMPGameSession::JoinSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, const FOnlineSessionSearchResult& SearchResult)
{
	bool _bSuccess = false;

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && UserID.IsValid())
		{
			OnJoinSessionCompleteDelegateHandle = _Session->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
			_bSuccess = _Session->JoinSession(*UserID, SessionName, SearchResult);
		}
	}

	return _bSuccess;
}

void AMPGameSession::FindSessions(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, bool bIsLAN, bool bIsPresence, bool bSearchDedicatedOnly, bool bSearchCustomGames)
{
	if (bSearchCustomGames && bSearchDedicatedOnly)
	{
		UE_LOG(LogSessions, Warning, TEXT("Both bSearchDedicatedOnly and bSearchCustomGames are enabled. A dedicated server can not be a custom game. Ignoring both."));
	}

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && UserID.IsValid())
		{
			CurrentSession.SessionName = SessionName;
			CurrentSession.bIsPresence = bIsPresence;
			CurrentSession.bIsLAN = bIsLAN;
			CurrentSession.User = UserID;

			SearchSettings = MakeShareable(new FMPOnlineSessionSearch(bIsLAN, bIsPresence));

			if (bSearchDedicatedOnly && !bSearchCustomGames)
			{
				SearchSettings->QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
			}

			TSharedRef<FOnlineSessionSearch> _SearchRef = SearchSettings.ToSharedRef();
			OnFindSessionsCompleteDelegateHandle = _Session->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);
			_Session->FindSessions(*CurrentSession.User, _SearchRef);
		}
	}
	else
	{
		OnFindSessionsComplete(false);
	}
}

bool AMPGameSession::CheckForAsyncInProgress() const
{
	if (HostSettings.IsValid() || SearchSettings.IsValid()) { return true; }

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			EOnlineSessionState::Type _GameSessionState = _Session->GetSessionState(NAME_GameSession);
			if (_GameSessionState != EOnlineSessionState::NoSession)
			{
				return true;
			}
		}
	}

	return false;
}

EOnlineAsyncTaskState::Type AMPGameSession::GetSearchResultState(int32& _SearchIndex, int32& _NumResults) const
{
	_NumResults = 0;
	_SearchIndex = 0;

	if (SearchSettings.IsValid())
	{
		if (SearchSettings->SearchState == EOnlineAsyncTaskState::Done)
		{
			_SearchIndex = CurrentSession.SessionIndex;
			_NumResults = SearchSettings->SearchResults.Num();
		}

		return SearchSettings->SearchState;
	}

	return EOnlineAsyncTaskState::NotStarted;
}

void AMPGameSession::ContinueMatchmaking()
{
	
}

void AMPGameSession::ResetSessionIndex()
{
	CurrentSession.SessionIndex = -1;
}

void AMPGameSession::CleanupOnlineSubsystem()
{ 
	SearchResults.Empty();
	SearchSettings = nullptr;
	HostSettings = nullptr;
	ResetSessionIndex();
}

void AMPGameSession::RegisterServer()
{
	DefferedStartServerLogic();
	StartServer();
}

void AMPGameSession::Restart()
{

}

void AMPGameSession::HandleMatchHasStarted()
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && _Session->GetNamedSession(NAME_GameSession))
		{
			OnStartSessionCompleteDelegateHandle = _Session->AddOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegate);
			_Session->StartSession(NAME_GameSession);
		}
	}
}

void AMPGameSession::HandleMatchHasEnded()
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && _Session->GetNamedSession(NAME_GameSession))
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				auto* _Controller = Cast<AMPController>(*It);
				if (_Controller && !_Controller->IsLocalController())
				{
					_Controller->ClientEndGame();
				}
			}
		}
	}
}

void AMPGameSession::Destroyed()
{

}

void AMPGameSession::StartServer()
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			TSharedPtr<class FMPOnlineSessionSettings> _HostSettings = MakeShareable(new FMPOnlineSessionSettings(false, false, MaxPlayers_Dedicated));
			_HostSettings->Set(SETTING_MATCHING_TIMEOUT, MATCH_TIMEOUT, EOnlineDataAdvertisementType::ViaOnlineService);
			_HostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			_HostSettings->Set(SETTING_GAMEMODE, Gamemode_Dedicated, EOnlineDataAdvertisementType::ViaOnlineService);
			_HostSettings->Set(SETTING_MAPNAME, Mapname_Dedicated, EOnlineDataAdvertisementType::ViaOnlineService);
			_HostSettings->bAllowInvites = true;
			_HostSettings->bIsDedicated = true;
			HostSettings = _HostSettings;

			OnCreateSessionCompleteDelegateHandle = _Session->AddOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegate);
			_Session->CreateSession(0, NAME_GameSession, *HostSettings);
		}
	}
}

void AMPGameSession::DefferedStartServerLogic()
{

}

void AMPGameSession::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			_Session->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateSessionCompleteDelegateHandle);
		}
	}
	
	OnCreateListenServerComplete().Broadcast(NAME_GameSession, bWasSuccessful);
}

void AMPGameSession::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			_Session->ClearOnStartSessionCompleteDelegate_Handle(OnStartSessionCompleteDelegateHandle);

			if (bWasSuccessful)
			{
				for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
				{
					AMPController* _Controller = Cast<AMPController>(*It);
					if (_Controller && !_Controller->IsLocalController())
					{
						_Controller->ClientStartGame();
					}
				}
			}
		}
	}
}

void AMPGameSession::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogSessions, Log, TEXT("Destroying session: %s. Was successful: %d"), *SessionName.ToString(), bWasSuccessful);

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			_Session->ClearOnDestroySessionCompleteDelegate_Handle(OnDestroySessionCompleteDelegateHandle);
			HostSettings = NULL;
		}
	}
}

void AMPGameSession::OnFindSessionsComplete(bool bWasSuccessful)
{
	UE_LOG(LogSessions, Log, TEXT("Created session was successful: %d"), bWasSuccessful);

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid())
		{
			_Session->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);

			for (int32 _Index = 0; _Index < SearchSettings->SearchResults.Num(); ++_Index)
			{
				const FOnlineSessionSearchResult& Result = SearchSettings->SearchResults[_Index];
				DumpSession(&Result.Session);

				FMPSearchResult _SearchResult;
				_SearchResult.CurrentPlayers = SearchSettings->SearchResults[_Index].Session.SessionSettings.NumPublicConnections - SearchSettings->SearchResults[_Index].Session.NumOpenPublicConnections;
				SearchSettings->SearchResults[_Index].Session.SessionSettings.Get(SETTING_GAMEMODE, _SearchResult.Gamemode);
				SearchSettings->SearchResults[_Index].Session.SessionSettings.Get(SETTING_SERVER_NAME, _SearchResult.ServerName);
			}
		}
	}
}

void AMPGameSession::OnFindFriendSessionsComplete(int32 ControllerID, bool bSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	
}

void AMPGameSession::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	FString _URL;

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && _Session->GetResolvedConnectString(SessionName, _URL))
		{
			_Session->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
		}
	}

	OnFindSessionsComplete().Broadcast(Result);
}

void AMPGameSession::OnNoMatchFound()
{

}

bool AMPGameSession::ClientTravelToSession(FName SessionName, int32 ControllerID)
{
	FString _URL;

	IOnlineSubsystem* _Subsystem = IOnlineSubsystem::Get();
	if (_Subsystem)
	{
		IOnlineSessionPtr _Session = _Subsystem->GetSessionInterface();
		if (_Session.IsValid() && _Session->GetResolvedConnectString(SessionName, _URL))
		{
			APlayerController* _Controller = UGameplayStatics::GetPlayerController(GetWorld(), ControllerID);
			if (_Controller)
			{
				_Controller->ClientTravel(_URL, TRAVEL_Absolute);
				return true;
			}
		}
		UE_LOG(LogSessions, Warning, TEXT("Failed to connect to session %s"), *SessionName.ToString());
	}

	return false;
}

const TArray<FMPSearchResult>& AMPGameSession::GetAllSearchResults() const
{
	return SearchResults;
}
