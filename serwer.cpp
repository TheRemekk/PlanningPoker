#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

constexpr int MAX_EVENTS = 1000;
constexpr int BUFFER_SIZE = 1024;
constexpr int MAX_GAMES = 9999;
constexpr int MIN_GAMES = 1000;

struct Player {
    std::string nick;
    int socket;
    int selected_card = -1;
    bool is_playing = false;
};

struct Game {
    int id;
    int admin_socket;
    std::string topic;
    bool voting_started = false;
    bool overtime = false;
    std::unordered_map<int, Player> players; // socket -> Player
};

std::unordered_map<int, Game> games; // game_id -> Game
std::unordered_map<int, int> player_to_game; // player_socket -> game_id

void send_message(int socket, const std::string &message) {
    send(socket, message.c_str(), message.size(), 0);
}

void broadcast_message(const Game &game, const std::string &message, int exclude_socket = -1, bool is_in_game = false) {
    for (const auto &[socket, player] : game.players) {
        if (socket != exclude_socket && (player.is_playing == true || player.is_playing == is_in_game)) {
            send_message(socket, message);
        }
    }
}

int create_server_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, SOMAXCONN) == -1) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    return server_socket;
}

void handle_disconnect(int client_socket, int epoll_fd) {
    std::cout << "Client disconnected: " << client_socket << std::endl;

    if (player_to_game.count(client_socket)) {
        int game_id = player_to_game[client_socket];
        Game &game = games[game_id];

        if (client_socket == game.admin_socket) {
            // Administrator disconnected
            broadcast_message(game, "end game\n");
            for (const auto &[socket, _] : game.players) {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, nullptr);
                close(socket);
            }
            games.erase(game_id);
        } else {
            // Regular player disconnected
            broadcast_message(game, "leave " + game.players[client_socket].nick + "\n");
            game.players.erase(client_socket);
            player_to_game.erase(client_socket);
        }
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
    close(client_socket);
}

void handle_new_connection(int server_socket, int epoll_fd) {
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_len);

    if (client_socket == -1) {
        perror("Accept failed");
        return;
    }

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = client_socket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
        perror("Epoll add failed");
        close(client_socket);
        return;
    }

    std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
}

