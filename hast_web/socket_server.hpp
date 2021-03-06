#ifndef hast_web_socket_server_hpp
#define hast_web_socket_server_hpp
#include <hast/tcp_config.hpp>
#include <hast_web/crypto.hpp>
#include <hast_web/server_thread.hpp>
#include <fstream>   //For read file to blob
#include <assert.h>  // For read file
#include <sys/poll.h>// For poll()
#include <cstring>   //errno
#include <list>
//#include <iostream>   //dev mode


#define MAX_EVENTS 10
enum WebSocketFrameType {
	ERROR_FRAME=0xFF00,
	INCOMPLETE_FRAME=0xFE00,

	OPENING_FRAME=0x3300,
	CLOSING_FRAME=0x3400,

	DONE_TEXT=0x01,         // This message is finish.
	DONE_TEXT_BEHIND=0x02,  // This message  is over, but more data need to be processed.
	DONE_TEXT_CONTIN=0x03,  // This message is follow previous message, but there are messages
							// are finish earlier than "previous message"
	
	DONE_BINARY=0x04,
	DONE_BINARY_BEHIND=0x05,
	DONE_BINARY_CONTIN=0x06,
	
	CONTIN_TEXT=0x07,
	CONTIN_TEXT_BEHIND=0x08,
	
	CONTIN_BINARY=0x09,
	CONTIN_BINARY_BEHIND=0x0A,
	
	OVERSIZE_FRAME=0x0B,
	NO_MESSAGE=0x0C,
	RECYCLE_THREAD=0x0D,

	TEXT_FRAME=0x81,
	BINARY_FRAME=0x82,
	
	PING_FRAME=0x19,
	PONG_FRAME=0x1A
};

namespace hast_web{
	class socket_server : public tcp_config, public hast_web::server_thread{
	protected:
		struct sockaddr_storage client_addr;
		socklen_t client_addr_size;
		int epollfd;
		bool got_it {true};
		const short int listen_pending{50};
		const short int transport_size{1000};
		const short int resize_while_loop{20};
		std::size_t file_max {5242880}; //5MiB
		struct epoll_event ev,ev_tmp, events[MAX_EVENTS];
		int host_socket {0};
		std::list<std::string> pending_msg;
		std::list<int> pending_socket;
		std::list<bool> pending_done;
		std::list<bool> pending_binary;
	
		std::mutex wait_mx,close_mx;

		bool single_poll(const int socket_index, const short int time);
		inline void epoll_on(const short int thread_index);
		inline void clear_pending(int socket_index);
		/**
		 * RETURN NO_MESSAGE
		 * RETURN DONE_TEXT
		 * RETURN CONTIN_TEXTE
		 * RETURN CONTIN_BINARY
		 * RETURN DONE_BINARY
		 **/
		virtual WebSocketFrameType pop_pending(const short int thread_index);
		/**
		 * RETURN NO_MESSAGE
		 * RETURN DONE_TEXT
		 * RETURN DONE_BINARY
		 * RETURN count [0]: No further action, kepp going. (RESET to 0)
		 * RETURN count [>0]: Get msg, and this socket has more msgs coming. (RESET to 0)
		 **/
		WebSocketFrameType msg_pop_pending(const short int thread_index, short int &count);
		std::string* push_pending(int socket_index, std::string &msg, bool done, bool binary);
		void upgrade(std::string &headers,std::string &user,std::string &password);
		/**
		 * RETURN NO_MESSAGE
		 * RETURN ERROR_FRAME
		 * RETURN DONE_TEXT
		 * RETURN CONTIN_TEXT
		 * RETURN DONE_TEXT_CONTIN
		 * RETURN CONTIN_BINARY
		 * RETURN DONE_BINARY
		 * RETURN DONE_BINARY_CONTIN
		 * RETURN count: How many message go to pending_list (NOT RESET)
		 **/
		WebSocketFrameType more_data(const short int thread_index, short int &count);
		virtual bool read_loop(const short int thread_index, std::basic_string<unsigned char> &raw_str);
		virtual inline void recv_epoll();
		WebSocketFrameType getFrame(std::basic_string<unsigned char> &raw_str, std::string &msg);
		std::size_t makeFrameU(WebSocketFrameType frame_type, unsigned char* msg, std::size_t msg_len, unsigned char* buffer, std::size_t buffer_len);
		std::size_t makeFrame(WebSocketFrameType frame_type, const char* msg, std::size_t msg_len, char* buffer, std::size_t buffer_len);
		virtual void close_socket(int socket);
	public:
		socket_server();
		~socket_server();
		std::function<void(const int)> on_close {nullptr};
		void file_max_mb(std::size_t  max); //Mib
		short int get_blob_frame(std::vector<char> &blob, std::size_t size);
		bool file_to_blob(std::string &file_name,std::vector<char> &blob, short int unsigned extra = 0);
		bool init(hast::tcp_socket::port port, short int unsigned max = 2);
		/**
		 * RETURN DONE_TEXT
		 * RETURN DONE_BINARY
		 * RETURN RECYCLE_THREAD
		 **/
		WebSocketFrameType msg_recv(const short int thread_index);
		/**
		 * RETURN DONE_TEXT
		 * RETURN CONTIN_TEXT
		 * RETURN RECYCLE_THREAD
		 * RETURN DONE_BINARY
		 * RETURN CONTIN_BINARY
		 **/
		WebSocketFrameType partially_recv(const short int thread_index);
		/**
		 * RETURN DONE_BINARY
		 * RETURN CONTIN_BINARY
		 * RETURN DONE_TEXT
		 * RETURN CONTIN_TEXT
		 * RETURN NO_MESSAGE
		 * RETURN ERROR_FRAME
		 **/
		WebSocketFrameType more_recv(const short int thread_index);
		virtual void close_socket(const short int thread_index);
		void done(const short int thread_index);
	};
};
#include <hast_web/socket_server.cpp>
#endif /* hast_web_socket_server_hpp */
