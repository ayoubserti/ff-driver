#include "Network.h"
#include "Network_Impl.h"

Endpoint::Endpoint(INetworkDelegate & delegator, Endpoint_Impl * impl)
	:m_delagate(delegator)
	,m_impl(impl)
{
}

void Endpoint::Start()
{
	m_impl->Start();
	
}

void Endpoint::Send(const JSONPacket & packet)
{
	m_impl->Send(packet);
}




