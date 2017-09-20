/*
	{{license}}
*/

#ifndef __JSONPACKET_H__
#define	__JSONPACKET_H__

#include <string>
#include <queue>

using namespace std;

class JSONPacketBuilder;
class JSONPacket {
	string m_msg;
	size_t m_len;
	friend class JSONPacketBuilder;
public:

	JSONPacket(const string& msg) : m_msg(msg) {}
	string Stringify() {
		string result;
		result += std::to_string(m_msg.size());
		result += ":";
		result += m_msg;
		return result;
	}

	static JSONPacket Parse(const string& jsonmsg) {

		JSONPacket packet("");
		size_t offset = jsonmsg.find(":");
		if (offset>0)
		{
			size_t packetlen = std::stol(jsonmsg.substr(0, offset + 1));
			packet.m_msg = jsonmsg.substr(offset + 1, packetlen); //be sure we get only one single packet form msg
			packet.m_len = packetlen;

		}

		return packet;

	}

	string GetMsg()
	{
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

	size_t Append(const string& m)
	{
		size_t diff = m_len - m_msg.size();
		if (diff)
		{
			
			m_msg.append(m.begin(), m.begin() + diff);
		
		}
		
		return diff;
	}

	bool IsEmpty() const
	{
		if (m_msg == "") return true;
		return false;
	}

	void Clean()
	{
		m_msg = "";
		m_len = 0;
	}


};

class JSONPacketBuilder {

	enum StatusType
	{
		eStopped,
		eRunning
	};

	StatusType m_status;
	string	   m_buffer;   //MAYBE: consider to use stringbuffer

	queue<JSONPacket> m_jsonpackets;
	JSONPacket m_lastpacket;
public:

	JSONPacketBuilder():m_status(eStopped),m_buffer(""),m_lastpacket("") {}


	/*
		return the remaining bytes to create a valid jsonpacket
		0 - means a new packet is ready to use
	*/
	size_t Append(const string& chunck) {

		if (chunck == "") return -1;
		if (m_status == eStopped)
		{
			m_status = eRunning;
			size_t offset = chunck.find(":");
			if (offset>0)
			{
				size_t packetlen = std::stol(chunck.substr(0, offset + 1));
				m_lastpacket.m_len = packetlen;
				if (chunck.size() >= packetlen + offset + 1)
				{
					//we got the hole msg
					m_lastpacket.m_msg = chunck.substr(offset + 1, packetlen);
					
					m_buffer = chunck.substr(offset + packetlen + 1);
					m_jsonpackets.push(m_lastpacket);
					m_lastpacket.Clean();
					return 0;
				}
				else
				{
					m_lastpacket.m_msg = chunck.substr(offset + 1);
					m_buffer = chunck.substr(offset + 1);
					return 0;
				}

			}
			else
			{
				//unreached area
			}
			
		}
		else if ( m_status == eRunning)
		{

		}
	}





};

#endif