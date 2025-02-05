// Server-side implementation of a multi-threaded chat server

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 1024

std::unordered_map<int, std::string> clients;
std::unordered_map<std::string, std::string> users;
std::unordered_map<std::string, std::unordered_set<int>> groups;
std::mutex clients_mutex;

void load_users()
{
    std::ifstream file("users.txt");
    std::string line;
    while (getline(file, line))
    {
        size_t delimiter = line.find(':');
        if (delimiter != std::string::npos)
        {
            std::string username = line.substr(0, delimiter);
            std::string password = line.substr(delimiter + 1);
            users[username] = password;
        }
    }
}

void send_message(int client_socket, const std::string &message)
{
    send(client_socket, message.c_str(), message.size(), 0);
}

void broadcast(const std::string &message, int sender_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &pair : clients)
    {
        if (pair.first != sender_socket)
        {
            send_message(pair.first, message + "\n");
        }
    }
}

void private_message(const std::string &recipient, const std::string &message, int sender_socket)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto &pair : clients)
    {
        if (pair.second == recipient)
        {
            send_message(pair.first, "[Private] " + message);
            return;
        }
    }
    send_message(sender_socket, "User not found\n");
}

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    std::string username;

    send_message(client_socket, "Enter username: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = buffer;
    username.erase(username.find_last_not_of(" \n\r") + 1);

    send_message(client_socket, "Enter password: ");
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    std::string password = buffer;
    password.erase(password.find_last_not_of(" \n\r") + 1);

    if (users.find(username) == users.end() || users[username] != password)
    {
        send_message(client_socket, "Authentication failed\n");
        close(client_socket);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = username;
    }
    send_message(client_socket, "Welcome to the chat server!\n");
    broadcast(username + " has joined the chat.", client_socket);

    while (true)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0)
        {
            break;
        }
        std::string message = buffer;
        message.erase(message.find_last_not_of(" \n\r") + 1);

        if (message.find("/msg ") == 0)
        {
            size_t space = message.find(' ', 5);
            if (space != std::string::npos)
            {
                std::string recipient = message.substr(5, space - 5);
                std::string msg = message.substr(space + 1);
                private_message(recipient, "[" + username + "]: " + msg, client_socket);
            }
        }
        else if (message.find("/broadcast ") == 0)
        {
            std::string msg = message.substr(11);
            broadcast("[Broadcast] " + username + ": " + msg, client_socket);
        }
        else if (message.find("/create_group ") == 0)
        {
            std::string group_name = message.substr(14);
            groups[group_name].insert(client_socket);
            send_message(client_socket, "Group " + group_name + " created\n");
        }
        else if (message.find("/join_group ") == 0)
        {
            std::string group_name = message.substr(12);
            if (groups.find(group_name) != groups.end())
            {
                groups[group_name].insert(client_socket);
                send_message(client_socket, "You joined group " + group_name + "\n");
            }
            else
            {
                send_message(client_socket, "Group not found\n");
            }
        }
        else if (message.find("/leave_group ") == 0)
        {
            std::string group_name = message.substr(13);
            groups[group_name].erase(client_socket);
            send_message(client_socket, "You left group " + group_name + "\n");
        }
        else if (message.find("/group_msg ") == 0)
        {
            size_t space = message.find(' ', 11);
            if (space != std::string::npos)
            {
                std::string group_name = message.substr(11, space - 11);
                std::string msg = message.substr(space + 1);
                for (int client : groups[group_name])
                {
                    if (client != client_socket)
                    {
                        send_message(client, "[Group " + group_name + "] " + username + ": " + msg + "\n");
                    }
                }
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(client_socket);
    }
    broadcast(username + " has left the chat.", client_socket);
    close(client_socket);
}

int main()
{
    load_users();
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr *)&server_address, sizeof(server_address));
    listen(server_socket, 5);
    std::cout << "Server is running on port " << PORT << "\n";

    while (true)
    {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
        int client_socket = accept(server_socket, (sockaddr *)&client_address, &client_len);
        std::thread(handle_client, client_socket).detach();
    }
    return 0;
}
