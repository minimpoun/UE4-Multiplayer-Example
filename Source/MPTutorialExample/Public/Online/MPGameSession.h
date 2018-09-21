#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "Online.h"

#include "MPGameSession.generated.h"

#define SETTING_SERVER_NAME FName(TEXT("SERVERNAME"))

class AMPBaseGameMode;
class FMPOnlineSessionSearch;
class FMPOnlineSessionSettings;

MPTUTORIALEXAMPLE_API DECLARE_LOG_CATEGORY_EXTERN(LogSessions, Log, All);

enum
{
	EXITCODE_UNKNOWN_FAILURE		= 0,
	EXITCODE_FAILED_TO_TRAVEL		= 1,
	EXITCODE_FAILED_TO_AUTH			= 2,
	EXITCODE_BEACON_FAILURE			= 3,
	EXITCODE_CONNECTION_FAILURE		= 4,
	EXITCODE_INTERAL_ERROR			= 5,
	EXITCODE_SAFE_SHUTDOWN			= 6,
};

struct FMPSessionParams
{
	FMPSessionParams()
		: SessionName(NAME_None)
		, bIsLAN(false)
		, bIsPresence(false)
		, bIsDedicated(false)
		, SessionIndex(-1)
	{}

	FName SessionName;
	bool bIsLAN;
	bool bIsPresence;
	bool bIsDedicated;
	TSharedPtr<const FUniqueNetId> User;
	int32 SessionIndex;
};

/** Used in UMG to represent a session */
USTRUCT(BlueprintType)
struct FMPSearchResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Ping;

	UPROPERTY(BlueprintReadOnly)
	FString ServerName;

	UPROPERTY(BlueprintReadOnly)
	FString Gamemode;

	UPROPERTY(BlueprintReadOnly)
	FString MapName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentPlayers;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers;

	UPROPERTY(BlueprintReadOnly)
	bool bIsLAN;

	UPROPERTY(BlueprintReadOnly)
	bool bIsPasswordProtected;

	UPROPERTY(BlueprintReadOnly)
	FString Password;
};

class FMPOnlineSessionSettings : public FOnlineSessionSettings
{

public:

	FMPOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, bool bCanJoinInProgress = true, int32 MaxPlayers = 8)
	{
		check(MaxPlayers > 0)
		NumPublicConnections = MaxPlayers;
		NumPrivateConnections = 0;
		bAllowJoinInProgress = bCanJoinInProgress;
		bIsLANMatch = bIsLAN;
		bUsesPresence = bIsPresence;
		bAllowInvites = true;
		bAllowJoinViaPresence = true;
		bAllowJoinViaPresenceFriendsOnly = true;
		bShouldAdvertise = true;
	}

	virtual ~FMPOnlineSessionSettings()
	{}
};

class FMPOnlineSessionSearch : public FOnlineSessionSearch
{

public:

	FMPOnlineSessionSearch(bool bIsLAN = false, bool bIsPresence = false)
	{
		bIsLanQuery = bIsLAN;
		PingBucketSize = 50;
		MaxSearchResults = 10;
		QuerySettings.Set(SEARCH_PRESENCE, bIsPresence, EOnlineComparisonOp::Equals);
	}

	virtual ~FMPOnlineSessionSearch()
	{}
};

class FMPOnlineDedicatedSearchSettings : public FMPOnlineSessionSearch
{

public:

	FMPOnlineDedicatedSearchSettings(bool bIsLAN = false, bool bIsPresence = false, bool bSearchEmptyOnly = false)
		: FMPOnlineSessionSearch(bIsLAN, bIsPresence)
	{
		QuerySettings.Set(SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals);
		QuerySettings.Set(SEARCH_EMPTY_SERVERS_ONLY, bSearchEmptyOnly, EOnlineComparisonOp::Equals);
	}

	virtual ~FMPOnlineDedicatedSearchSettings()
	{}
};

UCLASS(Config=DefaultServerSettings)
class AMPGameSession : public AGameSession
{
	GENERATED_BODY()
	
	DECLARE_EVENT_OneParam(ThisClass, FOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type);
	FOnJoinSessionComplete OnJoinSessionCompleteEvent;

