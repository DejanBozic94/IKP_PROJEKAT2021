#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define FOOD_SERVICE_PORT 12342
#define DELIVERY_SERVICE_PORT 12343
#define CUSTOMER_PORT 12341
#define MAXLINE 1024

#define NUMBER_OF_DIFFERENT_FOOD_ITEMS 3
#define START_NUMBER_OF_ICE_CREAMS 5
#define START_NUMBER_OF_SANDWICHES 7
#define START_NUMBER_OF_PIZZAS 10
#define ICE_CREAM_PRICE 100
#define PIZZA_PRICE 300
#define SANDWICH_PRICE 200

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <string.h>

#include "../Common/FunctionsAndStructures.h"



DWORD WINAPI CheckIfEnoughInStock(LPVOID lpParam) {
	LpParamForCheckIfEnoughInStock lp = *(LpParamForCheckIfEnoughInStock*)lpParam;
	
	bool enoughInStock = checkIfEnoughInStock(lp.customerOrder, lp.stock);

	return 0;
}

//DWORD WINAPI CheckIfFreeDelivery(LPVOID lpParam) {
//	SOCKET connectSocket = *(SOCKET*)lpParam;
//	checkIfFreeDelivery(connectSocket);
//
//	return 0;
//}

int main() {

	float price = 0;

	DWORD requestForCheckIfEnoughInStock;
	HANDLE hRequestForCheckIfEnoughInStock;

	//DWORD requestForCheckIfFreeDelivery;
	//HANDLE hRequestForCheckIfFreeDelivery = NULL;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}

	SOCKET listenCustomerSocket = INVALID_SOCKET;
	SOCKET customerSocket = INVALID_SOCKET;

	char buffer[MAXLINE];
	int iResult;

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddress.sin_port = htons(FOOD_SERVICE_PORT);

	listenCustomerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenCustomerSocket == INVALID_SOCKET) {
		printf("Function socket() failed with error code: %d\n", WSAGetLastError());
		WSACleanup();
	}

	iResult = bind(listenCustomerSocket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR) {
		printf("Function bind() failed with error code: %d\n", WSAGetLastError());
		closesocket(listenCustomerSocket);
		WSACleanup();
	}

	iResult = listen(listenCustomerSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("Function listen() failed with error code: %d\n", WSAGetLastError());
		closesocket(listenCustomerSocket);
		WSACleanup();
	}

	customerSocket = accept(listenCustomerSocket, NULL, NULL);
	if (customerSocket == INVALID_SOCKET) {
		if (WSAGetLastError() == WSAECONNRESET) {
			printf("Function accept() failed, because timeout expired.\n");
		}
		else {
			printf("Function accept() failed with error code: %d\n", WSAGetLastError());
			closesocket(listenCustomerSocket);
			WSACleanup();
		}
		return -1;
	}

	iResult = recv(customerSocket, buffer, MAXLINE, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Function recv() faild with error code: %d\n", WSAGetLastError());
		closesocket(customerSocket);
		closesocket(listenCustomerSocket);
		WSACleanup();
	}

	buffer[iResult] = '\0';

	printf("\n\nCustomer sent order: %s\n\n\n", buffer);

	//deserialise customer order
	Order* customerOrder = (Order*)malloc(sizeof(Order));
	customerOrder = deserializeCustomerOrder(buffer);

	// Food in FoodService stock
	Food* iceCream = (Food*)malloc(sizeof(Food));
	Food* sandwich = (Food*)malloc(sizeof(Food));
	Food* pizza = (Food*)malloc(sizeof(Food));

	strcpy(iceCream->name, "ice cream");
	strcpy(sandwich->name, "sandwich");
	strcpy(pizza->name, "pizza");

	iceCream->numberOfUnits = START_NUMBER_OF_ICE_CREAMS;
	iceCream->pricePerUnit = ICE_CREAM_PRICE;

	sandwich->numberOfUnits = START_NUMBER_OF_SANDWICHES;
	sandwich->pricePerUnit = SANDWICH_PRICE;

	pizza->numberOfUnits = START_NUMBER_OF_PIZZAS;
	pizza->pricePerUnit = PIZZA_PRICE;

	Food* stock[] = { iceCream, sandwich, pizza };

	LpParamForCheckIfEnoughInStock* lp = (LpParamForCheckIfEnoughInStock*)malloc(sizeof(LpParamForCheckIfEnoughInStock));
	lp->customerOrder = customerOrder;
	lp->stock = stock;

	printStock(stock);

	// Check if food from customer order is in stock
	// if (true) -> connect to a delivery and ask if free or busy
		// if (delivery == free) -> send 'customer order + total price' to delivery
		// else if (delivery == busy && emergency == false) -> sleep(3000) and try asking delivery again
		// else if (delivery == busy && emergency == true)  -> send "We are sorry, our delivery is busy, please try again in 10 minutes." to customer
	// if (false) -> send "We are sory, we don't have that many units of food in stock. Try again tomorrow." to customer

	// Checkin if food from customer order is in stock
	bool enoughInStock = checkIfEnoughInStock(customerOrder, stock);

	// THREAD
	hRequestForCheckIfEnoughInStock = CreateThread(NULL, 0, &CheckIfEnoughInStock, (LPVOID)lp, 0, &requestForCheckIfEnoughInStock);

	if (enoughInStock == true) {
		SOCKET connectSocket = INVALID_SOCKET;

		checkIfFreeDelivery(connectSocket);

		// THREAD
		//hRequestForCheckIfFreeDelivery = CreateThread(NULL, 0, &CheckIfFreeDelivery, (LPVOID)&connectSocket, 0, &requestForCheckIfFreeDelivery);

		char* deliveryState;

		do {
			deliveryState = foodServiceCheckDeliveryServiceState(connectSocket);
			if ((strcmp(deliveryState, "BUSY") == 0) && (customerOrder->emergency == false)) { // -> sleep(3000) and try asking delivery again
				printf("\nDelivery is busy at the moment.\nContacting again in 3, 2, 1...\n");
				Sleep(3000);
			}
		} while (strcmp(deliveryState, "BUSY") == 0 && customerOrder->emergency == false);

		if (strcmp(deliveryState, "FREE") == 0) {
			
			price = calculateTotalPriceOrder(customerOrder, stock);

			foodServiceContactDeliveryService(price, customerOrder, connectSocket);

			updateStock(customerOrder, stock);
			printStock(stock);
			
			// send() message to customer that order will arive in 10 minutes
			char messageToClient[] = "Order will arive in 10 minutes.";
			int iResult = send(customerSocket, messageToClient, strlen(messageToClient) + 1, 0);

			if (iResult == SOCKET_ERROR) {
				printf("Function send() failed with error code: %d\n", WSAGetLastError());
				closesocket(customerSocket);
				WSACleanup();
			}

			printf("\nSent message to client: %s\n", messageToClient);
			Sleep(3000);
			printf("Shuting down...");
			Sleep(3000);
			getchar();
			iResult = closesocket(customerSocket);
			WSACleanup();

		}

		else if ((strcmp(deliveryState, "BUSY") == 0) && (customerOrder->emergency == true)) {
			// send() message to customer "We are sorry, our delivery is busy, please try again in 30 minutes."
			char messageToClient[] = "We are sorry, our delivery is busy, please try again in 30 minutes.";
			int iResult = send(customerSocket, messageToClient, strlen(messageToClient) + 1, 0);

			if (iResult == SOCKET_ERROR) {
				printf("Function send() failed with error code: %d\n", WSAGetLastError());
				closesocket(customerSocket);
				WSACleanup();
			}

			printf("\nSent message to client: %s\n", messageToClient);
			Sleep(3000);
			printf("Shuting down...");
			Sleep(3000);
			getchar();
			iResult = closesocket(customerSocket);
			WSACleanup();
		}


	}
	else {	// not enough units in stock
		// send "We are sory, we don't have that many units of food in stock. Try again tomorrow." to customer
		char messageToClient[] = "We are sory, we don't have that many units of food in stock. Try again tomorrow.";
		int iResult = send(customerSocket, messageToClient, strlen(messageToClient) + 1, 0);

		if (iResult == SOCKET_ERROR) {
			printf("Function send() failed with error code: %d\n", WSAGetLastError());
			closesocket(customerSocket);
			WSACleanup();
		}

		printf("\nSent message to client: %s\n", messageToClient);
		Sleep(3000);
		printf("Shuting down...");
		Sleep(3000);

		getchar();

		iResult = closesocket(customerSocket);
		WSACleanup();

	}

	CloseHandle(hRequestForCheckIfEnoughInStock);
	//CloseHandle(hRequestForCheckIfFreeDelivery);
	printf("\n\nCao svete!\n");

	getchar();

	return 0;
}