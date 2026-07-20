/*
 * simple-helper.h
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#ifndef DTN_HELPER_H_
#define DTN_HELPER_H_

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3
{
	class DTNHelper : public Ipv4RoutingHelper
	{
	public:
		DTNHelper();

		DTNHelper* Copy(void) const;
		virtual Ptr<Ipv4RoutingProtocol> Create(Ptr<Node> node) const;
		void SetAttribute(std::string name,const AttributeValue &value);
		int64_t AssignStreams (NodeContainer c,int64_t stream);

	private:
		ObjectFactory m_agentFactory;
	};
}

#endif /* SIMPLE_HELPER_H_ */