	DECLARE_EVENT_OneParam(ThisClass, FOnFindSessionsComplete, bool);
	FOnFindSessionsComplete OnFindSessionsCompleteEvent;

	DECLARE_EVENT_TwoParams(ThisClass, FOnCreateSessionComplete, FName, bool);
	FOnCreateSessionComplete OnCreateSessionCompleteEvent;

public:

	AMPGameSession();
	virtual ~AMPGameSession(){}

	void RequestSafeShutdown(int32 ExitCode);
	
	void StartMatchmaking();
	void CancelMatchmaking();
	int32 GetBestSession();

	bool HostSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, const FString& Gamemode, const FString& Map, bool bIsLAN, bool bIsPresence, bool bAllowJoinInProgress, int32 MaxPlayers);
	void FindSessions(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, bool bIsLAN, bool bIsPresence, bool bSearchDedicatedOnly = true, bool bSearchCustomGames = false);
	bool JoinSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, int32 SessionIndex);
	bool JoinSession(TSharedPtr<const FUniqueNetId> UserID, FName SessionName, const FOnlineSessionSearchResult& SearchResult);
	FOnFindSessionsComplete& OnFindSessionsComplete() { return OnFindSessionsCompleteEvent; }
	FOnJoinSessionComplete& OnJoinSessionsComplete() { return OnJoinSessionCompleteEvent; }
	FOnCreateSessionComplete& OnCreateListenServerComplete() { return OnCreateSessionCompleteEvent; }

	UFUNCTION(BlueprintPure)
	bool CheckForAsyncInProgress() const;

	EOnlineAsyncTaskState::Type GetSearchResultState(int32& SearchIndex, int32& NumResults) const;

	FORCEINLINE const TArray<FMPSearchResult>& GetAllSearchResults() const;

protected:

	TArray<FMPSearchResult> SearchResults;

	FORCEINLINE AMPBaseGameMode* GetBaseGM() const { return BaseGM; }

	void ContinueMatchmaking();
	void ResetSessionIndex();
	virtual void CleanupOnlineSubsystem();

	FMPSessionParams CurrentSession;
	TSharedPtr<FMPOnlineSessionSearch> SearchSettings;
	TSharedPtr<FMPOnlineSessionSettings> HostSettings;

	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FOnFindFriendSessionCompleteDelegate OnFindFriendSessionCompleteDelegate;
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate; 
	FOnUpdateSessionCompleteDelegate OnUpdateSessionCompleteDelegate;
	FOnMatchmakingCompleteDelegate OnMatchmakingCompleteDelegate;
	FOnCancelMatchmakingCompleteDelegate OnCancelMatchmakingCompleteDelegate;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;
	FDelegateHandle OnFindFriendSessionCompleteHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnUpdateSessionCompleteDelegateHandle;
	FDelegateHandle OnMatchmakingCompleteDelegateHandle;
	FDelegateHandle OnCancelMatchmakingCompleteDelegateHandle;

	/* Begin AGameSession Interface */
	virtual void RegisterServer() override;
	virtual void Restart() override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchHasEnded() override;
	/* End AGameSession Interface */

	/* Begin AActor Interface */
	virtual void Destroyed() override;
	/* End AActor Interface */

	/** 
	 *	Starts a dedicated server session 
	 **/
	virtual void StartServer();
	virtual void EndServer() PURE_VIRTUAL(, );

	/** 
	 *	Runs right before the server session starts.
	 *	Gives code time to set anything up or run commands that can only be done before the server session is running
	 **/
	virtual void DefferedStartServerLogic();

	/* Being Delegate Functions */
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	virtual void OnFindSessionsComplete(bool bWasSuccessful);
	virtual void OnFindFriendSessionsComplete(int32 ControllerID, bool bSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResults);
	virtual void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	virtual void OnNoMatchFound();
	/* End Delegate Functions */

	bool ClientTravelToSession(FName SessionName, int32 ControllerID);

	UPROPERTY(Config)
	int32 MaxPlayers_Dedicated;

	UPROPERTY(Config)
	FString Gamemode_Dedicated;

	UPROPERTY(Config)
	FString Mapname_Dedicated;

private:

	AMPBaseGameMode* BaseGM;

};