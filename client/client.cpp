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
	al_init_ttf_addon();

	ALLEGRO_BITMAP *window = al_create_bitmap(width, height);

	ALLEGRO_BITMAP *net = al_create_bitmap(width, height);

	ALLEGRO_BITMAP *black = al_load_bitmap("black2.png");
	ALLEGRO_BITMAP *white = al_load_bitmap("white.png");

	ALLEGRO_BITMAP *win = al_create_bitmap(width, height);
	ALLEGRO_BITMAP *lose = al_create_bitmap(width, height);
	ALLEGRO_BITMAP *text = al_create_bitmap(width, height);

	ALLEGRO_FONT *font = al_load_ttf_font("pirulen.ttf", 24, 0);

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

	bool done = false, redraw=false;
	double timer = al_current_time();
	int r=0, index, count=0;
	char board[63];

	for(int i=0; i<64; i++){
		if(i==27 || i==36){
			board[i]='w';
		}else if(i==28 || i==35){
			board[i]='b';
		}else{
			board[i]='e';
		}
	}

	ALLEGRO_TIMER *time = al_create_timer(1.0 / 60);

	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	event_queue = al_create_event_queue();
	al_register_event_source(event_queue, al_get_timer_event_source(time));
	al_register_event_source(event_queue, al_get_mouse_event_source());

	al_start_timer(time);
	
	cout << "\nTablica pol: " << endl;

	
	ReceiveData(ConnectSocket, recvbuf, recvbuflen);
	char *odebrane = recvbuf;
	char pionki = *odebrane, pionkiRywal;

	bool ruch, zamien=false;

	if(pionki=='w'){
		pionkiRywal='b';
		ruch = false;
		cout <<"twoj ruch" << endl;
	}else{
		pionkiRywal='w';
		ruch = true;
		cout <<"zaczyna rywal" << endl;
	}

	

	while(!done){
		
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);
		if(ev.type == ALLEGRO_EVENT_TIMER) redraw = true;
		else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN){
			if(ev.mouse.button & 1){
				cout << "wcisnieta myszka!" << endl;
				int x, y, error=0, error2=4;
				x = ev.mouse.x/SIZE_TILE;
				y = ev.mouse.y/SIZE_TILE;
				index = y*8+x;
				if(board[index]=='e'){
					if(index >= 0 && board[index-8]==pionki){
						al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Nie mozna tu stawiac!");
					}else if(index <= 63  && board[index+8]==pionki){
						al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Nie mozna tu stawiac!");
					}else if(index >=0 && board[index-1]==pionki){
						al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Nie mozna tu stawiac!");
					}else if(index <=63 && board[index+1]==pionki){
						al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Nie mozna tu stawiac!");
					}else{
						if(board[index-8]=='e'){
								error++;
						}
						if(board[index+8]=='e'){
								error++;
						}
						if(board[index-1]=='e'){
							error++;
						}
						if(board[index+1]=='e'){
								error++;
						}

						if(error==4){
							al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Nie mozna tu stawiac!");
						}else{
							//mozna postawic
							board[index] = pionki;
							SendData(ConnectSocket, itoa(index, sendbuf, 10));
							ruch = true;
							zamien = true;
						}
					}
				}else{
					al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Nie mozna tu stawiac!");
				}
				
				if(zamien){
						//checkUp zamien
						for(int i=index-16; i>=index%8; i=i-8){
							if(board[i]==pionki){
								for(int j=i; j<=index; j=j+8){
									board[j]=pionki;
								}
							}
							if(board[i]=='e') break;
						}

						//checkDown zamien
						for(int i=index+16; i<=7*8+index%8; i=i+8){
							if(board[i]==pionki){
								for(int j=i; j>=index; j=j-8){
									board[j]=pionki;
								}
							}
						}

						//chekLeft zamien
						for(int i=index-2; i>=index/8*8; i=i--){
							if(board[i]==pionki){
								for(int j=i; j<=index; j=j++){
									board[j]=pionki;
								}
							}
						}

						//chekRight zamien
						for(int i=index+2; i<=index/8*8+8; i=i++){
							if(board[i]==pionki){
								for(int j=i; j>=index; j=j--){
									board[j]=pionki;
								}
							}
						}
				}
				/*
				ReceiveData(ConnectSocket, recvbuf, recvbuflen);
				index = (int)(recvbuf-48);
				board[index] = pionkiRywal;
				*/

				count++;
			}
		} else if(ev.type = ALLEGRO_EVENT_MOUSE_BUTTON_UP){
			if(ev.mouse.button & 1){
				
				
				
				//cout << "podniesiona myszka" << endl;
			}
		}
		if(redraw && al_event_queue_is_empty(event_queue)){
				al_set_target_bitmap(window);
				al_clear_to_color(al_map_rgb(0, 0, 255));
				al_draw_bitmap(net, 0, 0, 0);

				

				//Rysuj pionki z tablicy
				for(int i=0; i<64; i++){
					int x, y;
					x = i%8*SIZE_TILE;
					y = i/8*SIZE_TILE;

					if(board[i]=='w'){
						al_draw_bitmap(white, x, y, 0);
						//cout << "White x = " << x << ", y = " << y << endl;
					}

					if(board[i]=='b'){
						al_draw_bitmap(black, x, y, 0);
						//cout << "Black x = " << x << ", y = " << y << endl;
					}
				}
				
				
				al_get_keyboard_state(&keyboard_state);
				al_get_mouse_state(&mouse_state);

				if(al_key_down(&keyboard_state, ALLEGRO_KEY_ESCAPE)){
					char *sendbuf = "koniec";
					SendData(ConnectSocket, sendbuf);
					done=true;
				}

				if(ruch){
					al_draw_text(font, al_map_rgb(255, 0, 0), 10, 10, 0, "Ruch przeciwnika");
				}

				al_set_target_bitmap(al_get_backbuffer(display));            
				al_draw_bitmap(window, 0, 0, 0);
		
				al_flip_display();
		}

		if(ruch){
					ReceiveData(ConnectSocket, recvbuf, recvbuflen);
					index = atoi(recvbuf);
					board[index] = pionkiRywal;


					//checkUp zamien
				for(int i=index-16; i>=index%8; i=i-8){
					if(board[i]==pionkiRywal){
						for(int j=i; j<=index; j=j+8){
							board[j]=pionkiRywal;
						}
					}
					if(board[i]=='e') break;
				}

				//checkDown zamien
				for(int i=index+16; i<=7*8+index%8; i=i+8){
					if(board[i]==pionkiRywal){
						for(int j=i; j>=index; j=j-8){
							board[j]=pionkiRywal;
						}
					}
				}

				//chekLeft zamien
				for(int i=index-2; i>=index/8*8; i=i--){
					if(board[i]==pionkiRywal){
						for(int j=i; j<=index; j=j++){
							board[j]=pionkiRywal;
						}
					}
				}

				//chekRight zamien
				for(int i=index+2; i<=index/8*8+8; i=i++){
					if(board[i]==pionkiRywal){
						for(int j=i; j>=index; j=j--){
							board[j]=pionkiRywal;
						}
					}
				}
				

					ruch=false;
		}
		
	}


	


    closesocket(ConnectSocket);
    WSACleanup();


	return 0;
}
