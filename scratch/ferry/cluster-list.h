/*
 * cluster-list.h
 *
 *  Created on: 2015/10/20
 *      Author: terami
 */

#ifndef CLUSTER_LIST_H_
#define CLUSTER_LIST_H_

#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/spectrum-value.h"
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <vector>
#include <fstream>
#include <math.h>
#include <string>
#include <sstream>
#include <utility>

#define MASTER 0
#define SLAVE 1
#define NONE 2

namespace ns3{
namespace simple{

class CellularInfoMap : public std::unordered_map<uint32_t,uint8_t>{
public:
	void Print();
	bool Find(uint32_t cellularId){if(this->find(cellularId)!=this->end()){return true;}else{return false;}};
};



class ClusterMember{
public:
	ClusterMember(){m_hop=0;m_children=0;m_cellularId=0;m_x=0;m_y=0;}
	ClusterMember(uint8_t hop,uint8_t children,uint32_t cellularId,uint32_t x,uint32_t y){
		m_hop=hop;
		m_children=children;
		m_cellularId=cellularId;
		m_x=x;
		m_y=y;
	}

	void SetHop(uint8_t hop){m_hop=hop;}
	uint8_t GetHop(){return m_hop;}

	void SetChildren(uint8_t children){m_children=children;}
	uint8_t GetCoordinatehildren(){return m_children;}

	void SetCellularId(uint32_t cllularId){m_cellularId=cllularId;}
	uint32_t GetCoordinateellularId(){return m_cellularId;}

	void SetX(uint32_t x){m_x=x;}
	uint32_t GetX(){return m_x;}

	void SetY(uint32_t y){m_y=y;}
	uint32_t GetY(){return m_y;}


	void Print();
	std::string Output();

private:
	uint8_t m_hop;
	uint8_t m_children;
	uint32_t m_cellularId;
	uint32_t m_x;
	uint32_t m_y;
};




class ClusterMemberList : public std::map<Ipv4Address,ClusterMember>{
public:
	void AddMember(Ipv4Address id,ClusterMember cm){this->operator[](id)=cm;}
	void EraseMember(Ipv4Address id){this->erase(id);}
	void ClearList(){this->clear();}
	std::set<Ipv4Address> GetMembers();
	void Print();
	bool Find(Ipv4Address key){if(this->find(key)!=this->end()){return true;}else{return false;}}
	CellularInfoMap MakeCellularInfoMap();
	std::string Output();
//	ClusterMember GetCoordinatelusterMember{};
};


class ClusterAllMemberList : public std::map<Ipv4Address,ClusterMemberList>{
public:
	void AddList(Ipv4Address cid,ClusterMemberList cml){this->operator[](cid)=cml;}
	void EraseList(Ipv4Address cid){this->erase(cid);}
	std::set<ClusterMemberList> GetLists();
	void ClearList(){this->clear();}
	void Print();
	bool Find(Ipv4Address key){if(this->find(key)!=this->end()){return true;}else{return false;}}

};



class NeighborCluster{
public:
	NeighborCluster(){m_size=1;}
	NeighborCluster(uint8_t size){
		m_size=size;
	}

	void SetSize(uint8_t size){m_size=size;}
	uint8_t GetSize(){return m_size;}

	void Print();
private:
	uint8_t m_size;
};



class NeighborClusterList : public std::map<Ipv4Address,NeighborCluster> {
public:
	void AddNeighbor(Ipv4Address id,NeighborCluster nc){this->operator[](id)=nc;}
	void EraseNeighbor(Ipv4Address id){this->erase(id);}
	void ClearList(){this->clear();}
	std::set<Ipv4Address> GetNeighbors();
	void Print();
	bool Find(Ipv4Address key){if(this->find(key)!=this->end()){return true;}else{return false;}}
};


class SuperPeerInfo{
public:
	SuperPeerInfo(){}
	SuperPeerInfo(std::set<Ipv4Address> ml,std::set<Ipv4Address> nl){
		m_ml=ml;
		m_nl=nl;
	}

	void SetMemberList(std::set<Ipv4Address> ml){m_ml=ml;}
	std::set<Ipv4Address> GetMemberList(){return m_ml;}

	void SetNeighborList(std::set<Ipv4Address> nl){m_nl=nl;}
	std::set<Ipv4Address> GetNeighborList(){return m_nl;}

	void Print();
private:
	std::set<Ipv4Address> m_ml;
	std::set<Ipv4Address> m_nl;
};


class SuperPeerInfoList : public std::map<Ipv4Address,SuperPeerInfo>{
public:
	void AddSuperPeerInfo(Ipv4Address id,SuperPeerInfo spi){this->operator[](id)=spi;}
	void EraseSuperPeerInfo(Ipv4Address id){this->erase(id);}
	void EraseSuperPeerInfo(std::set<Ipv4Address> ids);
	void ClearList(){this->clear();}
	Ipv4Address GetPriorSuperPeer();
	std::set<Ipv4Address> GetSuperPeers();
	void Print();
	bool Find(Ipv4Address key){if(this->find(key)!=this->end()){return true;}else{return false;}}
};




class DataKey{
public:
	struct Hash;
	DataKey(){m_id=Ipv4Address();m_seqNo=0;}
	DataKey(Ipv4Address ad,uint32_t no);
	bool operator==(const DataKey& rhs) const;
	bool operator!=(const DataKey& rhs) const;

	void SetId(Ipv4Address id){m_id=id;}
	Ipv4Address GetId(){return m_id;}

	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}

	void Print();

private:
	Ipv4Address m_id;
	uint32_t m_seqNo;
};

inline DataKey::DataKey(Ipv4Address id,uint32_t seqNo){
	m_id=id;
	m_seqNo=seqNo;
}

inline bool DataKey::operator==(const DataKey& rhs) const{
	const DataKey& lhs = *this;
	return lhs.m_id==rhs.m_id && lhs.m_seqNo==rhs.m_seqNo;
}

inline bool DataKey::operator!=(const DataKey& rhs) const{
	return !(this->operator==(rhs));
}

struct DataKey::Hash{
	typedef std::size_t result_type;

	std::size_t operator()(const DataKey& key) const;
};

inline std::size_t DataKey::Hash::operator()(const DataKey& key) const {
	std::string bytes(reinterpret_cast<const char*>(&key),sizeof(DataKey));
	return std::hash<std::string>()(bytes);
}


class DataKeyList : public std::unordered_set<DataKey,DataKey::Hash> {
public:
	void AddKey(DataKey dk){this->emplace(dk);}
	void EraseKey(DataKey dk){this->erase(dk);}
	void ClearList(){this->clear();}
	void Print();
	bool GetDiffer(DataKeyList &dkl_differ,DataKeyList dkl_you);
};

}
}



#endif /* CLUSTER_LIST_H_ */
