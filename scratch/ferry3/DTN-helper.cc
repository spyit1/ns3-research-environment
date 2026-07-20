/*
 * simple-helper.cc
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#include "DTN-helper.h"

#include "DTN-protocol.h"
#include "ns3/node-list.h"
#include "ns3/names.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-list-routing.h"

namespace ns3
{
DTNHelper::DTNHelper() : Ipv4RoutingHelper()
{
	m_agentFactory.SetTypeId("ns3::simple::RoutingProtocol");
}

DTNHelper* DTNHelper::Copy(void) const
{
	return new DTNHelper(*this);
}

Ptr<Ipv4RoutingProtocol> DTNHelper::Create(Ptr<Node> node) const
{
	Ptr<simple::RoutingProtocol> agent=m_agentFactory.Create<simple::RoutingProtocol>();
	node->AggregateObject(agent);
	return agent;
}

void DTNHelper::SetAttribute(std::string name,const AttributeValue &value)
{
	m_agentFactory.Set(name,value);
}

int64_t DTNHelper::AssignStreams(NodeContainer c,int64_t stream)
{
	int64_t currentStream=stream;
	Ptr<Node> node;
	for(NodeContainer::Iterator i=c.Begin();i!=c.End();++i){
		node=(*i);
		Ptr<Ipv4> ipv4=node->GetObject<Ipv4>();
		NS_ASSERT_MSG(ipv4,"Ipv4 not installed on node");
		Ptr<Ipv4RoutingProtocol> simple=ipv4->GetRoutingProtocol();
		NS_ASSERT_MSG(simple,"Ipv4 routing not installed on node");
		Ptr<simple::RoutingProtocol> mysimple=DynamicCast<simple::RoutingProtocol>(simple);
		if(simple){
			currentStream+=mysimple->AssignStreams(currentStream);
			continue;
		}
		//simple may also be in a list
		Ptr<Ipv4ListRouting> list=DynamicCast<Ipv4ListRouting>(simple);
		if(list){
			int16_t priority;
			Ptr<Ipv4RoutingProtocol> listSimple;
			Ptr<simple::RoutingProtocol> listsimple;
			for(uint32_t i=0;i<list->GetNRoutingProtocols();i++){
				listSimple=list->GetRoutingProtocol(i,priority);
				listsimple=DynamicCast<simple::RoutingProtocol>(listSimple);
				if(listsimple){
					currentStream+=listsimple->AssignStreams(currentStream);
					break;
				}
			}
		}
	}
	return (currentStream-stream);
}

}


