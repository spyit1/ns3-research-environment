/*
 * section-data-trade.cc
 *
 *  Created on: Jan 12, 2021
 *      Author: matsunaga
 */



#include "section-data-trade.h"


namespace ns3 {
namespace simple {

void StoreSectionData::Print(){
	m_cml.Print();
}

//void StoreSectionData::SetClusterMemberList(ClusterMemberList cml){
//	ClusterMemberList::iterator cmli;
//	m_cmlSize=cml.size();
//	for(cmli=cml.begin();cmli!=cml.end();cmli++){
//		std::pair<Ipv4Address,ClusterMember> pair=*cmli;
//		ClusterMember cm=pair.second;
//
//		m_memberId.push_back(pair.first);
//		m_memberHop.push_back(cm.GetHop());
//		m_memberChildren.push_back(cm.GetCoordinatehildren());
//		m_memberCellular.push_back(cm.GetCoordinateellularId());
//
//		m_memberX.push_back(cm.GetX());
//		m_memberY.push_back(cm.GetY());
//	}
//}

//ClusterMemberList StoreSectionData::GetCoordinatelusterMemberList(){
//	ClusterMemberList cml;
//
//	std::list<Ipv4Address>::iterator midli;
//	std::list<uint8_t>::iterator mhopli;
//	std::list<uint8_t>::iterator mchildrenli;
//	std::list<uint32_t>::iterator mcellularli;
//
//	std::list<uint32_t>::iterator mxli;
//	std::list<uint32_t>::iterator myli;
//
//	for(midli=m_memberId.begin(),mhopli=m_memberHop.begin(),mchildrenli=m_memberChildren.begin(),mcellularli=m_memberCellular.begin(),mxli=m_memberX.begin(),myli=m_memberY.begin();
//			midli!=m_memberId.end();
//			midli++,mhopli++,mchildrenli++,mcellularli++,mxli++,myli++){
//		ClusterMember cm;
//		cm.SetHop(*mhopli);
//		cm.SetChildren(*mchildrenli);
//		cm.SetCellularId(*mcellularli);
//
//		cm.SetX(*mxli);
//		cm.SetY(*myli);
//
//		cml[*midli]=cm;
//	}
//
//	return cml;
//}
}
}
