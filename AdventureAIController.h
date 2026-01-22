#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AdventureAIController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 * AI Controller:
 * - Behavior tree boot
 * - Blackboard init
 * - AI Perception (Sight)
 * - Sets initial BB values (player actor, etc.)
 */
UCLASS()
class SIMPLESHOOTER_API A_
