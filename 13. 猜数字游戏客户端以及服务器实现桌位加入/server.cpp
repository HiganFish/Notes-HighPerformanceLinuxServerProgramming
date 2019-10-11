#include <sys/socket.h>
#include <sys/types.h>
#include <cassert>
#include <cstring>
#include <string>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cerrno>

#define BUFFER_SIZE 1024
#define MAX_DESK 3
#define DESK_MAXUSER 2
#define FD_LIMIT 65535
int MAX_NUM = 100;
int MIN_NUM = 0;

#define B_GETDESK 1
#define B_ENTER 2
#define B_GAMESTATUS 3
#define B_GAME 4
#define B_GAME_RANGE 5
#define B_GAME_RESULT 6

int MsgProcessCenter(int msg_fd, const char* msg);
void SendDeskMsg(int socket);
void AddToDesk(int msg_fd, const char* desk_msg);
void PlayGame(int msg_fd, int desk_sub, const char* number);
void ResetGameDesk(const int desk_sub);

struct SDESK {
	int online_user_num = 0;
	int online_user[DESK_MAXUSER];
	bool is_playing = false;

	int max_num;
	int min_num;
	int ans_num;
};

struct SCLIENT_DATA {
	sockaddr_in sock_address;
	int desk_sub;
	char buf[BUFFER_SIZE];
	char* write_buf;
};

SCLIENT_DATA* users = new SCLIENT_DATA[FD_LIMIT];
SDESK* desks = new SDESK[MAX_DESK];

void SendMsgCenter(const int sockfd, int desk_sub, int send_method, const int type, const bool result, const char* msg)
{
	char msgBuffer[1024] = {};

	if(nullptr == msg)
	{
		sprintf(msgBuffer, "{%d%d@1@1}", type, result?1:0);
	}
	else
	{
		sprintf(msgBuffer, "{%d%d@%zu@%s}", type, result?1:0, strlen(msg), msg);
	}
	// 1 all 2 except owner 3 owner
	if(send_method == 3)
	{
		send(sockfd, msgBuffer, strlen(msgBuffer), 0);
		return;
	}

	for(int sock : desks[desk_sub].online_user)
	{

		if(send_method == 2 && sock != sockfd) {
			send(sock, msgBuffer, strlen(msgBuffer), 0);
		}
		else if (send_method == 1)
		{
			send(sock, msgBuffer, strlen(msgBuffer), 0);
		}
	}

}

void RemoveLeftClient(pollfd* fds, int& user_counter, int left_sock) {
	int desk_sub;
	if ((desk_sub = users[left_sock].desk_sub) != -1) {
		int (*desk_users) = desks[desk_sub].online_user;
		for (int i = 0; i < DESK_MAXUSER; ++i) {
			if (desk_users[i] == left_sock) {
				std::string msg = "player " + std::to_string(left_sock) + " left, game over;";
				SendMsgCenter(left_sock, desk_sub, 2, B_GAME_RESULT, true, msg.c_str());
			}
		}
		ResetGameDesk(desk_sub);
	}

	int i = 0;
	while (fds[i].fd != left_sock) {
		i++;
	}
	close(fds[i].fd);
	fds[i].fd = fds[user_counter].fd;
	fds[user_counter].fd = -1;
	user_counter--;

}

int main(int argc, char* argv[]) {

	if (argc <= 2)
	{
		printf("Wrong number of parameters");
		return 1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address{};
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);

	int listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert(listenfd >= 0);

	int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
	assert(ret != -1);

	ret = listen(listenfd, 5);
	assert(ret != -1);

	int MAX_USER = MAX_DESK * DESK_MAXUSER;
	int user_counter = 0;

	pollfd fds[MAX_USER + 1];
	for (int i = 1; i <= MAX_USER; i++)
	{
		fds[i].fd = -1;
		fds[i].events = 0;
	}
	fds[0].fd = listenfd;
	fds[0].events = POLLIN | POLLERR;
	fds[0].revents = 0;

	while (1)
	{

		ret = poll(fds, MAX_USER + 1, -1);
		if (ret <= 0)
		{
			printf("poll error\n");
			break;
		}

		for (int i = 0; i < MAX_USER + 1; i++)
		{

			pollfd the_fds = fds[i];

			if ((the_fds.fd == listenfd) && (the_fds.revents & POLLIN))
			{
				if (user_counter == MAX_USER)
				{
					continue;
				}
				user_counter++;

				struct sockaddr_in client_address{};
				socklen_t client_addrlength = sizeof(client_address);
				int client_sock = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);

				users[client_sock].sock_address = client_address;
				users[client_sock].desk_sub = -1;
				fds[user_counter].fd = client_sock;
				fds[user_counter].events = POLLIN | POLLRDHUP | POLLERR;
				continue;
			}

			if (the_fds.revents & POLLERR)
			{
				// TODO remove loser
				printf("client %d left-POLLERR\n", the_fds.fd);
				continue;
			}

			if (the_fds.revents & POLLRDHUP)
			{
				// TODO remove loser

				printf("client %d left-POLLRDHUP\n", the_fds.fd);
				RemoveLeftClient(fds, user_counter, the_fds.fd);
				continue;
			}

			if (the_fds.revents & POLLIN)
			{

				char recvbuf[BUFFER_SIZE] = {};
				recv(the_fds.fd, recvbuf, BUFFER_SIZE - 1, 0);

				printf("%d-%s\n", the_fds.fd, recvbuf);

				int processResult = MsgProcessCenter(the_fds.fd, recvbuf);
				if (processResult == -1)
				{
					printf("Wrong msg %s\n", recvbuf);
				}
			}
		}
	}
	delete[] users;
	delete[] desks;
	close(listenfd);
	return 0;
}