void handle_client_message(int client_socket, int epoll_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);

    if (bytes_read <= 0) {
        handle_disconnect(client_socket, epoll_fd);
        return;
    }

    buffer[bytes_read] = '\0';
    std::string message(buffer);
    std::cout << "Received message from " << client_socket << ": " << message << std::endl;

    std::istringstream iss(message);
    std::string command;
    iss >> command;

    if (command == "new") {
        iss >> command;
        if (command == "game") {
            int game_id;
            do {
                game_id = rand() % (MAX_GAMES - MIN_GAMES + 1) + MIN_GAMES;
            } while (games.count(game_id));

            games[game_id] = {game_id, client_socket, "", false, false, {}};
            games[game_id].players[client_socket] = {"admin", client_socket, -1, true};
            player_to_game[client_socket] = game_id;

            send_message(client_socket, "code " + std::to_string(game_id) + "\n");
        }
    } else if (command == "join") {
        int game_id;
        std::string nick;
        iss >> game_id >> nick;

        if (games.count(game_id)) {
            Game &game = games[game_id];
	    for(const auto &[socket, player] : game.players) {
                if(player.nick == nick) {
		    send_message(client_socket, "error player nick exists\n");
		    return;
		}
	    }
            game.players[client_socket] = {nick, client_socket};
            player_to_game[client_socket] = game_id;
            //send_message(client_socket, "join success\n");
	    broadcast_message(game, "joined " + games[game_id].players[client_socket].nick+ "\n", client_socket);
	    std::string result = "players";
            for (const auto &[socket, player] : game.players) {
                    result += " " + player.nick;
            }
	    send_message(client_socket, result + "\n");
        } else {
            send_message(client_socket, "error game not found\n");
        }
    } else if (command == "temat") {
        std::string topic;
        getline(iss, topic);
        topic = topic.substr(1); // Remove leading space

        int game_id = player_to_game[client_socket];
        Game &game = games[game_id];
		for(auto &[socket, player] : game.players) {
			player.is_playing = true;
		}
        if (client_socket == game.admin_socket) {
            game.topic = topic;
            broadcast_message(game, "temat " + topic + "\n", client_socket, true);
        } else {
            send_message(client_socket, "error not admin\n");
        }
    } else if (command == "start") {
        int game_id = player_to_game[client_socket];
        Game &game = games[game_id];

        if (client_socket == game.admin_socket) {
            game.voting_started = true;
            game.overtime = false;
            broadcast_message(game, "start\n", client_socket, true);
        } else {
            send_message(client_socket, "error not admin\n");
        }
    } else if (command == "stop") {
        int game_id = player_to_game[client_socket];
        Game &game = games[game_id];

        if (client_socket == game.admin_socket) {
            broadcast_message(game, "stop\n", client_socket, true);
            std::unordered_map<int, int> card_counts;

            for (const auto &[socket, player] : game.players) {
                if (player.selected_card >= 1 && player.selected_card <= 13) {
                    card_counts[player.selected_card]++;
                }
            }

            std::string result = "selected card";
            for (const auto &[socket, player] : game.players) {
                if (player.selected_card >= 1 && player.selected_card <= 13) {
                    result += " " + player.nick + " " + std::to_string(player.selected_card);
                }
            }
            broadcast_message(game, result + "\n", -1, true);

            int total_votes = 0;
            int winning_card = -1;
            for (const auto &[card, count] : card_counts) {
                total_votes += count;
	    }
	    for (const auto &[card, count] : card_counts) {
                if (count > total_votes / 2) {
                    winning_card = card;
                }
            }

            if (winning_card != -1) {
                broadcast_message(game, "won " + std::to_string(winning_card) + "\n", -1, true);
            } else {
                game.overtime = true;
                broadcast_message(game, "overtime\n", -1, true);
            }
        } else {
            send_message(client_socket, "error not admin\n");
        }
    } else if (command == "selected") {
        std::string sub_command;
        iss >> sub_command;
        if (sub_command == "card") {
            int card;
            iss >> card;

            int game_id = player_to_game[client_socket];
            Game &game = games[game_id];
            if (game.voting_started) {
				if (card >= 1 && card <= 13) {
                game.players[client_socket].selected_card = card;
                send_message(game.admin_socket, "selected card " + game.players[client_socket].nick + " " + std::to_string(card) + "\n");
				} else {
					send_message(client_socket, "error wrong card number\n");
				}
				
            } else {
                send_message(client_socket, "error voting not started\n");
            }
        }
    } else if (command == "message") {
        int game_id = player_to_game[client_socket];
        Game &game = games[game_id];

        if (game.overtime) {
            std::string message;
            getline(iss, message);
            message = message.substr(1); // Remove leading space

            broadcast_message(game, "message " + game.players[client_socket].nick + " " + message+ "\n", client_socket, true);
        } else {
            send_message(client_socket, "error not in overtime");
        }
    } else if (command == "leave") {
        int game_id = player_to_game[client_socket];
        Game &game = games[game_id];

        broadcast_message(game, "leave " + games[game_id].players[client_socket].nick+ "\n", client_socket);
        game.players.erase(client_socket);
        player_to_game.erase(client_socket);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
        close(client_socket);
    } else if (command == "end") {
        std::string sub_command;
        iss >> sub_command;
        if (sub_command == "game") {
            int game_id = player_to_game[client_socket];
            Game &game = games[game_id];

            if (client_socket == game.admin_socket) {
                broadcast_message(game, "end game\n", client_socket);
                for (const auto &[socket, _] : game.players) {
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, nullptr);
                    close(socket);
                }
                games.erase(game_id);
            } else {
                send_message(client_socket, "error not admin\n");
            }
        }
    } else {
        send_message(client_socket, "error unknown command\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }

    int port = std::stoi(argv[1]);
    int server_socket = create_server_socket(port);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Epoll create failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_socket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        perror("Epoll add failed");
        close(server_socket);
        close(epoll_fd);
        return EXIT_FAILURE;
    }

    epoll_event events[MAX_EVENTS];

    while (true) {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        if (event_count == -1) {
            perror("Epoll wait failed");
            break;
        }

        for (int i = 0; i < event_count; ++i) {
            if (events[i].data.fd == server_socket) {
                handle_new_connection(server_socket, epoll_fd);
            } else {
                handle_client_message(events[i].data.fd, epoll_fd);
            }
        }
    }

    close(server_socket);
    close(epoll_fd);
    return 0;
}
