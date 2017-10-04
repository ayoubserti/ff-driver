
#include <string>
#include "JSONPacket.h"
#include "JSONPacket_Impl.h"

JSONPacket::JSONPacket(JSONPacket_Impl* impl)
	:m_impl(impl)
{

}

const std::string& JSONPacket::Stringify() const
{
	return m_impl->Stringify();
}

JSONPacket JSONPacket::Parse(const std::string & jsonmsg)
{
	return JSONPacket_Impl::Parse(jsonmsg);
}

const string & JSONPacket::GetMsg()
{
	return m_impl->GetMsg();
}

const string & JSONPacket::GetMsg() const
{
	return m_impl->GetMsg();
}

size_t JSONPacket::GetLen() const
{
	return m_impl->GetLen();
}

