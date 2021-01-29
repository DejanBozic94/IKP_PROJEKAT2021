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

DWORD WINAPI CustomerSendOrderToFoodService(LPVOID lpParam) {
	LpParamForCustomerSendOrderToFoodService lp = *(LpParamForCustomerSendOrderToFoodService*)lpParam;

	//bool enoughInStock = checkIfEnoughInStock(lp.customerOrder, lp.stock);
	customerSendOrderToFoodService(lp.customerOrder, lp.connectSocket);

	return 0;
}

int main() {

	DWORD requestForCustomerSendOrderToFoodService;
	HANDLE hRequestForCustomerSendOrderToFoodService;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}

	char buffer[MAXLINE];
	// Customer order
	Order* customerOrder = (Order*)malloc(sizeof(Order));
	strcpy(customerOrder->name, "Dejan Bozic");
	strcpy(customerOrder->food, "sandwich_2,pizza_3,ice cream_1");
	strcpy(customerOrder->location, "Novi Sad");
	customerOrder->emergency = true;

	SOCKET connectSocket = INVALID_SOCKET;

	//customerSendOrderToFoodService(customerOrder, connectSocket);

	LpParamForCustomerSendOrderToFoodService* lp = (LpParamForCustomerSendOrderToFoodService*)malloc(sizeof(LpParamForCustomerSendOrderToFoodService));
	lp->connectSocket = connectSocket;
	lp->customerOrder= customerOrder;

	// THREAD
	hRequestForCustomerSendOrderToFoodService = CreateThread(NULL, 0, &CustomerSendOrderToFoodService, (LPVOID)lp, 0, &requestForCustomerSendOrderToFoodService);

	// Customer receives order from delivery and send money + tip
	SOCKET listenDeliverySocket = INVALID_SOCKET;
	SOCKET deliverySocket = INVALID_SOCKET;

	sockaddr_in serverAddress2;
	memset((char*)&serverAddress2, 0, sizeof(serverAddress2));

	serverAddress2.sin_family = AF_INET; // IPv4
	serverAddress2.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddress2.sin_port = htons(CUSTOMER_PORT);

	listenDeliverySocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenDeliverySocket == INVALID_SOCKET) {
		printf("Function socket() failed with error code: %d\n", WSAGetLastError());
		WSACleanup();
	}

	int iResult = bind(listenDeliverySocket, (const sockaddr*)&serverAddress2, sizeof(serverAddress2));
	if (iResult == SOCKET_ERROR) {
		printf("Function bind() failed with error code: %d\n", WSAGetLastError());
		closesocket(listenDeliverySocket);
		WSACleanup();
	}

	iResult = listen(listenDeliverySocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("Function listen() failed with error code: %d\n", WSAGetLastError());
		closesocket(listenDeliverySocket);
		WSACleanup();
	}

	deliverySocket = accept(listenDeliverySocket, NULL, NULL);
	if (deliverySocket == INVALID_SOCKET) {
		if (WSAGetLastError() == WSAECONNRESET) {
			printf("Function accept() failed, because timeout expired.\n");
		}
		else {
			printf("Function accept() failed with error code: %d\n", WSAGetLastError());
			closesocket(listenDeliverySocket);
			WSACleanup();
		}
		return -1;
	}

	Sleep(3000);

	iResult = recv(deliverySocket, buffer, MAXLINE, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Function recv() faild with error code: %d\n", WSAGetLastError());
		closesocket(deliverySocket);
		closesocket(listenDeliverySocket);
		WSACleanup();
	}

	buffer[iResult] = '\0';

	printf("\n\Delivery sent me order: %s\n\n\n", buffer);

	// sending delivery money + tip

	Sleep(3000);

	char messageForDelivery[50] = "Thank you, here is your money + tip. Bye!";

	iResult = send(deliverySocket, messageForDelivery, strlen(messageForDelivery) + 1, 0);

	if (iResult == SOCKET_ERROR) {
		printf("Function send() failed with error code: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
	}

	printf("Sending to customer message: %s\n", messageForDelivery);


	printf("caossss");
	CloseHandle(hRequestForCustomerSendOrderToFoodService);
	getchar();
	return 0;
}