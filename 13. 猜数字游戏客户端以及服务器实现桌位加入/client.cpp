#include <sys/socket.h>
#include <sys/types.h>
#include <cassert>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <cerrno>
#include <string>

#define BUFFER_SIZE 1024

#define B_GETDESK 1
#define B_ENTER 2
#define B_GAMESTATUS 3
#define B_GAME 4
#define B_GAME_RANGE 5
#define B_GAME_RESULT 6

void ProcessMsgCenter(const char* msg);
int now_game_status = 1;
bool can_guess_num = true;

int main(int argc, char* argv[]) {


	if (argc <= 2)
	{
		printf("Wrong number of parameters");
		return 1;
	}

	char* ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in host_address{};
	host_address.sin_port = htons(port);
	host_address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &host_address.sin_addr);

	int host_sock = socket(PF_INET, SOCK_STREAM, 0);
	assert(host_sock >= 0);

	int ret = connect(host_sock, (struct sockaddr*)&host_address, sizeof(host_address));
	assert(ret != -1);

	pollfd pds[2];
	pds[0].fd = 0;
	pds[0].events = POLLIN;
	pds[0].revents = 0;
	pds[1].fd = host_sock;
	pds[1].events = POLLIN | POLLRDHUP;
	pds[1].revents = 0;

	assert(ret != -1);

	char buff[BUFFER_SIZE];

	printf("%s", "输入任意内容回车获取房间信息\n");

	while (1)
	{

		ret = poll(pds, 2, -1);
		if (ret <= 0)
		{
			printf("Poll error\n");
			break;
		}

		if (pds[0].revents & POLLIN)
		{
			memset(buff, '\0', BUFFER_SIZE);
			int recv_length = read(pds[0].fd, buff, BUFFER_SIZE - 1);
			buff[strlen(buff) - 1] = '\0';

			if (buff[0] == '&')
			{
				now_game_status++;
				continue;
			}

			if (now_game_status == 4 && !can_guess_num)
			{

				printf("现在没有轮到你， 请等待\n");
				continue;
			}
			if (now_game_status == 4)
			{
				can_guess_num = false;
			}
			std::string the_msg = "{" + std::to_string(now_game_status) + "@" + buff + "}";
			send(host_sock, the_msg.c_str(), the_msg.length(), 0);
		}

		if (pds[0].revents & POLLRDHUP)
		{
			const char* msg = "lose connection";
			printf("%s", msg);
		}

		if (pds[1].revents & POLLIN)
		{
			memset(buff, '\0', BUFFER_SIZE);
			read(pds[1].fd, buff, BUFFER_SIZE - 1);


			ProcessMsgCenter(buff);
		}

	}

	return 0;
}

void PlayGame() {

}

void EnterRoom(int msg_bool, int msg_length, char* true_msg) {
	if(msg_bool == 1) {
		printf("已有%d-游戏开始所需人数%d\n", true_msg[0] - 48, true_msg[2] - 48);
		now_game_status = 3;
	}
	else
	{
		printf("%s\n","进入房间失败, 请进入其它房间\n");
	}

}

void MsgProcess(int msg_type, int msg_type_bool, int msg_length, char* true_msg) {
	switch (msg_type)
	{
		case B_GETDESK:
			printf("请选择房间加入\n%s\n", true_msg);
			now_game_status = 2;
			break;
		case B_ENTER:
			EnterRoom(msg_type_bool, msg_length, true_msg);
			break;
		case B_GAMESTATUS:
			printf("游戏开始\n");
			now_game_status = 4;
			break;
		case B_GAME:
			printf("%s\n",true_msg);
			can_guess_num = msg_type_bool;
			break;
		case B_GAME_RANGE:
			printf("当前范围%s\n",true_msg);

			break;
		case B_GAME_RESULT:
			printf("玩家%s胜利\n输入任意内容回车获取房间信息\n",true_msg);
			now_game_status = 1;
			break;
	}
}

void GetInfoFromMsg(const char* msg) {

	int msg_type = msg[0] - 48;
	int msg_type_bool = msg[1] - 48;
	char msg_length_char[3] = {};
	int msg_length_char_sub = 0;
	bool is_first_flag = true;
	int msg_length = strlen(msg);
	for(int i = 0; i < msg_length; ++i)
	{
		if(is_first_flag &&msg[i] == '@')
		{
			msg_length_char[msg_length_char_sub] = msg[i + 1];
			msg_length_char_sub++;
			if(msg[i + 2] == '@')
			{
				break;
			}

			is_first_flag = false;
			continue;
		}
		if(!is_first_flag && msg[i] == '@')
		{
			msg_length_char[msg_length_char_sub] = msg[i - 1];
			break;
		}
	}

	int msg_true_length = atoi(msg_length_char);
	char true_msg[1024] = {};
	strcpy(true_msg, msg + msg_length - msg_true_length);
	MsgProcess(msg_type, msg_type_bool, msg_true_length, true_msg);
}

void ProcessMsgCenter(const char* msg)
{
	bool left_bracket_falg = false;
	bool right_bracket_falg = false;

	static char recv_buffer [1024];
	static char recv_buffer_last[1024];

	std::string the_msg = msg;

	int the_msg_length = (int)the_msg.length();

	int begin_sub = 0;
	for (int i = 0; i < the_msg_length; ++i)
	{

		if (the_msg[i] == '{')
		{
			left_bracket_falg = true;
			begin_sub = i + 1;
			memset(recv_buffer_last, '\0', 1024);
		}
		if (the_msg[i] == '}')
		{
			if (left_bracket_falg)
			{
				memset(recv_buffer, '\0', 1024);
				strncpy(recv_buffer, msg + begin_sub, i - begin_sub);

				if (i + 1 < the_msg_length)
				{
					strncpy(recv_buffer_last, msg + i + 1, the_msg_length - i);
					GetInfoFromMsg(recv_buffer);
					ProcessMsgCenter(recv_buffer_last);
				}
				else
				{
					GetInfoFromMsg(recv_buffer);
					fflush(stdout);
				}

			}
		}

	}
}