#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#define MAXLINE 1024
#define NUMBER_OF_DIFFERENT_FOOD_ITEMS 3

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>
#include <assert.h>

#pragma comment (lib, "Ws2_32.lib")

typedef struct order {
	char name[50];
	char food[50];
	char location[50];
	bool emergency;
}Order;

typedef struct food {
	char name[50];
	int numberOfUnits;
	float pricePerUnit;
}Food;

typedef struct orderForDelivery {
	Order* order;
	float price;
}OrderForDelivery;

char* concat(const char *s1, const char *s2)
{
	char *result = (char*)malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

char* serializeOrderForDelivery(OrderForDelivery* orderForDelivery) {
	char *orderForDeliveryString;

	Order* order = (Order*)malloc(sizeof(Order));
	order = orderForDelivery->order;
	float price = orderForDelivery->price;
	char priceString[50];
	sprintf(priceString, "%.2f", price);
	
	char name[50];
	char food[50];
	char location[50];
	bool emergency;

	strcpy(name, order->name);
	strcpy(food, order->food);
	strcpy(location, order->location);
	emergency = order->emergency;

	// "Dejan Bozic|sandwich_2,pizza_3,ice cream_1|Novi Sad|0|1400.00"
	orderForDeliveryString = concat("", name);
	orderForDeliveryString = concat(orderForDeliveryString, "|");
	orderForDeliveryString = concat(orderForDeliveryString, food);
	orderForDeliveryString = concat(orderForDeliveryString, "|");
	orderForDeliveryString = concat(orderForDeliveryString, location);
	orderForDeliveryString = concat(orderForDeliveryString, "|");
	if (emergency == true) {
		orderForDeliveryString = concat(orderForDeliveryString, "1|");
	}
	else {
		orderForDeliveryString = concat(orderForDeliveryString, "0|");
	}
	orderForDeliveryString = concat(orderForDeliveryString, priceString);

	return orderForDeliveryString;
}

char** str_split(char* a_str, const char a_delim)
{
	char** result = 0;
	size_t count = 0;
	char* tmp = a_str;
	char* last_comma = 0;
	char delim[2];
	delim[0] = a_delim;
	delim[1] = 0;

	/* Count how many elements will be extracted. */
	while (*tmp)
	{
		if (a_delim == *tmp)
		{
			count++;
			last_comma = tmp;
		}
		tmp++;
	}

	/* Add space for trailing token. */
	count += last_comma < (a_str + strlen(a_str) - 1);

	/* Add space for terminating null string so caller
	   knows where the list of returned strings ends. */
	count++;

	result = (char**)malloc(sizeof(char*) * count);

	if (result)
	{
		size_t idx = 0;
		char* token = strtok(a_str, delim);

		while (token)
		{
			assert(idx < count);
			*(result + idx++) = _strdup(token);
			token = strtok(0, delim);
		}
		assert(idx == count - 1);
		*(result + idx) = 0;
	}

	return result;
}

char* serializeOrderForFoodService(Order* orderForFoodService) {
	char *orderForFoodServiceString;

	char name[50];
	char food[50];
	char location[50];
	bool emergency;

	strcpy(name, orderForFoodService->name);
	strcpy(food, orderForFoodService->food);
	strcpy(location, orderForFoodService->location);
	emergency = orderForFoodService->emergency;

	// "Dejan Bozic|sandwich_2,pizza_3,ice cream_1|Novi Sad|0"
	orderForFoodServiceString = concat("", name);
	orderForFoodServiceString = concat(orderForFoodServiceString, "|");
	orderForFoodServiceString = concat(orderForFoodServiceString, food);
	orderForFoodServiceString = concat(orderForFoodServiceString, "|");
	orderForFoodServiceString = concat(orderForFoodServiceString, location);
	orderForFoodServiceString = concat(orderForFoodServiceString, "|");
	if (emergency == true) {
		orderForFoodServiceString = concat(orderForFoodServiceString, "1");
	}
	else {
		orderForFoodServiceString = concat(orderForFoodServiceString, "0");
	}

	return orderForFoodServiceString;
}

// "Dejan Bozic|sandwich_2,pizza_3,ice cream_1|Novi Sad|0"
Order* deserializeCustomerOrder(char* customerOrderString) {
	Order* order = (Order*)malloc(sizeof(Order));
	char customerOrderStringTemp[1024];
	strcpy(customerOrderStringTemp, customerOrderString);

	char** splitByStraightLine = str_split(customerOrderStringTemp, '|'); // array = { "Dejan Bozic", "sandwich_2,pizza_3,ice cream_1", "Novi Sad", "0" }
		
	strcpy(order->name, *(splitByStraightLine + 0));
	strcpy(order->food, *(splitByStraightLine + 1));
	strcpy(order->location, *(splitByStraightLine + 2));
	if (*(splitByStraightLine + 3) == "0") {
		order->emergency = false;
	}
	else {
		order->emergency = true;
	}
		
	return order;
}



void foodServiceContactDeliveryService(float totalPrice, Order* customerOrder, SOCKET connectSocket) {
	printf("\nSending order to delivery with total price of: %.2f\n", totalPrice);
	Sleep(3000);
	// send() customer order to Delivery Service
	OrderForDelivery* orderForDelivery = (OrderForDelivery*)malloc(sizeof(OrderForDelivery));
	orderForDelivery->order = customerOrder;
	orderForDelivery->price = totalPrice;

	char* messageOrderForDelivery = serializeOrderForDelivery(orderForDelivery);

	printf("\n\nOrder for delivery string: %s\n\n", messageOrderForDelivery);

	int iResult = send(connectSocket, messageOrderForDelivery, strlen(messageOrderForDelivery) + 1, 0);

	if (iResult == SOCKET_ERROR) {
		printf("Function send() failed with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	Sleep(3000);

	printf("\nOrder sent.\n");

}

void printStock(Food** stock) {
	printf("\nFood in stock:\n");
	printf("No.\t Name\t No. of units\t Price per unit\t\n");

	for (int i = 0; i < NUMBER_OF_DIFFERENT_FOOD_ITEMS; i++) {
		printf("%d. %s\t %d\t %.2f\t\n", i + 1, stock[i]->name, stock[i]->numberOfUnits, stock[i]->pricePerUnit);
	}
}

char* foodServiceCheckDeliveryServiceState(SOCKET connectSocket) {
	char buffer[MAXLINE];

	char messageCheckDeliveryState[] = "What is your state?";

	int iResult = send(connectSocket, messageCheckDeliveryState, strlen(messageCheckDeliveryState) + 1, 0);

	if (iResult == SOCKET_ERROR) {
		printf("Function send() failed with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	printf("Sending to Delivery Service message: What is your state?\n");

	// recv()
	// Receiving message from delivery service if free or busy
	iResult = recv(connectSocket, buffer, MAXLINE, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Function recv() faild with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	buffer[iResult] = '\0';

	printf("\nDelivery is %s\n", buffer);
	char* deliveryState = buffer;

	return deliveryState;
}

bool checkIfEnoughInStock(Order* customerOrder, Food** stock) {
	// Checkin if food from customer order is in stock
	char customerOrderFood[50];
	strcpy(customerOrderFood, customerOrder->food);
	char** foodName_Units = str_split(customerOrderFood, ',');	// in foodName_Units we have array = { "sandwich_2" , "pizza_3" , "ice cream_4" }
	bool enoughInStock = true;
	float price = 0;

	// split each item in foodName_Units with ',' delimiter and check if in stock
	if (foodName_Units) {
		for (int i = 0; *(foodName_Units + i); i++) {
			char** foodNameAndUnits = str_split(*(foodName_Units + i), '_');	// in foodNameAndUnits we have array = { "sandwich" , "2" } for i = 0

			char foodName[50];
			char numberOfUnitsChar[10];
			int numberOfUnits;

			strcpy(foodName, *foodNameAndUnits);
			strcpy(numberOfUnitsChar, *(foodNameAndUnits + 1));
			numberOfUnits = atoi(numberOfUnitsChar);

			// go through stock and if foodName == stock[j], check if numberOfUnits <= stock[j]->numberOfUnits
			for (int j = 0; j < NUMBER_OF_DIFFERENT_FOOD_ITEMS; j++) {
				if (strcmp(stock[j]->name, foodName) == 0) {
					if (numberOfUnits <= stock[j]->numberOfUnits) {
						// everything is okay, there are units of food that customer ordered in the stock
					}
					else {
						// break, there are not enough units of food that customer ordered
						printf("\nWe are sory, we don't have that many %ss in stock. Try again tomorrow.\n\n", foodName);
						enoughInStock = false;
						break;
					}
				}
			}

			if (enoughInStock == false) {
				break;
			}
		}
	}

	return enoughInStock;

}

typedef struct lpParamForCheckIfEnoughInStock {
	Order* customerOrder;
	Food** stock;
	float* totalPrice;
}LpParamForCheckIfEnoughInStock;



float calculateTotalPriceOrder(Order* customerOrder, Food** stock) {
	char customerOrderFood[50];
	strcpy(customerOrderFood, customerOrder->food);
	char** foodName_Units = str_split(customerOrderFood, ',');	// in foodName_Units we have array = { "sandwich_2" , "pizza_3" , "ice cream_4" }
	bool enoughInStock = true;
	float price = 0;

	// split each item in foodName_Units with ',' delimiter and check if in stock
	if (foodName_Units) {
		for (int i = 0; *(foodName_Units + i); i++) {
			char** foodNameAndUnits = str_split(*(foodName_Units + i), '_');	// in foodNameAndUnits we have array = { "sandwich" , "2" } for i = 0

			char foodName[50];
			char numberOfUnitsChar[10];
			int numberOfUnits;

			strcpy(foodName, *foodNameAndUnits);
			strcpy(numberOfUnitsChar, *(foodNameAndUnits + 1));
			numberOfUnits = atoi(numberOfUnitsChar);

			// go through stock and if foodName == stock[j], check if numberOfUnits <= stock[j]->numberOfUnits
			for (int j = 0; j < NUMBER_OF_DIFFERENT_FOOD_ITEMS; j++) {
				if (strcmp(stock[j]->name, foodName) == 0) {
					if (numberOfUnits <= stock[j]->numberOfUnits) {
						// everything is okay, there are units of food that customer ordered in the stock
						// calculate total price for customer order
						price += numberOfUnits * stock[j]->pricePerUnit;
					}
				}
			}

			
		}
	}

	return price;
}


void updateStock(Order* customerOrder, Food** stock) {
	// Checkin if food from customer order is in stock
	char customerOrderFood[50];
	strcpy(customerOrderFood, customerOrder->food);
	char** foodName_Units = str_split(customerOrderFood, ',');	// in foodName_Units we have array = { "sandwich_2" , "pizza_3" , "ice cream_4" }
	bool enoughInStock = true;
	float price = 0;

	// split each item in foodName_Units with ',' delimiter and check if in stock
	if (foodName_Units) {
		for (int i = 0; *(foodName_Units + i); i++) {
			char** foodNameAndUnits = str_split(*(foodName_Units + i), '_');	// in foodNameAndUnits we have array = { "sandwich" , "2" } for i = 0

			char foodName[50];
			char numberOfUnitsChar[10];
			int numberOfUnits;

			strcpy(foodName, *foodNameAndUnits);
			strcpy(numberOfUnitsChar, *(foodNameAndUnits + 1));
			numberOfUnits = atoi(numberOfUnitsChar);

			// go through stock and if foodName == stock[j], check if numberOfUnits <= stock[j]->numberOfUnits
			for (int j = 0; j < NUMBER_OF_DIFFERENT_FOOD_ITEMS; j++) {
				if (strcmp(stock[j]->name, foodName) == 0) {
					if (numberOfUnits <= stock[j]->numberOfUnits) {
						// everything is okay, there are units of food that customer ordered in the stock
						// subtract units of food in stock that customer ordered
						// calculate total price for customer order
						stock[j]->numberOfUnits -= numberOfUnits;
						price += numberOfUnits * stock[j]->pricePerUnit;
					}
					else {
						// break, there are not enough units of food that customer ordered
						printf("\nWe are sory, we don't have that many %ss in stock. Try again tomorrow.\n\n", foodName);
						enoughInStock = false;
						break;
					}
				}
			}

			if (enoughInStock == false) {
				break;
			}
		}
	}

}


typedef struct lpParamForCustomerSendOrderToFoodService {
	Order* customerOrder;
	SOCKET connectSocket;
}LpParamForCustomerSendOrderToFoodService;

void customerSendOrderToFoodService(Order* customerOrder, SOCKET connectSocket) {
	// now, FoodService is a Client and DeliveryService is Server

	char buffer[MAXLINE];
	int iResult;

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(FOOD_SERVICE_PORT);

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET) {
		printf("Function socket() failed with error code: %d\n", WSAGetLastError());
		WSACleanup();
	}

	iResult = connect(connectSocket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR) {
		printf("Function connect() failed with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	char *messageOrderForFoodService = serializeOrderForFoodService(customerOrder);

	iResult = send(connectSocket, messageOrderForFoodService, strlen(messageOrderForFoodService) + 1, 0);

	if (iResult == SOCKET_ERROR) {
		printf("Function send() failed with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	printf("Sending to Food Service message: %s\n", messageOrderForFoodService);
	Sleep(10000);

	iResult = recv(connectSocket, buffer, MAXLINE, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Function recv() faild with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	buffer[iResult] = '\0';

	printf("Message from food service: %s\n\n", buffer);
}

void checkIfFreeDelivery(SOCKET connectSocket) {
	int connectToDelivery;
	do {
		printf("\nCustomer order is okay. Connecting to delivery to see if free or busy [1 - YES]: ");
		scanf("%d", &connectToDelivery);
		getchar();
	} while (connectToDelivery != 1);

	Sleep(3000);

	// now, FoodService is a Client and DeliveryService is Server




	char buffer[MAXLINE];
	int iResult;

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DELIVERY_SERVICE_PORT);

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (connectSocket == INVALID_SOCKET) {
		printf("Function socket() failed with error code: %d\n", WSAGetLastError());
		WSACleanup();
	}

	iResult = connect(connectSocket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR) {
		printf("Function connect() failed with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

}