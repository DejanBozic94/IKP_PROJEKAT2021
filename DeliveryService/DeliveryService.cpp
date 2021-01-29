#pragma comment(lib, "ws2_32.lib")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#define DELIVERY_SERVICE_PORT 12343
#define FOOD_SERVICE_PORT 12342
#define CUSTOMER_PORT 12341
#define MAXLINE 1024

#include <stdio.h>
#include <stdlib.h>
#include <WinSock2.h>
#include "../Common/FunctionsAndStructures.h"

int main() {

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup failed with error code: %d\n", WSAGetLastError());
		return -1;
	}

	SOCKET listenFoodServiceSocket = INVALID_SOCKET;
	SOCKET foodServiceSocket = INVALID_SOCKET;

	char buffer[MAXLINE];
	int iResult;

	sockaddr_in serverAddress;
	memset((char*)&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddress.sin_port = htons(DELIVERY_SERVICE_PORT);

	listenFoodServiceSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenFoodServiceSocket == INVALID_SOCKET) {
		printf("Function socket() failed with error code: %d\n", WSAGetLastError());
		WSACleanup();
	}

	iResult = bind(listenFoodServiceSocket, (const sockaddr*)&serverAddress, sizeof(serverAddress));
	if (iResult == SOCKET_ERROR) {
		printf("Function bind() failed with error code: %d\n", WSAGetLastError());
		closesocket(listenFoodServiceSocket);
		WSACleanup();
	}

	iResult = listen(listenFoodServiceSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("Function listen() failed with error code: %d\n", WSAGetLastError());
		closesocket(listenFoodServiceSocket);
		WSACleanup();
	}

	foodServiceSocket = accept(listenFoodServiceSocket, NULL, NULL);
	if (foodServiceSocket == INVALID_SOCKET) {
		if (WSAGetLastError() == WSAECONNRESET) {
			printf("Function accept() failed, because timeout expired.\n");
		}
		else {
			printf("Function accept() failed with error code: %d\n", WSAGetLastError());
			closesocket(listenFoodServiceSocket);
			WSACleanup();
		}
		return -1;
	}

	char deliveryState[] = "BUSY";
	int option;

	do {
		iResult = recv(foodServiceSocket, buffer, MAXLINE, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Function recv() faild with error code: %d\n", WSAGetLastError());
			closesocket(foodServiceSocket);
			closesocket(listenFoodServiceSocket);
			WSACleanup();
		}

		buffer[iResult] = '\0';

		do {
			printf("Food Service:\n");
			printf("%s", buffer);
			printf("\n1.\t FREE\n");
			printf("2.\t BUSY\n");
			printf("Input option: ");
			scanf("%d", &option);
			getchar();

		} while (option != 1 && option != 2);

		if (option == 1) {
			strcpy(deliveryState, "FREE");
			break;
		}

		// send to Food Service that Delivery is BUSY
		char messageDeliveryState[] = "BUSY";
		iResult = send(foodServiceSocket, messageDeliveryState, strlen(messageDeliveryState) + 1, 0);

		if (iResult == SOCKET_ERROR) {
			printf("Function send() failed with error code: %d\n", WSAGetLastError());
			closesocket(foodServiceSocket);
			WSACleanup();
		}

	} while (strcmp(deliveryState, "BUSY") == 0);

	if (option == 1) {
		// send()
		char messageDeliveryState[] = "FREE";
		iResult = send(foodServiceSocket, messageDeliveryState, strlen(messageDeliveryState) + 1, 0);

		if (iResult == SOCKET_ERROR) {
			printf("Function send() failed with error code: %d\n", WSAGetLastError());
			closesocket(foodServiceSocket);
			WSACleanup();
		}

		Sleep(3000);

		// recv() customer order + price
		iResult = recv(foodServiceSocket, buffer, MAXLINE, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Function recv() faild with error code: %d\n", WSAGetLastError());
			closesocket(foodServiceSocket);
			closesocket(listenFoodServiceSocket);
			WSACleanup();
		}

		buffer[iResult] = '\0';

		printf("\nOrder received from Food Service: %s\n", buffer);

		Sleep(3000);
		printf("Press ENTER to send order to customer: ");
		getchar();
		
		printf("\nSending order to customer...\n");
		Sleep(3000);
		// send() to customer "this is your order, that will be 1400,00 dinars"
		char temp[1024];
		strcpy(temp, buffer);

		char** temp2 = str_split(temp, '|');	// temp2 = { "Dejan Bozic", "sandwich_2,pizza_3,ice cream_1", "Novi Sad", "0", "1400.00" }
		char price[50];
		strcpy(price, *(temp2 + 4));

		SOCKET connectSocket = INVALID_SOCKET;

		sockaddr_in serverAddress2;
		memset((char*)&serverAddress2, 0, sizeof(serverAddress2));

		serverAddress2.sin_family = AF_INET; // IPv4
		serverAddress2.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		serverAddress2.sin_port = htons(CUSTOMER_PORT);

		connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (connectSocket == INVALID_SOCKET) {
			printf("Function socket() failed with error code: %d\n", WSAGetLastError());
			WSACleanup();
		}

		iResult = connect(connectSocket, (const sockaddr*)&serverAddress2, sizeof(serverAddress2));
		if (iResult == SOCKET_ERROR) {
			printf("Function connect() failed with error code: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
		}

		char messageOrderForCustomer[50] = "This is your order. That will be ";
		char* messageOrderForCustomerTemp = concat(messageOrderForCustomer, price);
		messageOrderForCustomerTemp = concat(messageOrderForCustomerTemp, " dinars.");

		iResult = send(connectSocket, messageOrderForCustomerTemp, strlen(messageOrderForCustomerTemp) + 1, 0);

		if (iResult == SOCKET_ERROR) {
			printf("Function send() failed with error code: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
		}

		printf("Sending to customer message: %s\n", messageOrderForCustomerTemp);

		Sleep(3000);

		iResult = recv(connectSocket, buffer, MAXLINE, 0);
		if (iResult == SOCKET_ERROR) {
			printf("Function recv() faild with error code: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			WSACleanup();
		}

		buffer[iResult] = '\0';

		printf("Message from customer: %s\n", buffer);

		printf("caossss");

	}
	
	printf("\nDOSAO JE DOVDE SERVER\n");
	
	getchar();
	
	return 0;
}

