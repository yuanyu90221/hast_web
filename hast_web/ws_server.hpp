#ifndef hast_web_ws_server_hpp
#define hast_web_ws_server_hpp
#include <hast_web/socket_server.hpp>

class ws_server : public hast_web::socket_server<int>{
public:
	std::function<bool(const int socket_index, std::string &user, std::string &password)> on_open {nullptr};
	std::function<bool(std::string &user, std::string &password)> on_connect {nullptr};
	void start_accept();
	inline void echo_back_msg(const short int thread_index, std::string &msg);
	inline void echo_back_msg(const short int thread_index, const char *msg);
	inline void echo_back_msg(const int socket_index, std::string &msg);
	inline void echo_back_msg(const int socket_index, const char *msg);
};

#include <hast_web/ws_server.cpp>
#endif /* hast_web_ws_server_hpp */
