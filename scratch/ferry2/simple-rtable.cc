/*
 * simple-rtable.cc
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#include "simple-rtable.h"

#include <algorithm>
#include <iomanip>
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <iostream>

NS_LOG_COMPONENT_DEFINE("SimpleRTable");

namespace ns3
{
namespace simple
{
//経路エントリークラスのコンストラクタ
RoutingTableEntry::RoutingTableEntry(Ptr<NetDevice> dev,
		Ipv4Address dst,
		Ipv4InterfaceAddress iface,
		Ipv4Address nextHop,
		uint16_t metrics,
		std::string rtype,
		std::string iftype,
		Time lifetime):
									m_metrics(metrics),
									m_iface(iface),
									m_rtype(rtype),
									m_iftype(iftype),
									m_lifetime(lifetime+Simulator::Now()),
									m_flag(VALID)
{
	//経路情報を設定する
	m_ipv4Route=Create<Ipv4Route>();
	m_ipv4Route->SetSource(m_iface.GetLocal());
	m_ipv4Route->SetDestination(dst);
	m_ipv4Route->SetGateway(nextHop);
	m_ipv4Route->SetOutputDevice(dev);
}

RoutingTableEntry::~RoutingTableEntry()
{
	//...
}


//経路エントリーをファイルに出力する
void RoutingTableEntry::Print(Ptr<OutputStreamWrapper> stream) const
{
	std::ostream* os=stream->GetStream();
	std::ostringstream gw,local,dst;
	gw << m_ipv4Route->GetGateway();
	local << m_iface.GetLocal();
	dst << m_ipv4Route->GetDestination();
	*os << m_rtype << "\t"
			<< std::setiosflags(std::ios::left)
	<< std::setw(16) << dst.str()
	<< std::setw(16) << gw.str()
	<< std::setw(16) << local.str()
	<< std::setw(8) << int(m_metrics)
	<< std::setw(16) << m_iftype
	<< std::setprecision(5) << (m_lifetime-Simulator::Now()).GetSeconds()
	<< "\n";
}

//経路エントリーを画面に表示する
void RoutingTableEntry::Print() const
{
	std::cout << "\nPROTO Routing table(Flags: C-directly connected, P-Proto S-Static)\n"
			<< "Flag\tDestination\tGateway\t\tInterface\tHopa\tI/F Type\tExpire\n";

	std::ostringstream gw,local,dst;
	gw << m_ipv4Route->GetGateway();
	local << m_iface.GetLocal();
	dst << m_ipv4Route->GetDestination();

	std::cout << m_rtype << "\t"
			<< std::setiosflags(std::ios::left)
	<< std::setw(16) << dst.str()
	<< std::setw(16) << gw.str()
	<< std::setw(16) << local.str()
	<< std::setw(8) << int(m_metrics)
	<< std::setw(16) << m_iftype
	<< std::setprecision(5) << (m_lifetime-Simulator::Now()).GetSeconds()
	<< std::endl;
}


//ルーティングテーブルクラスのコントラスタ
RoutingTable::RoutingTable()
{
	//クラスの初期化
}

bool RoutingTable::LookupRoute(Ipv4Address dst,RoutingTableEntry &rt)
{
	//std::cout<<"LookupRoute"<<std::endl;
	NS_LOG_FUNCTION(this << dst);

	//あて先dstの経路エントリーを検索する
	if(m_RoutingTable.empty()){
		NS_LOG_LOGIC("Route to " << dst << " not found; m_RoutingTable is empty");
		return false;
	}
	std::map<Ipv4Address,RoutingTableEntry>::const_iterator i=m_RoutingTable.find(dst);
	if(i==m_RoutingTable.end()){
		NS_LOG_LOGIC("Route to " << dst << " not found");
		return false;
	}
	rt=i->second;
	NS_LOG_LOGIC("Route to " << dst << " found");
	return true;
}

bool
RoutingTable::LookupValidRoute (Ipv4Address id, RoutingTableEntry & rt)
{
	NS_LOG_FUNCTION (this << id);
	if (!LookupRoute (id, rt))
	{
		NS_LOG_LOGIC ("Route to " << id << " not found");
		return false;
	}
	NS_LOG_LOGIC ("Route to " << id << " flag is " << ((rt.GetFlag () == VALID) ? "valid" : "not valid"));
	return (rt.GetFlag () == VALID);
}

bool RoutingTable::DeleteRoute(Ipv4Address dst)
{
	NS_LOG_FUNCTION(this << dst);

	//あて先dstの経路エントリーを削除する
	if(m_RoutingTable.erase(dst)!=0){
		NS_LOG_LOGIC("Route deletion to " << dst << " successful");
		return true;
	}
	NS_LOG_LOGIC("Route deletion to " << dst << " not successful");
	return false;
}

bool RoutingTable::AddRoute(RoutingTableEntry &rt)
{
	NS_LOG_FUNCTION(this);

	//経路エントリrtをルーティングテーブルに挿入する
	std::pair<std::map<Ipv4Address,RoutingTableEntry>::iterator,bool> result=m_RoutingTable.insert(std::make_pair(rt.GetDestination(),rt));
	return result.second;
}

bool RoutingTable::Update(RoutingTableEntry &rt)
{
	NS_LOG_FUNCTION(this);

	//ルーティングテーブルを更新する
	std::map<Ipv4Address,RoutingTableEntry>::iterator i=m_RoutingTable.find(rt.GetDestination());
	if(i==m_RoutingTable.end()){
		NS_LOG_LOGIC("Route update to " << rt.GetDestination() << " fails; not found");
		return false;
	}
	i->second=rt;
	return true;
}

void RoutingTable::Print(Ptr<OutputStreamWrapper> stream) const
{
	//ルーティングテーブル全体をファイルに出力する
	std::map<Ipv4Address,RoutingTableEntry> table=m_RoutingTable;
	*stream->GetStream() << "\nPROTO Routing table(Flags: C-directly connected,P-Proto S-Static)\n"
			<< "Flag\tDestination\tGateway\t\tInterface\tHops\tI/F Type\tExpire\n";
	for(std::map<Ipv4Address,RoutingTableEntry>::const_iterator i=table.begin();i!=table.end();++i)
	{
		i->second.Print(stream);
	}
	*stream->GetStream() << "\n";
}

void RoutingTable::Print(){
	std::map<Ipv4Address,RoutingTableEntry> table;
	table=m_RoutingTable;
	std::cout << "\nPROTO Routing table(Flags: C-directly connected,P-Proto S-Static)\n"
			<< "Flag\tDestination\tGateway\t\tInterface\tHops\tI/F Type\tExpire\n";
	for(std::map<Ipv4Address,RoutingTableEntry>::const_iterator i=table.begin();i!=table.end();++i)
	{
		i->second.Print();
	}
	std::cout << "\n";
}

}
}


