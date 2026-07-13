/*
 * simple-packet.h
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#ifndef SIMPLE_PACKET_H_
#define SIMPLE_PACKET_H_

#include <iostream>
#include <map>
#include <list>
#include <unordered_map>
#include <vector>

#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "cluster-list.h"

#define HOGEHOGE 0
#define ADHOC_CLUSTER 5

namespace ns3
{
namespace simple
{
//必要なパケットを識別するための識別子を定義しておく
enum MessageType
{
	TYPE_MEP=1,
	TYPE_MAP=2,
	TYPE_MQP=3,
	TYPE_MQAP=4,
	TYPE_RNCP=5,
	TYPE_CNCP=6,
	TYPE_DATA=7,
	TYPE_ROUTEREQUEST=10,
	TYPE_ROUTEREPLY=11,
	TYPE_ACK=12,
	TYPE_HELLO=13,
	TYPE_DATAACK=20
};

class TypeHeader : public Header
{
public:
	TypeHeader(MessageType t=TYPE_ROUTEREQUEST);

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;
	uint32_t GetSerializedSize() const; //カプセル化後のヘッダサイズを取得する関数
	void Serialize(Buffer::Iterator start) const; //ヘッダ部のカプセル化関数
	uint32_t Deserialize(Buffer::Iterator start); //ヘッダ部のデカプセル化関数
	void Print(std::ostream &os) const;

	//プロトコルパケットのタイプ識別子を取得する関数
	MessageType Get() const { return m_type; }
	bool IsValid () const { return m_valid; }

	bool operator==(TypeHeader const & o) const;

private:
	MessageType m_type;
	bool m_valid;
};
std::ostream &operator << (std::ostream &os,TypeHeader const &h);

class Mep : public Header
{
public:
	Mep(){m_hop=0;m_size=1;m_ttl=4;m_cellularId=0;m_mepId=0;}
	Mep(Ipv4Address cid,uint8_t hop,uint8_t size,uint8_t ttl,uint32_t cellularId,uint32_t mepId){
		m_cid=cid;
		m_hop=hop;
		m_size=size;
		m_ttl=ttl;
		m_cellularId=cellularId;
		m_mepId=mepId;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetHopCount (uint8_t hop) { m_hop = hop; }
	uint8_t GetHopCount () const { return m_hop; }

	void SetCid (Ipv4Address cid) { m_cid = cid; }
	Ipv4Address GetCid () const { return m_cid; }

	void SetSize(uint8_t size){m_size=size;}
	uint8_t GetSize(){return m_size;}

	void SetTtl(uint8_t ttl){m_ttl=ttl;}
	uint8_t GetTtl(){return m_ttl;}

	void SetCellularId (uint32_t cellularId) { m_cellularId = cellularId; }
	uint32_t GetCellularId () const { return m_cellularId; }

	void SetId (uint32_t mepId) { m_mepId = mepId; }
	uint32_t GetId () const { return m_mepId; }

	bool operator==(Mep const &o) const;

private:
	mutable uint32_t m_serialSize;
	Ipv4Address    m_cid;               ///< Cluster ID
	uint8_t        m_hop;       ///< Hop Count
	uint8_t m_size;
	uint8_t m_ttl;
	uint32_t m_cellularId;
	uint32_t m_mepId;
};
std::ostream &operator << (std::ostream &os,Mep const &);

//class Map : public Header
//{
//public:
//	Map(){m_cmlSize=0;m_nclSize=0;}
//	Map(ClusterMemberList cml,NeighborClusterList ncl){
//		this->SetClusterMemberList(cml);
//		this->SetNeighborClusterList(ncl);
//	}
//
//	static TypeId GetTypeId();
//	TypeId GetInstanceTypeId() const;
//
//	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
//	uint32_t GetSerializedSize() const;
//
//	void Serialize(Buffer::Iterator start) const;
//	uint32_t Deserialize (Buffer::Iterator start);
//	void PrintList() const;
//	void Print (std::ostream &os) const;
//	bool operator==(Map const &o) const;
//
//	void SetClusterMemberList(ClusterMemberList cml);
//	ClusterMemberList GetClusterMemberList();
//
//	void SetNeighborClusterList(NeighborClusterList ncl);
//	NeighborClusterList GetNeighborClusterList();
//
//private:
//	mutable uint32_t m_serialSize;
//
//	uint8_t m_cmlSize;
//	std::list<Ipv4Address> m_memberId;
//	std::list<uint8_t> m_memberHop;
//	std::list<uint8_t> m_memberChildren;
//	std::list<uint32_t> m_memberCellular;
//
//	uint8_t m_nclSize;
//	std::list<Ipv4Address> m_neighborId;
//	std::list<uint8_t> m_neighborSize;
//
//};
//std::ostream &operator << (std::ostream &os,Map const &);


class Mqp : public Header
{
public:
	Mqp(){}
	Mqp(Ipv4Address source,Ipv4Address dst,uint8_t size){
		m_source=source;
		m_dst=dst;
		m_size=size;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetSource(Ipv4Address source){m_source=source;}
	Ipv4Address GetSource(){return m_source;}

	void SetDst(Ipv4Address dst){m_dst=dst;}
	Ipv4Address GetDst(){return m_dst;}

	void SetSize(uint8_t size){m_size=size;}
	uint8_t GetSize(){return m_size;}

	bool operator==(Mqp const &o) const;

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_source;
	Ipv4Address m_dst;
	uint8_t m_size;
};
std::ostream &operator << (std::ostream &os,Mqp const &);


class Mqap : public Header
{
public:
	Mqap(){m_newHop=0;m_mqapId=0;}
	Mqap(Ipv4Address newCid,Ipv4Address preCid,uint8_t newHop,uint8_t size,uint32_t mqapId){
		m_newCid=newCid;
		m_preCid=preCid;
		m_newHop=newHop;
		m_size=size;
		m_mqapId=mqapId;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetNewCid(Ipv4Address newCid){m_newCid=newCid;}
	Ipv4Address GetNewCid(){return m_newCid;}

	void SetPreCid(Ipv4Address preCid){m_preCid=preCid;}
	Ipv4Address GetPreCid(){return m_preCid;}

	void SetNewHop(uint8_t newHop){m_newHop=newHop;}
	uint8_t GetNewHop(){return m_newHop;}

	void SetSize(uint8_t size){m_size=size;}
	uint8_t GetSize(){return m_size;}

	void SetId (uint32_t mqapId) { m_mqapId = mqapId; }
	uint32_t GetId () const { return m_mqapId; }

	bool operator==(Mqap const &o) const;

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_newCid;
	Ipv4Address m_preCid;
	uint8_t m_newHop;
	uint8_t m_size;
	uint32_t m_mqapId;
};
std::ostream &operator << (std::ostream &os,Mqap const &);

class Rncp : public Header
{
public:
	Rncp(){}
	Rncp(Ipv4Address currentCid,Ipv4Address candidateCid){
		m_currentCid=currentCid;
		m_candidateCid=candidateCid;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetCurrentCid(Ipv4Address currentCid){m_currentCid=currentCid;}
	Ipv4Address GetCurrentCid(){return m_currentCid;}

	void SetCandidateCid(Ipv4Address candidateCid){m_candidateCid=candidateCid;}
	Ipv4Address GetCandidateCid(){return m_candidateCid;}

	bool operator==(Rncp const &o) const;

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_currentCid;
	Ipv4Address m_candidateCid;
};
std::ostream &operator << (std::ostream &os,Rncp const &);

class Cncp : public Header
{
public:
	Cncp(){m_newHop=0;m_cncpId=0;}
	Cncp(Ipv4Address newCid,Ipv4Address preCid,uint8_t newHop,uint32_t cncpId){
		m_newCid=newCid;
		m_preCid=preCid;
		m_newHop=newHop;
		m_cncpId=cncpId;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetNewCid(Ipv4Address newCid){m_newCid=newCid;}
	Ipv4Address GetNewCid(){return m_newCid;}

	void SetPreCid(Ipv4Address preCid){m_preCid=preCid;}
	Ipv4Address GetPreCid(){return m_preCid;}

	void SetNewHop(uint8_t newHop){m_newHop=newHop;}
	uint8_t GetNewHop(){return m_newHop;}

	void SetCncpId(uint32_t cncpId){m_cncpId=cncpId;}
	uint32_t GetCncpId(){return m_cncpId;}

	bool operator==(Cncp const &o) const;

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_newCid;
	Ipv4Address m_preCid;
	uint8_t m_newHop;
	uint32_t m_cncpId;
};
std::ostream &operator << (std::ostream &os,Cncp const &);



class Hello : public Header
{
public:

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void Print(std::ostream &os) const;

	void SetId (Ipv4Address id) { m_id = id; }
	Ipv4Address GetId () const { return m_id; }

	void SetBroadcastId(uint32_t id){m_broadcast_id=id;}
	uint32_t GetBroadcastId(){return m_broadcast_id;}

	bool operator==(Hello const &o) const;

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_id;
	uint32_t m_broadcast_id;
};
std::ostream &operator << (std::ostream &os,Hello const &);


class Ack : public Header
{
public:

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(Ack const &o) const;

	void SetId (Ipv4Address id) { m_id = id; }
	Ipv4Address GetId () const { return m_id; }

	void SetSeqNo(uint32_t seq_no){m_seq_no=seq_no;}
	uint32_t GetSeqNo(){return m_seq_no;}

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_id;
	uint32_t m_seq_no;
};
std::ostream &operator << (std::ostream &os,Ack const &);


class Data : public Header
{
public:
	Data(){m_seqNo=0;m_flag=HOGEHOGE;m_hop=0;m_dataSize=0;}
	Data(Ipv4Address source,Ipv4Address dst,Ipv4Address optionalSource,Ipv4Address optionalDst,uint32_t seqNo,uint8_t flag,uint8_t hop,uint64_t dataSize){
		m_source=source;
		m_dst=dst;
		m_optionalSource=optionalSource;
		m_optionalDst=optionalDst;
		m_seqNo=seqNo;
		m_flag=flag;
		m_hop=hop;
		m_dataSize=dataSize;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	uint64_t GetSerializedSize(bool flag) const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(Data const &o) const;

	void SetSource(Ipv4Address source){m_source=source;}
	Ipv4Address GetSource(){return m_source;}

	void SetDst(Ipv4Address dst){m_dst=dst;}
	Ipv4Address GetDst(){return m_dst;}

	void SetOptionalSource(Ipv4Address optionalSource){m_optionalSource=optionalSource;}
	Ipv4Address GetOptionalSource(){return m_optionalSource;}

	void SetOptionalDst(Ipv4Address optionalDst){m_optionalDst=optionalDst;}
	Ipv4Address GetOptionalDst(){return m_optionalDst;}

	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}

	void SetFlag(uint8_t flag){m_flag=flag;}
	uint8_t GetFlag(){return m_flag;}

	void SetHop(uint8_t hop){m_hop=hop;}
	uint8_t GetHop(){return m_hop;}

//	DataKey GetKey(){return DataKey(m_source,m_seqNo);}
	DataKey GetKey(){return DataKey(m_dst,m_seqNo);}

	void SetDataSize(uint64_t size){m_dataSize=size;}
	uint64_t GetDataSize(){return m_dataSize;}

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_source;
	Ipv4Address m_dst;
	Ipv4Address m_optionalSource;
	Ipv4Address m_optionalDst;
	uint32_t m_seqNo;
	uint8_t m_flag;
	uint8_t m_hop;

	uint64_t m_dataSize;

};
std::ostream &operator << (std::ostream &os,Data const &);

class RouteRequest : public Header
{
public:
	RouteRequest(){m_ttl=0;}
	RouteRequest(Ipv4Address source,Ipv4Address dst,uint32_t seqNo,uint8_t ttl,uint32_t rreqId){
		m_source=source;
		m_dst=dst;
		m_seqNo=seqNo;
		m_ttl=ttl;
		m_rreqId=rreqId;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(RouteRequest const &o) const;

	void SetSource(Ipv4Address source){m_source=source;}
	Ipv4Address GetSource(){return m_source;}

	void SetDst(Ipv4Address dst){m_dst=dst;}
	Ipv4Address GetDst(){return m_dst;}

	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}

	void SetTtl(uint8_t ttl){m_ttl=ttl;}
	uint8_t GetTtl(){return m_ttl;}

	void SetRreqId(uint32_t rreqId){m_rreqId=rreqId;}
	uint32_t GetRreqId(){return m_rreqId;}

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_source;
	Ipv4Address m_dst;
	uint32_t m_seqNo;
	uint8_t m_ttl;

	uint32_t m_rreqId;
};
std::ostream &operator << (std::ostream &os,RouteRequest const &);


class RouteReply : public Header
{
public:
	RouteReply(){}
	RouteReply(Ipv4Address source,Ipv4Address dst,uint32_t seqNo){
		m_source=source;
		m_dst=dst;
		m_seqNo=seqNo;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(RouteReply const &o) const;

	void SetSource(Ipv4Address source){m_source=source;}
	Ipv4Address GetSource(){return m_source;}

	void SetDst(Ipv4Address dst){m_dst=dst;}
	Ipv4Address GetDst(){return m_dst;}

	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_source;
	Ipv4Address m_dst;
	uint32_t m_seqNo;
};
std::ostream &operator << (std::ostream &os,RouteReply const &);


class DataAck : public Header
{
public:
	DataAck(){}
	DataAck(Ipv4Address source,Ipv4Address dst,uint32_t seqNo){
		m_source=source;
		m_dst=dst;
		m_seqNo=seqNo;
	}

	static TypeId GetTypeId();
	TypeId GetInstanceTypeId() const;

	void SetSerializedSize(uint32_t size) const {m_serialSize=size;};
	uint32_t GetSerializedSize() const;

	void Serialize(Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	void PrintList() const;
	void Print (std::ostream &os) const;
	bool operator==(DataAck const &o) const;

	void SetSource(Ipv4Address source){m_source=source;}
	Ipv4Address GetSource(){return m_source;}

	void SetDst(Ipv4Address dst){m_dst=dst;}
	Ipv4Address GetDst(){return m_dst;}

	void SetSeqNo(uint32_t seqNo){m_seqNo=seqNo;}
	uint32_t GetSeqNo(){return m_seqNo;}

private:
	mutable uint32_t m_serialSize;

	Ipv4Address m_source;
	Ipv4Address m_dst;
	uint32_t m_seqNo;
};
std::ostream &operator << (std::ostream &os,DataAck const &);

}
}



#endif /* SIMPLE_PACKET_H_ */
