/*
	{{license}}
*/

#ifndef __JSONPACKET_H__
#define	__JSONPACKET_H__
#include "config.h"


#include <string>

class JSONPacket_Impl;
class PUBLIC_API JSONPacket {

	JSONPacket_Impl* m_impl;
public:

	JSONPacket(JSONPacket_Impl* impl);

	virtual const std::string& Stringify() const;

	static JSONPacket Parse(const std::string& jsonmsg);
	
	virtual const std::string& GetMsg();
	
	virtual const std::string& GetMsg() const;
	
	size_t GetLen() const;

};

#endif