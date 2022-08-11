#include <enet/enet.h>

#include <iostream>
#include <thread>
#include <string>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

ENetHost* server = nullptr;
ENetAddress address;
ENetEvent event;
ENetPeer* peer;

string userInput = "";
string userName = "";
bool didQuit = false;
const int k_eventWaitTime = 50;

bool CreateServer();
int InitServer();
int RunServer();
void HandleEvent();
void HandlePacket();


int main(int argc, char** argv)
{
    RunServer();   
}

bool CreateServer()
{
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server != nullptr;
}

int InitServer()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }

    if (!CreateServer())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }
}

int RunServer()
{
    atexit(enet_deinitialize);

    InitServer();

    cout << "Enter your username?" << endl;
    getline(cin, userName);
    cout << "Welcome " << userName << "! You may begin chatting." << endl;

    thread PacketThread(HandlePacket);
    HandleEvent();
    PacketThread.join();

    if (server != nullptr)
    {
        enet_host_destroy(server);
    }

    return EXIT_SUCCESS;
}

void HandleEvent()
{
    /* Wait up to 100 milliseconds for an event. */
    while (enet_host_service(server, &event, k_eventWaitTime) >= 0 && !didQuit)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from "
                << event.peer->address.host
                << ":" << event.peer->address.port
                << endl;
            /* Store any relevant client information here. */
            event.peer->data = (void*)("Client information");

            break;

        case ENET_EVENT_TYPE_RECEIVE:
            cout << (char*)event.packet->data << endl;
        /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            cout << (char*)event.peer->data << " disconnected." << endl;
            /* Reset the peer's client information. */
            event.peer->data = NULL;
        }

        enet_host_flush(server);
    }
}

void HandlePacket()
{
    do
    {
        string formattedMessage;
        getline(cin, userInput);
        if (userInput == "quit")
        {
            formattedMessage = "The server host (" + userName + ") has ended the chat session.";
            didQuit = true;
        }
        else
        {
            formattedMessage = userName + ": " + userInput;
        }
        /* Create a reliable packet of size 7 containing "packet\0" */
        ENetPacket* packet = enet_packet_create(formattedMessage.c_str(),
            formattedMessage.length() + 1,
            ENET_PACKET_FLAG_RELIABLE);

        enet_host_broadcast(server, 0, packet);
        //enet_peer_send(event.peer, 0, packet);
    } while (!didQuit);
}