#include <enet/enet.h>

#include <iostream>
#include <thread>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Winmm.lib")

ENetHost* client = nullptr;
ENetAddress address;
ENetEvent event;
ENetPeer* peer;

string userInput = "";
string userName = "";
bool didQuit = false;

bool CreateClient();
int InitClient();
int RunClient();
void HandleConnection();
void HandleEvent();
void HandlePacket();

int main(int argc, char** argv)
{
    RunClient();
}

bool CreateClient()
{
    client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    return client != nullptr;
}

int InitClient()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }

    if (!CreateClient())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
        cout << "An error occurred while trying to create an ENet client host." << endl;
        exit(EXIT_FAILURE);
    }
}

int RunClient()
{
    atexit(enet_deinitialize);
    
    InitClient();

    HandleConnection();

    cout << "Enter your username?" << endl;
    cin >> userName;
    cout << "Welcome " << userName << "! You may begin chatting." << endl;

    //while (enet_host_service(client, &event, 50) > 0 && !didQuit)
    while (!didQuit)
    {
        thread PacketThread(HandlePacket);
        HandleEvent();
        /* One could just use enet_host_service() instead. */
        //enet_host_service();
        enet_host_flush(client);
        PacketThread.join();
    }

    if (client != nullptr)
    {
        enet_host_destroy(client);
    }


    return EXIT_SUCCESS;
}


void HandleConnection()
{
    /* Connect to some.server.net:1234. */
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(client, &address, 2, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        cout << "Connection to 127.0.0.1:1234 succeeded." << endl;
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        cout << "Connection to 127.0.0.1:1234 failed." << endl;
    }
}

void HandleEvent()
{    
    if (enet_host_service(client, &event, 50) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
            /* Print received packet. */
            cout << (char*)event.packet->data << endl;
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);
        }
    }
}

void HandlePacket()
{
    string formattedMessage;
    cout << userName << ": ";
    cin >> userInput;
    if (userInput == "quit")
    {
        formattedMessage = userName + " has left the chat.";
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

    enet_host_broadcast(client, 0, packet);
    //enet_peer_send(event.peer, 0, packet);
    
}