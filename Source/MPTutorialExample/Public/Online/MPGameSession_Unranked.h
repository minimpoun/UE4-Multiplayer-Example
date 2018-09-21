#pragma once

#include "CoreMinimal.h"
#include "MPGameSession.h"

#include "MPGameSession_Unranked.generated.h"

UCLASS()
class AMPGameSession_Unranked : public AMPGameSession
{
	GENERATED_BODY()
	
public:

	AMPGameSession_Unranked(){}
	virtual ~AMPGameSession_Unranked(){}
	


protected:

	virtual void StartServer() override;
	virtual void EndServer() override;

private:
	
};