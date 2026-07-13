/*
 * cluster-list.cc
 *
 *  Created on: 2015/10/20
 *      Author: terami
 */
#include "cluster-list.h"

#include <iostream>

namespace ns3{
namespace simple{

void ClusterMember::Print(){
	std::cout<<","<<(int)m_hop<<","<<(int)m_children<<","<<m_cellularId<<std::endl;
}
/*
CellularInfoMap ClusterMemberList::MakeCellularInfoMap(){
	CellularInfoMap cim;
	ClusterMemberList::iterator cmli;
	for(cmli=this->begin();cmli!=this->end();cmli++){
		std::pair<Ipv4Address,ClusterMember> pair=*cmli;
		ClusterMember cm=pair.second;
		uint32_t id=cm.GetCellularId();

		if(cim.Find(id)){
			uint8_t num=cim[id]+1;
			cim[id]=num;
		}else{
			cim[id]=1;
		}
	}
	return cim;
}



void ClusterMemberList::Print(){
	std::cout<<"================================"<<std::endl;
	std::cout<<"Member: Id,Hop,Children,Cellular"<<std::endl;
	std::cout<<"================================"<<std::endl;

	ClusterMemberList::iterator cmli;
	for(cmli=this->begin();cmli!=this->end();cmli++){
		std::pair<Ipv4Address,ClusterMember> pair=*cmli;
		std::cout<<pair.first;
		pair.second.Print();
	}
	std::cout<<"================================"<<std::endl;
}

std::unordered_set<Ipv4Address,Ipv4Address::Hash> ClusterMemberList::GetMembers(){
	std::unordered_set<Ipv4Address,Ipv4Address::Hash> members;
	ClusterMemberList::iterator cmli;

	for(cmli=this->begin();cmli!=this->end();cmli++){
		std::pair<Ipv4Address,ClusterMember> pair=*cmli;
		members.emplace(pair.first);
	}

	return members;
}

void NeighborCluster::Print(){
	std::cout<<","<<(int)m_size<<std::endl;
}


std::unordered_set<Ipv4Address,Ipv4Address::Hash> NeighborClusterList::GetNeighbors(){
	std::unordered_set<Ipv4Address,Ipv4Address::Hash> neighbors;
	NeighborClusterList::iterator ncli;

	for(ncli=this->begin();ncli!=this->end();ncli++){
		std::pair<Ipv4Address,NeighborCluster> pair=*ncli;
		neighbors.emplace(pair.first);
	}

	return neighbors;
}

void NeighborClusterList::Print(){
	std::cout<<"=========================="<<std::endl;
	std::cout<<"Neighbor: Id,Size,Cellular"<<std::endl;
	std::cout<<"=========================="<<std::endl;

	NeighborClusterList::iterator ncli;
	for(ncli=this->begin();ncli!=this->end();ncli++){
		std::pair<Ipv4Address,NeighborCluster> pair=*ncli;
		std::cout<<pair.first;
		pair.second.Print();
	}
	std::cout<<"=========================="<<std::endl;
}

void SuperPeerInfo::Print(){
	std::unordered_set<Ipv4Address,Ipv4Address::Hash>::iterator usi;

	std::cout<<"Member:"<<std::endl;
	for(usi=m_ml.begin();usi!=m_ml.end();usi++){
		std::cout<<*usi<<std::endl;
	}
	std::cout<<"Neighbor:"<<std::endl;
	for(usi=m_nl.begin();usi!=m_nl.end();usi++){
		std::cout<<*usi<<std::endl;
	}
}

std::unordered_set<Ipv4Address,Ipv4Address::Hash> SuperPeerInfoList::GetSuperPeers(){
	std::unordered_set<Ipv4Address,Ipv4Address::Hash> superpeers;
	SuperPeerInfoList::iterator spili;

	for(spili=this->begin();spili!=this->end();spili++){
		std::pair<Ipv4Address,SuperPeerInfo> pair=*spili;
		superpeers.emplace(pair.first);
	}

	return superpeers;
}

void SuperPeerInfoList::EraseSuperPeerInfo(std::unordered_set<Ipv4Address,Ipv4Address::Hash> ids){
	std::unordered_set<Ipv4Address,Ipv4Address::Hash>::iterator usi;
	for(usi=ids.begin();usi!=ids.end();usi++){
		this->erase(*usi);
	}
}

Ipv4Address SuperPeerInfoList::GetPriorSuperPeer(){
	SuperPeerInfoList::iterator spili;
	Ipv4Address prior;
	int maxMember=-1;
	int maxNeighbor=-1;

	for(spili=this->begin();spili!=this->end();spili++){
		std::pair<Ipv4Address,SuperPeerInfo> pair=*spili;
		int numMember=pair.second.GetMemberList().size();
		int numNeighbor=pair.second.GetNeighborList().size();
		if(maxMember<numMember){
			prior=pair.first;
			maxMember=numMember;
			maxNeighbor=numNeighbor;
		}else if(maxMember==numMember){
			if(maxNeighbor<numNeighbor){
				prior=pair.first;
				maxMember=numMember;
				maxNeighbor=numNeighbor;
			}
		}
	}

	return prior;
}

void SuperPeerInfoList::Print(){
	SuperPeerInfoList::iterator spili;

	std::cout<<"=========="<<std::endl;
	for(spili=this->begin();spili!=this->end();spili++){
		std::pair<Ipv4Address,SuperPeerInfo> pair=*spili;
		std::cout<<pair.first<<std::endl;
		std::cout<<"----------"<<std::endl;
		pair.second.Print();
		std::cout<<"----------"<<std::endl;
	}
	std::cout<<"**********"<<std::endl;

}
*/

void DataKey::Print(){
	std::cout<<m_id<<","<<m_seqNo<<std::endl;
}

void DataKeyList::Print(){
	std::cout<<"========================="<<std::endl;
	std::cout<<"DataKeyList: Source,SeqNo"<<std::endl;
	std::cout<<"========================="<<std::endl;

	DataKeyList::iterator dkli;

	for(dkli=this->begin();dkli!=this->end();dkli++){
		DataKey dk=*dkli;
		dk.Print();
	}
	std::cout<<"========================="<<std::endl;
}

bool DataKeyList::GetDiffer(DataKeyList &dkl_differ,DataKeyList dkl_you){
	DataKeyList::iterator dkli;

	bool differ=false;

	for(dkli=dkl_you.begin();dkli!=dkl_you.end();dkli++){
		if(this->find(*dkli)==this->end()){
			this->emplace(*dkli);
			dkl_differ.emplace(*dkli);
			differ=true;
		}
	}

	return differ;
}


}
}
