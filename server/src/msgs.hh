#ifndef MSGS_HH
#define MSGS_HH

#include <vector>
#include <deque>
#include <string>
#include <memory>
#include <exception>
#include <mutex>
#include <iostream>

namespace Cfg
{
	static const bool msg_q_debug = true;
};

class Msg
{

public:
	typedef std::string MsgBody;
	typedef std::string Username;

	enum class Priority : unsigned
	{
		Min = 0,
		Debug = Min,
		Info,
		Warning,
		Critical,
		Error,
		Illegal,
		Max = Illegal
		// If you update this, don't forget to update the corresponding strings in msgs.cc
	};

	class InvalidPriorityException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "Invalid priority exception";
		}
	};

	static Priority get_priority_from_str(const std::string & priority_str)
	{
		Priority pri = Priority::Illegal;

		for (unsigned i = 0; i < _priority_strings.size(); ++i)
		{
			if (priority_str == _priority_strings[i])
			{
				pri = static_cast<Priority> (i);
				break;
			}
		}

		return pri;

	}

	static const char * get_priority_str(Priority pri)
	{
		return _priority_strings[static_cast<size_t>(pri)];
	};

	Priority get_priority() const
	{
		return _priority;
	}

	bool operator==(const Msg & b) const
	{
		return
			(this->_priority == b._priority) &&
			(this->_username == b._username) &&
			(this->_body == b._body);
	}

	// Returning by value for thread safety for now.
	// TODO: add ref version when needed.
	Username get_username() const
	{
		return _username;
	}

	Priority get_pri() const
	{
		return _priority;
	}

	MsgBody get_msg() const
	{
		return _body;
	}

	Msg(MsgBody body, Username username, Priority priority)
	:	_body(body),
		_username(username),
		_priority(priority)
	{
		if (_priority == Priority::Illegal)
		{
			throw InvalidPriorityException();
		}
	}

	friend std::ostream & operator<<(std::ostream & os, const Msg & msg)
	{
		os << "u[" << msg._username << "] p[" << Msg::get_priority_str(msg._priority) << "] m[" << msg._body << "]";
		return os;
	}


private:
	// N.B. If you modify member elements here, you should also update
	// the mathematical operators!
	MsgBody _body;
	Username _username;
	Priority _priority;

	static const std::vector<const char *> _priority_strings;
};


class MsgQueueWrapper
{
public:

	typedef std::deque<Msg> Queue;
	typedef std::mutex Mutex;
	typedef std::unique_lock<Mutex> Lock;

	Queue & get_queue()
	{
		return _queue;
	}

	Lock get_lock()
	{
		return Lock(_mutex);
	}

	void dump_to_stream(std::ostream & os, Msg::Priority priority_cap = Msg::Priority::Debug) const
	// Thread unsafe! Caller's responsibility ;)
	{
		for (const auto & msg : _queue)
		{
			if (static_cast<unsigned>(msg.get_priority()) >= static_cast<unsigned>(priority_cap))
			{
				os << msg << std::endl;
			}
		}
	}

private:

	Queue _queue;
	Mutex _mutex;

};


class GlobalMsgQueue
// The class that's supposed to be used as a global.
{
public:
	static void init()
	{
		if (Cfg::msg_q_debug)
		{
			std::cout << "Initialized GlobalMsgQueue!\n";
		}
		_inst = std::unique_ptr<MsgQueueWrapper>(new MsgQueueWrapper);
	}

	static MsgQueueWrapper & get_inst()
	{
		return *_inst;
	}

	static MsgQueueWrapper::Queue & get_queue()
	{
		return _inst->get_queue();
	}

	static MsgQueueWrapper::Lock get_lock()
	{
		return _inst->get_lock();
	}

private:
	static std::unique_ptr<MsgQueueWrapper> _inst;

};

#endif
