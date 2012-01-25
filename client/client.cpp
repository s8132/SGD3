#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "Tactic.h"
#include <ctime>
#include <allegro5\allegro.h>
#include <allegro5\allegro5.h>
#include <allegro5\keyboard.h>
#include <allegro5\mouse.h>
#include <allegro5\allegro_primitives.h>
#include <allegro5\bitmap.h>
#include <allegro5\bitmap_io.h>
#include <allegro5\allegro_image.h>
#include <allegro5\allegro_font.h>
#include <allegro5\allegro_ttf.h>


using namespace std;


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "1313"

const int SIZE_TILE = 50;

bool SendData(SOCKET ClientScoket, char* buffer)
{
	send(ClientScoket, buffer, strlen(buffer), 0);
	return true;
}

bool ReceiveData(SOCKET ClientSocket, char* buffer, int size)
{
	int i = recv(ClientSocket, buffer, size, 0);
	buffer[i]='\0';
	return true;
}

const int STRLEN = 256;

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    //char *sendbuf = "this is a test";
    char recvbuf[DEFAULT_BUFLEN];
	char sendbuf[STRLEN];
	char name[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

	
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }


	/*ALLEGRO INIT*/

	if(!al_init()){
		cout << "Problem z inicjalizacja biblioteki allegro!" << endl;
		return -1;
	}

	int width, height;

	width = 8 * SIZE_TILE + 1;
	height = 8 * SIZE_TILE + 1;

	ALLEGRO_DISPLAY *display = al_create_display(width, height);

	if(!display){
		cout << "Utworzenie okna sie nie powiodlo! :(" << endl;
		return -1;
	}

	
	
	al_set_window_title(display, "Reversi");
	al_set_window_position(display, 0, 0);

	//al_clear_to_color(al_map_rgb(57, 10, 70));

	

	//Inicjalizacje dodatków
	al_init_primitives_addon();
	al_install_keyboard();
	al_install_mouse();
	al_init_image_addon();
	al_init_font_addon();

	ALLEGRO_BITMAP *window = al_create_bitmap(width, height);

	ALLEGRO_BITMAP *net = al_create_bitmap(width, height);

	ALLEGRO_BITMAP *black = al_load_bitmap("black2.png");
	ALLEGRO_BITMAP *white = al_load_bitmap("white.png");

	ALLEGRO_BITMAP *win = al_create_bitmap(width, height);
	ALLEGRO_BITMAP *lose = al_create_bitmap(width, height);
	ALLEGRO_BITMAP *text = al_create_bitmap(width, height);

	ALLEGRO_FONT *font = al_load_ttf_font("font.ttf", 12, 0);

	if(!font){
		cout << "nie udalo sie wczytac cziconki" << endl;
	}

	ALLEGRO_KEYBOARD_STATE keyboard_state;
	al_get_keyboard_state(&keyboard_state);

	ALLEGRO_MOUSE_STATE mouse_state;
	al_get_mouse_state(&mouse_state);

	al_set_target_bitmap(net);

	int xpos, ypos;

	xpos=1;
	ypos=1;

	for(int i=0; i<width; i++){
		al_draw_line(xpos, ypos, xpos, height, al_map_rgb(255, 255, 255), 1);
		xpos = xpos + SIZE_TILE;
	}

	xpos=1;
	ypos=1;

	for(int i=0; i<height; i++){
		al_draw_line(xpos, ypos, width, ypos, al_map_rgb(255, 255, 255), 1);
		ypos = ypos + SIZE_TILE;
	}

	bool done = false;
	double timer = al_current_time();
	int r=0;
	while(!done){
	
		al_set_target_bitmap(window);
		al_clear_to_color(al_map_rgb(r, 0, 0));
		//al_convert_mask_to_alpha(this->buff, al_map_rgb(0, 0, 0));

		al_draw_bitmap(net, 0, 0, 0);

		al_draw_bitmap(white, 150, 150, 0);
		al_draw_bitmap(white, 200, 200, 0);
		al_draw_bitmap(black, 200, 150, 0);
		al_draw_bitmap(black, 150, 200, 0);

		al_get_keyboard_state(&keyboard_state);
		al_get_mouse_state(&mouse_state);

		if(al_key_down(&keyboard_state, ALLEGRO_KEY_ESCAPE)){
			char *sendbuf = "koniec";
			SendData(ConnectSocket, sendbuf);
			done=true;
		}

		
		if(al_mouse_button_down(&mouse_state, 1)){
			if(al_current_time() >= timer+0.5){
				//al_draw_text(font, al_map_rgb(0, 0, 255), width/2, height/2, ALLEGRO_ALIGN_CENTRE, "Cos tam");
				timer = al_current_time();
			}

		}
		
		if(al_current_time() >= timer+0.2){
				//al_draw_text(font, al_map_rgb(0, 0, 255), width/2, height/2, ALLEGRO_ALIGN_CENTRE, "Cos tam");
				timer = al_current_time();
				r++;
		}

		al_draw_bitmap_region(white, 1, 1, 20, 20, 3, 3, 0);
		
		//al_draw_bitmap(this->point, xpos, ypos, NULL);

		al_set_target_bitmap(al_get_backbuffer(display));            
		//al_clear_to_color(al_map_rgb(57, 10, 70));
		al_draw_bitmap(window, 0, 0, 0);
		
		al_flip_display();
		
		if(r==255){
			r=0;
		}
		
	}


	


    closesocket(ConnectSocket);
    WSACleanup();


	return 0;
}
