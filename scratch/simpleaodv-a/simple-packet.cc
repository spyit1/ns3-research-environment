/*
 * simple-packet.cc
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#include "simple-protocol.h"
#include "simple-packet.h"
#include "simple-rtable.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3
{
namespace simple
{
NS_OBJECT_ENSURE_REGISTERED(TypeHeader);

TypeHeader::TypeHeader(MessageType t) :
					  m_type(t),m_valid(true)
{
}

TypeId TypeHeader::GetTypeId() //パケットヘッダのタイプIDを取得する
{
	static TypeId tid=TypeId("ns3::simple::TypeHeader")
						  .SetParent<Header>()
						  .AddConstructor<TypeHeader>()
						  ;
	return tid;
}

TypeId TypeHeader::GetInstanceTypeId() const //タイプIDのインスタンスを取得する
{
	return GetTypeId();
}

uint32_t TypeHeader::GetSerializedSize() const //パケットタイプヘッダのカプセル化サイズを取得する
{
	return 1;
}

void TypeHeader::Serialize(Buffer::Iterator i) const //パケッタイプヘッダのカプセル化処理
{
	i.WriteU8((uint8_t) m_type);
}

uint32_t TypeHeader::Deserialize(Buffer::Iterator start) //パケットタイプヘッダのデカプセル化処理
{
	Buffer::Iterator i=start;
	uint8_t type=i.ReadU8();
	m_valid=true;
	switch(type){
	case TYPE_HELLO:
	case TYPE_ACK:
	case TYPE_DATA:
	case TYPE_ROUTEREQUEST:
	case TYPE_ROUTEREPLY:
	case TYPE_DATAACK:
		m_type=(MessageType) type;
		break;
	default:
		m_valid=false;
	}
	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT (dist==GetSerializedSize());
	return dist;
}

void TypeHeader::Print(std::ostream &os) const
{
	//...
}

bool TypeHeader::operator==(TypeHeader const &o) const
		{
	return (m_type==o.m_type && m_valid==o.m_valid);
		}

std::ostream &operator << (std::ostream &os,TypeHeader const &h)
{
	h.Print(os);
	return os;
}

//Mepパケット処理部

NS_OBJECT_ENSURE_REGISTERED(Mep);

TypeId Mep::GetTypeId()
{
	static TypeId tid=TypeId("ns3::cluster::Mep").SetParent<Header>().AddConstructor<Mep>();
	return tid;
}

TypeId Mep::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Mep::GetSerializedSize() const
{
	return 4+1+1+1+4+4;
}

//パケットを送出する際に使用されるカプセル化関数
void Mep::Serialize(Buffer::Iterator i) const
{
	WriteTo (i, m_cid);
	i.WriteU8 (m_hop);
	i.WriteU8(m_size);
	i.WriteU8(m_ttl);
	i.WriteU32(m_cellularId);
	i.WriteU32(m_mepId);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Mep::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom (i, m_cid);
	m_hop=i.ReadU8 ();
	m_size=i.ReadU8();
	m_ttl=i.ReadU8();
	m_cellularId=i.ReadU32();
	m_mepId=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Mep::Print(std::ostream &os) const
{
	//...
}

std::ostream &operator << (std::ostream &os,Mep const &h)
{
	h.Print(os);
	return os;
}

bool Mep::operator==(Mep const &o) const{
	return (m_hop==o.m_hop && m_cid==o.m_cid && m_size==o.m_size && m_ttl==o.m_ttl && m_cellularId==o.m_cellularId && m_mepId==o.m_mepId);
}

TypeId Mqp::GetTypeId()
{
	static TypeId tid=TypeId("ns3::cluster::Mqp").SetParent<Header>().AddConstructor<Mqp>();
	return tid;
}

TypeId Mqp::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Mqp::GetSerializedSize() const
{
	return 4+4+1;
}

//パケットを送出する際に使用されるカプセル化関数
void Mqp::Serialize(Buffer::Iterator i) const
{
	WriteTo (i,m_source);
	WriteTo(i,m_dst);
	i.WriteU8(m_size);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Mqp::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom (i,m_source);
	ReadFrom(i,m_dst);
	m_size=i.ReadU8();
	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Mqp::Print(std::ostream &os) const
{
	//...
}

std::ostream &operator << (std::ostream &os,Mqp const &h)
{
	h.Print(os);
	return os;
}

bool Mqp::operator==(Mqp const &o) const{
	return (m_source==o.m_source && m_dst==o.m_dst && m_size==o.m_size);
}

TypeId Mqap::GetTypeId()
{
	static TypeId tid=TypeId("ns3::cluster::Mqap").SetParent<Header>().AddConstructor<Mqap>();
	return tid;
}

TypeId Mqap::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Mqap::GetSerializedSize() const
{
	return 4+4+1+1+4;
}

//パケットを送出する際に使用されるカプセル化関数
void Mqap::Serialize(Buffer::Iterator i) const
{
	WriteTo (i,m_newCid);
	WriteTo(i,m_preCid);
	i.WriteU8(m_newHop);
	i.WriteU8(m_size);
	i.WriteU32(m_mqapId);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Mqap::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom (i,m_newCid);
	ReadFrom(i,m_preCid);
	m_newHop=i.ReadU8();
	m_size=i.ReadU8();
	m_mqapId=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Mqap::Print(std::ostream &os) const
{
	//...
}

std::ostream &operator << (std::ostream &os,Mqap const &h)
{
	h.Print(os);
	return os;
}

bool Mqap::operator==(Mqap const &o) const{
	return (m_newCid==o.m_newCid && m_preCid==o.m_preCid && m_newHop==o.m_newHop && m_size==o.m_size && m_mqapId==o.m_mqapId);
}


TypeId Rncp::GetTypeId()
{
	static TypeId tid=TypeId("ns3::cluster::Rncp").SetParent<Header>().AddConstructor<Rncp>();
	return tid;
}

TypeId Rncp::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Rncp::GetSerializedSize() const
{
	return 4+4;
}

//パケットを送出する際に使用されるカプセル化関数
void Rncp::Serialize(Buffer::Iterator i) const
{
	WriteTo(i,m_currentCid);
	WriteTo(i,m_candidateCid);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Rncp::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom(i,m_currentCid);
	ReadFrom (i,m_candidateCid);

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Rncp::Print(std::ostream &os) const
{
	//...
}

std::ostream &operator << (std::ostream &os,Rncp const &h)
{
	h.Print(os);
	return os;
}

bool Rncp::operator==(Rncp const &o) const{
	return (m_currentCid==o.m_currentCid && m_candidateCid==o.m_candidateCid);
}

TypeId Cncp::GetTypeId()
{
	static TypeId tid=TypeId("ns3::cluster::Cncp").SetParent<Header>().AddConstructor<Rncp>();
	return tid;
}

TypeId Cncp::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Cncp::GetSerializedSize() const
{
	return 4+4+1+4;
}

//パケットを送出する際に使用されるカプセル化関数
void Cncp::Serialize(Buffer::Iterator i) const
{
	WriteTo (i,m_newCid);
	WriteTo(i,m_preCid);
	i.WriteU8(m_newHop);
	i.WriteU32(m_cncpId);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Cncp::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom (i,m_newCid);
	ReadFrom(i,m_preCid);
	m_newHop=i.ReadU8();
	m_cncpId=i.ReadU32();
	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Cncp::Print(std::ostream &os) const
{
	//...
}

std::ostream &operator << (std::ostream &os,Cncp const &h)
{
	h.Print(os);
	return os;
}

bool Cncp::operator==(Cncp const &o) const{
	return (m_newCid==o.m_newCid && m_preCid==o.m_preCid && m_newHop==o.m_newHop && m_cncpId==o.m_cncpId);
}


TypeId Hello::GetTypeId()
{
	static TypeId tid=TypeId("ns3::simple::Hello")
						  .SetParent<Header>()
						  .AddConstructor<Hello>()
						  ;
	return tid;
}

TypeId Hello::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Hello::GetSerializedSize() const
{
	return 4+4;
}

//パケットを送出する際に使用されるカプセル化関数
void Hello::Serialize(Buffer::Iterator i) const
{
	WriteTo (i, m_id);
	i.WriteU32(m_broadcast_id);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Hello::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom (i, m_id);
	m_broadcast_id=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Hello::Print(std::ostream &os) const
{
	//...
}

std::ostream &operator << (std::ostream &os,Hello const &h)
{
	h.Print(os);
	return os;
}

bool Hello::operator==(Hello const &o) const
{
	return (m_id==o.m_id && m_broadcast_id==o.m_broadcast_id);
}


TypeId Ack::GetTypeId()
{
	static TypeId tid=TypeId("ns3::simple::Ack")
	  								  .SetParent<Header>()
	  								  .AddConstructor<Ack>()
	  								  ;
	return tid;
}

TypeId Ack::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Ack::GetSerializedSize() const
{
	return 4+4;
}

void Ack::Serialize(Buffer::Iterator i) const
{
	WriteTo(i,m_id);
	i.WriteU32(m_seq_no);
}

uint32_t Ack::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;

	ReadFrom(i,m_id);
	m_seq_no=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Ack::Print(std::ostream &os) const
{
	//...
}

void Ack::PrintList() const
{

}

std::ostream &operator << (std::ostream &os,Ack const &h)
{
	h.Print(os);
	return os;
}

bool Ack::operator==(Ack const &o) const{
	return (m_id==o.m_id && m_seq_no==o.m_seq_no);
}

TypeId Data::GetTypeId()
{
	static TypeId tid=TypeId("ns3::simple::Data").SetParent<Header>().AddConstructor<Data>();
	return tid;
}

TypeId Data::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t Data::GetSerializedSize() const
{
	return 4+4+4+4+4+1+1+8;
}

uint64_t Data::GetSerializedSize(bool flag) const
{
	return 4+4+4+4+4+1+1+8+m_dataSize;
}

void Data::Serialize(Buffer::Iterator i) const
{
	WriteTo(i,m_source);
	WriteTo(i,m_dst);
	WriteTo(i,m_optionalSource);
	WriteTo(i,m_optionalDst);
	i.WriteU32(m_seqNo);
	i.WriteU8(m_flag);
	i.WriteU8(m_hop);

	i.WriteU64(m_dataSize);
}

uint32_t Data::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;

	ReadFrom(i,m_source);
	ReadFrom(i,m_dst);
	ReadFrom(i,m_optionalSource);
	ReadFrom(i,m_optionalDst);
	m_seqNo=i.ReadU32();
	m_flag=i.ReadU8();
	m_hop=i.ReadU8();

	m_dataSize=i.ReadU64();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void Data::Print(std::ostream &os) const
{
	//...
}

void Data::PrintList() const
{

}

std::ostream &operator << (std::ostream &os,Data const &h)
{
	h.Print(os);
	return os;
}

bool Data::operator==(Data const &o) const{
	return (m_source==o.m_source && m_dst==o.m_dst && m_optionalSource==o.m_optionalSource && m_optionalDst==o.m_optionalDst &&
			m_seqNo==o.m_seqNo && m_flag==o.m_flag && m_hop==o.m_hop && m_dataSize==o.m_dataSize
	);
}

TypeId RouteRequest::GetTypeId()
{
	static TypeId tid=TypeId("ns3::simple::RouteRequest").SetParent<Header>().AddConstructor<RouteRequest>();
	return tid;
}

TypeId RouteRequest::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t RouteRequest::GetSerializedSize() const
{
	return 4+4+4+1+4;
}

void RouteRequest::Serialize(Buffer::Iterator i) const
{
	WriteTo(i,m_source);
	WriteTo(i,m_dst);
	i.WriteU32(m_seqNo);
	i.WriteU8(m_ttl);
	i.WriteU32(m_rreqId);
}

uint32_t RouteRequest::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom(i,m_source);
	ReadFrom(i,m_dst);
	m_seqNo=i.ReadU32();
	m_ttl=i.ReadU8();
	m_rreqId=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void RouteRequest::Print(std::ostream &os) const
{
	//...
}

void RouteRequest::PrintList() const
{

}

std::ostream &operator << (std::ostream &os,RouteRequest const &h)
{
	h.Print(os);
	return os;
}

bool RouteRequest::operator==(RouteRequest const &o) const{
	return (m_source==o.m_source && m_dst==o.m_dst && m_seqNo==o.m_seqNo && m_ttl==o.m_ttl && m_rreqId==o.m_rreqId);
}

TypeId RouteReply::GetTypeId()
{
	static TypeId tid=TypeId("ns3::simple::RouteReply").SetParent<Header>().AddConstructor<RouteReply>();
	return tid;
}

TypeId RouteReply::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t RouteReply::GetSerializedSize() const
{
	return 4+4+4;
}

void RouteReply::Serialize(Buffer::Iterator i) const
{
	WriteTo(i,m_source);
	WriteTo(i,m_dst);
	i.WriteU32(m_seqNo);
}

uint32_t RouteReply::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom(i,m_source);
	ReadFrom(i,m_dst);
	m_seqNo=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void RouteReply::Print(std::ostream &os) const
{
	//...
}

void RouteReply::PrintList() const
{

}

std::ostream &operator << (std::ostream &os,RouteReply const &h)
{
	h.Print(os);
	return os;
}

bool RouteReply::operator==(RouteReply const &o) const{
	return (m_source==o.m_source && m_dst==o.m_dst && m_seqNo==o.m_seqNo);
}


TypeId DataAck::GetTypeId()
{
	static TypeId tid=TypeId("ns3::simple::DataAck").SetParent<Header>().AddConstructor<DataAck>();
	return tid;
}

TypeId DataAck::GetInstanceTypeId() const
{
	return GetTypeId();
}

uint32_t DataAck::GetSerializedSize() const
{
	return 4+4+4;
}

void DataAck::Serialize(Buffer::Iterator i) const
{
	WriteTo(i,m_source);
	WriteTo(i,m_dst);
	i.WriteU32(m_seqNo);
}

uint32_t DataAck::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom(i,m_source);
	ReadFrom(i,m_dst);
	m_seqNo=i.ReadU32();

	uint32_t dist=i.GetDistanceFrom(start);
	NS_ASSERT(dist==GetSerializedSize());
	return dist;
}

void DataAck::Print(std::ostream &os) const
{
	//...
}

void DataAck::PrintList() const
{

}

std::ostream &operator << (std::ostream &os,DataAck const &h)
{
	h.Print(os);
	return os;
}

bool DataAck::operator==(DataAck const &o) const{
	return (m_source==o.m_source && m_dst==o.m_dst && m_seqNo==o.m_seqNo);
}

}
}


