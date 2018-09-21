#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineIdentityInterface.h"
#include "OnlineSessionInterface.h"

#include "MPGameInstance.generated.h"

class AMPGameSession;

DECLARE_LOG_CATEGORY_EXTERN(LogGameInstance, Log, All);

UENUM(BlueprintType)
enum class EOnlineConnectionType : uint8
{
	Online,
	LAN,
	SinglePlayer,
	Offline,
};

USTRUCT(BlueprintType)
struct FServerFilter
{
	GENERATED_BODY()

	FServerFilter()
	{}

	FServerFilter(FString ServerName, FString GameMode, FString MapName, uint8 bSearchDedicated = true, uint8 bSearchLAN = false, uint8 bPasswordProtected = false, uint8 bShowEmpty = true, uint8 bShowFull = true, uint8 bAllowCustomGames = false)
	{
		this->ServerName = ServerName;
		this->GameMode = GameMode;
		this->MapName = MapName;
		this->bSearchDedicated = bSearchDedicated;
		this->bSearchLAN = bSearchLAN;
		this->bPasswordProtected = bPasswordProtected;
		this->bShowEmpty = bShowEmpty;
		this->bShowFull = bShowFull;
		this->bAllowCustomGames = bAllowCustomGames;
	}

	UPROPERTY(BlueprintReadWrite)
	FString ServerName;

	UPROPERTY(BlueprintReadWrite)
	FString GameMode;

	UPROPERTY(BlueprintReadWrite)
	FString MapName;

	UPROPERTY(BlueprintReadWrite)
	uint8 bSearchDedicated:1;

	UPROPERTY(BlueprintReadWrite)
	uint8 bSearchLAN:1;

	UPROPERTY(BlueprintReadWrite)
	uint8 bPasswordProtected:1;

	UPROPERTY(BlueprintReadWrite)
	uint8 bShowEmpty:1;

	UPROPERTY(BlueprintReadWrite)
	uint8 bShowFull:1;

	UPROPERTY(BlueprintReadWrite)
	uint8 bAllowCustomGames:1;
};

namespace FGameNetworkState
{
	extern const FName None;
	extern const FName MainMenu;
	extern const FName WelcomeMenu;
	extern const FName Playing;
	extern const FName RetryingConnection;
	extern const FName Transition;
}

UCLASS()
class UMPGameInstance : public UGameInstance
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta = (AllowPrivateAccess = true))
	UEnum* MapsEnum;
	
public:

	UMPGameInstance(){}
	virtual ~UMPGameInstance(){}
	
	UFUNCTION(BlueprintCallable)
	bool HostListenServer(ULocalPlayer* OwningPlayer, const FString& GameMode, const FString& GameOptions);
	
	UFUNCTION(BlueprintCallable)
	bool JoinServer(ULocalPlayer* OwningPlayer, int32 SessionIndex);

	bool JoinServer(ULocalPlayer* OwningPlayer, const FOnlineSessionSearchResult& SearchResult);
	bool FindServers(ULocalPlayer* OwningPlayer, const FServerFilter& ServerFilter);

	void TravelToSession(const FName& SessionName);

	void BeginQuickHost();

	UFUNCTION(BlueprintPure, Category = Utility)
	EOnlineConnectionType GetConnectionType() const { return ConnectionType; }

	void SetMultiplayerFeaturesForType(EOnlineConnectionType InType);
	void ReturnToMenuWithErrorToast(const FText& Error, const FText& Button);
	void ReturnToMenu();

public:

	/************************************************************************/
	/*							State Handling                              */
	/************************************************************************/

	void ResetState();
	void SetState(FName NewState);
	FName GetDefaultState() const { return FGameNetworkState::MainMenu; }
	FName GetCurrentState() const { return CurrentState; }
	bool IsStateInTransition() const { return (CurrentState == PendingState) || CurrentState == FGameNetworkState::Transition ? true : false; }
	void SwitchStateWithToast(const FName& NewState, const FText& ToastMessage, const FText& ConextButtonLeft, const FText& ContextButtonRight, const bool bOverride = true, TWeakObjectPtr<ULocalPlayer> OwningPlayer = nullptr);

protected:

	virtual void StartGameInstance() override;

	void OnCreateSessionComplete(FName SessionName, bool bSuccess);
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	virtual void FinalizeServerCreation(EOnJoinSessionCompleteResult::Type Result);

	AMPGameSession* GetGameSession() const;

	FDelegateHandle OnCreateSessionCompleteDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

private:
	
	FName Map;

	void DeferredTravelToSession(const FName& SessionName);

	FName PendingState;
	FName CurrentState;

	/** The Game URL that the player will travel to when possible **/
	FString GameURL;

	EOnlineConnectionType ConnectionType;

};