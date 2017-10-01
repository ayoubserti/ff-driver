#ifndef __JSONPACKET_IMPL_H__
#define __JSONPACKET_IMPL_H__
#include <string>
#include "JSONPacket.h"

using namespace std;

class JSONPacket_Impl : public JSONPacket
{

	std::string m_msg;
	size_t m_len;
	mutable std::string m_stringified;

public:
	JSONPacket_Impl(const std::string& msg) :JSONPacket(this),
		m_msg(msg),
		m_len(0),
		m_stringified("") {}


	const std::string& Stringify() const {
		if (m_stringified == "")
		{
			std::string result;
			result += std::to_string(m_msg.size());
			result += ":";
			result += m_msg;
			m_stringified = result;
		}
		return m_stringified;
	}

	static JSONPacket_Impl Parse(const std::string& jsonmsg) {

		JSONPacket_Impl packet("");
		size_t offset = jsonmsg.find(":");
		if (offset > 0)
		{
			size_t packetlen = std::stol(jsonmsg.substr(0, offset + 1));
			packet.m_msg = jsonmsg.substr(offset + 1, packetlen); //be sure we get only one single packet form msg
			packet.m_len = packetlen;

		}
		return packet;
	}

	virtual const string& GetMsg() {
		return m_msg;
	}

	const string& GetMsg() const
	{
		return m_msg;
	}


	size_t GetLen() const
	{
		return m_len;
	}

};




#endif