void MsgProcess(const int msg_fd, const int msgType, const char* trueMsg)
{
	switch (msgType)
	{
		case 1:
			SendDeskMsg(msg_fd);
			break;
		case 2:

		case 3:
			AddToDesk(msg_fd, trueMsg);
			break;
		case 4:
			PlayGame(msg_fd, users[msg_fd].desk_sub, trueMsg);
		default:
			break;
	}
}

int MsgProcessCenter(const int msg_fd, const char* msg)
{
	char trueMsg[BUFFER_SIZE] = {};
	int length = strlen(msg);
	if (msg[0] == '{' && msg[length-1] == '}')
	{

		strncpy(trueMsg, msg + 3, length - 4);

		MsgProcess(msg_fd, msg[1] - 48, trueMsg);
		return 0;
	}
	else
	{
		return -1;
	}
}

void SendDeskMsg(const int socket)
{
	std::string deskInfo;
	for (int i =0; i < MAX_DESK; i++)
	{
		deskInfo += std::to_string(i) + " SDESK - " + std::to_string(DESK_MAXUSER - desks[i].online_user_num) + " persion available\n";
	}

	SendMsgCenter(socket, 0, 3, B_GETDESK, true, deskInfo.c_str());
}

void AddToDesk(const int msg_fd, const char* desk_msg)
{

	int deskSub = desk_msg[0] - 48;
	int deskOnlineer = desks[deskSub].online_user_num;


	for (int onlinesocket : desks[msg_fd].online_user)
	{
		if (onlinesocket == msg_fd)
		{
			SendMsgCenter(msg_fd, 0, 3, B_ENTER, false, nullptr);
			return;
		}
	}

	if (deskOnlineer == DESK_MAXUSER)
	{

		SendMsgCenter(msg_fd, 0,3,B_ENTER, false, nullptr);
	}
	users[msg_fd].desk_sub = deskSub;
	desks[deskSub].online_user[deskOnlineer] = msg_fd;
	desks[deskSub].online_user_num++;
	deskOnlineer++;

	if (deskOnlineer == DESK_MAXUSER)
	{
		SendMsgCenter(msg_fd, users[msg_fd].desk_sub, 1, B_GAMESTATUS, true, nullptr);
	}
	else
	{
		std::string result4 = std::to_string(deskOnlineer) + "-" + std::to_string(DESK_MAXUSER);
		SendMsgCenter(msg_fd, users[msg_fd].desk_sub, 3, B_ENTER, true, result4.c_str());
	}
}

void IntiGame(const int desk_sub)
{
	desks[desk_sub].max_num = MAX_NUM;
	desks[desk_sub].min_num = MIN_NUM;

	std::srand((unsigned int)time(nullptr));
	int a = MAX_NUM - MIN_NUM;
	desks[desk_sub].ans_num = std::rand() % a;
	desks[desk_sub].is_playing = true;
}

void ResetGameDesk(const int desk_sub)
{
	users[desks[desk_sub].online_user[0]].desk_sub = -1;
	users[desks[desk_sub].online_user[1]].desk_sub = -1;
	desks[desk_sub].online_user[0] = -1;
	desks[desk_sub].online_user[1] = -1;
	desks[desk_sub].max_num = MAX_NUM;
	desks[desk_sub].min_num = MIN_NUM;
	desks[desk_sub].online_user_num = 0;
}

void PlayGame(const int msg_fd, const int desk_sub, const char* number)
{

	if (!desks[desk_sub].is_playing)
	{
		IntiGame(desk_sub);
	}

	int num = atoi(number);


	if ((num < desks[desk_sub].min_num) || (num > desks[desk_sub].max_num))
	{
		SendMsgCenter(msg_fd, desk_sub, 3, B_GAME, true, nullptr);

		return;
	}
	std::string result_status ="player-" + std::to_string(msg_fd) + " guess-" + number;
	SendMsgCenter(msg_fd, desk_sub, 2, B_GAME, true, result_status.c_str());

	if (num < desks[desk_sub].ans_num)
	{
		desks[desk_sub].min_num = num;
	}
	else if (num > desks[desk_sub].ans_num)
	{
		desks[desk_sub].max_num = num;
	}
	else if (num == desks[desk_sub].ans_num)
	{
		char result_winer[1] = {static_cast<char>(48 + msg_fd)};
		SendMsgCenter(msg_fd, desk_sub, 1, B_GAME_RESULT, true, result_winer);
		ResetGameDesk(desk_sub);
		return;
	}

	std::string result1 = std::to_string(desks[desk_sub].min_num) + "-" + std::to_string(desks[desk_sub].max_num);
	SendMsgCenter(msg_fd, desk_sub, 1, B_GAME_RANGE, true, result1.c_str());
}