// Fill out your copyright notice in the Description page of Project Settings.

#include "Lab_1.h"
#include "SimpleAIController.h"

ASimpleAIController::ASimpleAIController()
{
	bDeliveringOrder = false;
	CurrentOrderNumber = -1;

}

float ASimpleAIController::CalcRank(float timeLeft, float Distance) {
	return timeLeft/100 * timeLeft * timeLeft * (Distance);
}

void ASimpleAIController::Tick(float DeltaSeconds)
{
	auto HouseLocations = GetHouseLocations();
	auto Orders = GetPizzaOrders();

	if (bDeliveringOrder) {
		float Distance = GetDistanceToDestination(CurrentDestination);
		for (int i = 0; i < Orders.Num(); ++i) {
			if (Orders[i].OrderNumber != CurrentOrderNumber) {
				float currentDistance = GetDistanceToDestination(HouseLocations[Orders[i].HouseNumber]);
				if (currentDistance < 400.0f) {
					bool res = TryDeliverPizza(Orders[i].OrderNumber);
					if (res) {
						int PizzaAmount = GetPizzaAmount();
						if (PizzaAmount == 0) {
							bool bGrabbedPizza = TryGrabPizza();
							// Failed to retrieve pizza, need to get closer to the bakery.
							if (!bGrabbedPizza) {
								return;
							}
						}
					}
				}
			}
		}

		if (Distance <= 300.f) {

			UE_LOG(LogTemp, Warning, TEXT("Trying to deliver order %d, current distance: %1.3f"), CurrentOrderNumber, Distance);
			bool bDeliveredOrder = TryDeliverPizza(CurrentOrderNumber);
			if (bDeliveredOrder) {
				UE_LOG(LogTemp, Warning, TEXT("Delivered order %d"), CurrentOrderNumber);
				bDeliveringOrder = false;
				CurrentOrderNumber = -1;
			}
		}
//		else {
//			SetNewMoveDestination(CurrentDestination);
//		}
//		return;
	}

	int lastOrder = CurrentOrderNumber;

	HouseLocations = GetHouseLocations();
	Orders = GetPizzaOrders();
	if (Orders.Num() == 0) {
		// No orders to serve.
		return;
	}
	// Take first order.

	int nextOrder = 0;
	float rank = CalcRank(GetHouseTimeLeft(Orders[0].HouseNumber), GetDistanceToDestination(HouseLocations[Orders[0].HouseNumber]));
	bool neighbour = false;
	for (int i = 0; i < Orders.Num(); ++i) {
		float currentDistance = GetDistanceToDestination(HouseLocations[Orders[i].HouseNumber]);
		if (currentDistance < 400.0f) {
			nextOrder = i;
			break;
		}

		float curLeftTime = GetHouseTimeLeft(Orders[i].HouseNumber);
		float curRank = CalcRank(GetHouseTimeLeft(Orders[i].HouseNumber), GetDistanceToDestination(HouseLocations[Orders[i].HouseNumber]));
		if (curLeftTime < 10 && currentDistance < 600.0f) {
			nextOrder = i;
			neighbour = true;
		}
		if (!neighbour && curRank < rank) {
			nextOrder = i;
			rank = curRank;
		}
	}
	auto Order = Orders[nextOrder];

	int PizzaAmount = GetPizzaAmount();
	if (PizzaAmount == 0) {
		bool bGrabbedPizza = TryGrabPizza();
		// Failed to retrieve pizza, need to get closer to the bakery.
		if (!bGrabbedPizza) {
			return;
		}
	}

	auto HouseLocation = HouseLocations[Order.HouseNumber];
	bDeliveringOrder = true;
	CurrentOrderNumber = Order.OrderNumber;
	CurrentDestination = HouseLocation;
	SetNewMoveDestination(HouseLocation);
	if (lastOrder != CurrentOrderNumber) {
		UE_LOG(LogTemp, Warning, TEXT("Took new order %d to house %d"), Order.OrderNumber, Order.HouseNumber);
	}
}

