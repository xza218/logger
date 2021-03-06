#include <iostream>

#include <string>
#include <vector>
#include <deque>

#include <cctype>

#include <boost/asio.hpp>

#include "reqs.hh"
#include "msgs.hh"

using boost::asio::ip::tcp;

// Local Scope Definitions //////////////////////////////////////////
namespace
{
	typedef uint16_t PortType;
};

namespace Cfg
{
	static const PortType port = 9876;
	static const size_t session_buf_size = 4096;
	static const char * req_delim = "\r\n";
	static const bool req_trim_trailing_whitespaces = true;
	static const bool req_filter_out_non_printable_chars = true;

	// Debug flags
	static const bool req_debug = true;
	static const bool session_obj_debug = true;
	static const bool session_obj_debug_verbose = false;
	static const bool comm_debug = true;
	static const bool comm_debug_ultra_verbose = false;
	static const bool comm_write_debug_ultra_verbose = false;

};

class Session
	: public std::enable_shared_from_this<Session>
{
public:

	typedef std::string InMsg;
	typedef std::deque<InMsg> InQueue;


	Session(boost::asio::io_service & io_service)
	:	_socket(io_service),
		_in_buf(Cfg::session_buf_size)
	{
		if (Cfg::session_obj_debug_verbose)	std::cout << "Session constructed!\n";
	}

	~Session()
	{
		if (Cfg::session_obj_debug)	std::cout << "Session destroyed!\n";
	}

	void start()
	{
		if (Cfg::session_obj_debug_verbose)	std::cout << "Session started!\n";
		read();
	}

	tcp::socket & get_socket()
	{
		return _socket;
	}

private:
	void read()
	{
		std::shared_ptr<Session> self(shared_from_this());

		boost::asio::async_read_until(
			_socket,
			_in_buf,
			std::string(Cfg::req_delim),
			[this, self]
				// N.B.: Must copying "self" by value to increase ref count
				// so that the calling object doesn't die before callback.
				(boost::system::error_code ec, std::size_t bytes_transferred)
			{
				// Async read callback function

				if (ec)
				{
					if (Cfg::comm_debug)
						std::cout << "Read error: " << ec << std::endl;
					return;
				}

				if (Cfg::comm_debug)
					std::cout << "Bytes read: " << bytes_transferred << " Bytes\n";

				// Read buffer into temporary string
				//_in_buf.commit(bytes_transferred);
				std::istream is(&_in_buf);
				std::string in_msg;
				std::getline(is, in_msg);

				// Test raw message
				if (Cfg::comm_debug_ultra_verbose)
				{
					std::cout << "Read Raw: " << in_msg << std::endl;
					for (int c : in_msg)
					{
						std::cout << c << " ";
					}
					std::cout << std::endl;
				}

				// Trim trailing whitespace

				if (Cfg::req_trim_trailing_whitespaces)
				{
					while (!in_msg.empty())
					{
						char trailing_char = in_msg.back();
						if (!isprint(trailing_char) || isspace(trailing_char))
						{
							in_msg.pop_back();
						}
						else
						{
							break;
						}
					}
				}

				// Delete all non-printables

				if (Cfg::req_filter_out_non_printable_chars)
				{
					in_msg.erase(
						std::remove_if(
							in_msg.begin(),
							in_msg.end(),
							[](char c)
								{
									return !isprint(c);
								}
							),
						in_msg.end()
					);
				}

				// If message still isn't empty, add it to the queue
				if (!in_msg.empty())
				{

					if (Cfg::comm_debug)
						std::cout << "Read Req: " << in_msg << std::endl;

					if (Cfg::comm_debug_ultra_verbose)
					{
						for (int c : in_msg)
						{
							std::cout << c << " ";
						}
						std::cout << std::endl;
					}

					ReqUtils::ResultCode result_code;
					auto req_uptr = Reqs::parse_req_str(in_msg, result_code);

					if (Cfg::req_debug)
					{
						std::cout << "New Req (" << ReqUtils::get_short_result_str(result_code) << "): ";
						if (req_uptr)
							std::cout << *req_uptr << std::endl;
						else
							std::cout << "nullptr" << std::endl;
					}

					std::ostringstream returned_contents;
					returned_contents << ReqUtils::get_short_result_str(result_code) << std::endl;
					if (req_uptr != nullptr)
					{
						req_uptr->serve(GlobalMsgQueue::get_inst(), returned_contents);
					}

					// TODO: send returned contents and status message back
					write(returned_contents.str());
					read();
				}

				read();
			});
	}

	void write(const std::string & str)
	{
		if (Cfg::comm_write_debug_ultra_verbose)
			std::cout << "Sending: " << str << std::endl;
		auto self(shared_from_this());
		boost::asio::async_write(
			_socket,
			boost::asio::buffer(str),
			[this, self](boost::system::error_code ec, std::size_t)
			{
				if (!ec)
				{

				}
			});
	}

	tcp::socket _socket;

	boost::asio::streambuf _in_buf;

};


class Server
{
public:



	Server(
		boost::asio::io_service& io_service,
		PortType port)
	:
		_acceptor(io_service, tcp::endpoint(tcp::v4(), port)),
		_io_service(io_service),
		_port(port)
	{

	}

	void run()
	{
		if (Cfg::comm_debug)
			std::cout << "Listing on port...\n";
		std::shared_ptr<Session> session( new Session(_io_service) );

		_acceptor.async_accept(
			session->get_socket(),
			[this, session](boost::system::error_code ec)
				{
					if (Cfg::comm_debug)
						std::cout << "accept ec: " << ec << std::endl;
					if (!ec)
					{
						session->start();
					}

					run();
				});
	}

private:

	tcp::acceptor _acceptor;
	boost::asio::io_service & _io_service;
	const PortType _port;
};


void start_server()
{
	GlobalMsgQueue::init();

	boost::asio::io_service io_service;

	Server s(io_service, Cfg::port);
	s.run();

	io_service.run();
}


