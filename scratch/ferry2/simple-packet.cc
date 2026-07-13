/*
 * simple-packet.cc
 *
 *  Created on: 2016/03/03
 *      Author: terami
 */

#include "simple-packet.h"

//#include "simple-protocol.h"
#include "DTN-protocol.h"
//#include "DTN-helper.h"
#include "simple-rtable.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

#include <set>
#include <string>
#include <iostream>
#include <cstring>


namespace ns3 {
namespace simple {

NS_OBJECT_ENSURE_REGISTERED(TypeHeader);

TypeHeader::TypeHeader(MessageType t):
		m_type(t),
		m_valid(true)
{}

TypeId TypeHeader::GetTypeId() //パケットヘッダのタイプIDを取得する
{
	static TypeId tid=TypeId("ns3::simple::TypeHeader")
						  .SetParent<Header>()
						  .AddConstructor<TypeHeader>();
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
	Buffer::Iterator i = start;
	uint8_t type = i.ReadU8();
	//std::cout << type << std::endl;
	m_valid = true;
	switch(type){
	case TYPE_HELLO:
		m_type=(MessageType) type;
		break;
	case TYPE_USERDATA:
		m_type=(MessageType) type;
		break;
	case TYPE_META_DATA:
		m_type=(MessageType) type;
		break;
	case TYPE_SHELTERDATA:
		m_type=(MessageType) type;
		break;
    case TYPE_REQUEST:
        m_type=(MessageType) type;
        break;
	default:
		m_valid=false;
		break;
	}
	uint32_t dist = i.GetDistanceFrom(start);
	NS_ASSERT (dist == GetSerializedSize());
	return dist;
}

void TypeHeader::Print(std::ostream &os) const
{
	//...
}

bool TypeHeader::operator== (TypeHeader const &o) const
{
	return (m_type==o.m_type && m_valid==o.m_valid);
}

std::ostream &operator << (std::ostream &os,TypeHeader const &h)
{
	h.Print(os);
	return os;
}



NS_OBJECT_ENSURE_REGISTERED(ShelterInfo);

TypeId ShelterInfo::GetTypeId ()
{
	static TypeId tid=TypeId("ns3::simple::ShelterInfo")
						.SetParent<Header>()
						.AddConstructor<ShelterInfo>()
						;
	return tid;
}

TypeId ShelterInfo::GetInstanceTypeId () const
{
	return GetTypeId();
}

uint32_t ShelterInfo::GetSerializedSize () const
{
	uint32_t size = 4+4+4;
    return size;
	//return 4+4*3+4+4+4+4+4+4+4+4*MyBuilding::GetExitNodes().size()+4*m_blocked_set.size();
}

uint64_t ShelterInfo::GetSerializedSize (bool flag) const
{
	uint32_t size = 4+4+4;
    return size;
	//return 4+4*3+4+4+4+4+4+4+4+4*MyBuilding::GetExitNodes().size()+4*m_blocked_set.size();
}

void ShelterInfo::Serialize (Buffer::Iterator i) const
{

	// i.WriteU32(m_broadcast_id);
	i.WriteU32(m_shelterID);
	i.WriteU32(m_exit_num);
	i.WriteU32(m_time);
	
}

uint32_t ShelterInfo::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	// m_broadcast_id=i.ReadU32();
	m_shelterID=i.ReadU32();
	m_exit_num=i.ReadU32();
	m_time = i.ReadU32();
	
	uint32_t dist = i.GetDistanceFrom(start);
	// std::cout<< dist <<"!!!!!!!!!!!" <<std::endl;
	NS_ASSERT(dist == GetSerializedSize());
	return dist;
}

void ShelterInfo::Print (std::ostream &os) const
{
	// ..
}

void ShelterInfo::PrintList () const
{

}


std::ostream &operator << (std::ostream &os,ShelterInfo const &h)
{
	h.Print(os);
	return os;
}

bool ShelterInfo::operator== (ShelterInfo const &o) const
{
	return (m_shelterID==o.m_shelterID && m_exit_num==o.m_exit_num && m_time==o.m_time);
}

NS_OBJECT_ENSURE_REGISTERED(SummaryVector);

TypeId
SummaryVector::GetTypeId()
{
        static TypeId tid = TypeId("ns3::simple::SummaryVector")
                                    .SetParent<Header>()
                                    .AddConstructor<SummaryVector>();
        return tid;
}

TypeId
SummaryVector::GetInstanceTypeId() const
{
        return GetTypeId();
}

uint32_t
SummaryVector::GetSerializedSize() const
{
        uint32_t exit_count = 0;
        for (auto &p : m_exitSummary) {
                if (!p.first.empty()) exit_count++;
        }

        uint32_t block_count = 0;
        for (auto &p : m_blockSummary) {
                if (!p.first.empty()) block_count++;
        }

        uint32_t size = 0;
        size += 4; // m_broadcast_id
        size += 4; // m_userId
        size += 4; // m_dst
        size += 4 + (4 + 4) * exit_count;   // exitSummary: count + id + time
        size += 4 + (4 + 4) * block_count;  // blockSummary: count + id + time(float bits)

        return size;
}

void
SummaryVector::Serialize(Buffer::Iterator i) const
{
        i.WriteU32(m_broadcast_id);
        i.WriteU32(m_userId);
        WriteTo(i, m_dst);

        uint32_t exit_count = 0;
        for (auto &p : m_exitSummary) {
                if (!p.first.empty()) exit_count++;
        }
        i.WriteU32(exit_count);
        for (auto &p : m_exitSummary) {
                if (p.first.empty()) continue;
                i.WriteU32(JSON_Data::NodeIDstoi(p.first));
                i.WriteU32(p.second);
        }

        uint32_t block_count = 0;
        for (auto &p : m_blockSummary) {
                if (!p.first.empty()) block_count++;
        }
        i.WriteU32(block_count);
        for (auto &p : m_blockSummary) {
                if (p.first.empty()) continue;
                i.WriteU32(JSON_Data::NodeIDstoi(p.first));

                uint32_t tmp;
                static_assert(sizeof(tmp) == sizeof(float), "sizes must match");
                memcpy(&tmp, &p.second, sizeof(float));
                i.WriteU32(tmp);
        }
}

uint32_t
SummaryVector::Deserialize(Buffer::Iterator start)
{
        Buffer::Iterator i = start;

        m_broadcast_id = i.ReadU32();
        m_userId = i.ReadU32();
        ReadFrom(i, m_dst);

        uint32_t exit_count = i.ReadU32();
        m_exitSummary.clear();
        for (uint32_t k = 0; k < exit_count; ++k) {
                uint32_t id = i.ReadU32();
                uint32_t time = i.ReadU32();
                m_exitSummary.insert(
                        std::make_pair("node/" + std::to_string(id), time)
                );
        }

        uint32_t block_count = i.ReadU32();
        m_blockSummary.clear();
        for (uint32_t k = 0; k < block_count; ++k) {
                uint32_t id = i.ReadU32();
                uint32_t tmp = i.ReadU32();

                float time;
                memcpy(&time, &tmp, sizeof(float));

                m_blockSummary.insert(
                        std::make_pair("node/" + std::to_string(id), time)
                );
        }

        uint32_t dist = i.GetDistanceFrom(start);
        NS_ASSERT(dist == GetSerializedSize());
        return dist;
}

void
SummaryVector::Print(std::ostream &os) const
{
        os << "SummaryVector";
}

std::ostream &
operator << (std::ostream &os, SummaryVector const &h)
{
        h.Print(os);
        return os;
}


NS_OBJECT_ENSURE_REGISTERED(RequestData);

TypeId
RequestData::GetTypeId()
{
    static TypeId tid = TypeId("ns3::simple::RequestData")
                            .SetParent<Header>()
                            .AddConstructor<RequestData>();
    return tid;
}

TypeId
RequestData::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
RequestData::GetSerializedSize() const
{
    return 4 + 4 + 4 * m_exitIds.size()
             + 4 + 4 * m_blockIds.size();
}

void
RequestData::Serialize(Buffer::Iterator i) const
{
    i.WriteU32(m_requesterId);

    i.WriteU32(m_exitIds.size());
    for (auto id : m_exitIds) {
        i.WriteU32(JSON_Data::NodeIDstoi(id));
    }

    i.WriteU32(m_blockIds.size());
    for (auto id : m_blockIds) {
        i.WriteU32(JSON_Data::NodeIDstoi(id));
    }
}

uint32_t
RequestData::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;

    m_requesterId = i.ReadU32();

    uint32_t exitCount = i.ReadU32();
    m_exitIds.clear();
    for (uint32_t k = 0; k < exitCount; k++) {
        uint32_t id = i.ReadU32();
        m_exitIds.push_back("node/" + std::to_string(id));
    }

    uint32_t blockCount = i.ReadU32();
    m_blockIds.clear();
    for (uint32_t k = 0; k < blockCount; k++) {
        uint32_t id = i.ReadU32();
        m_blockIds.push_back("node/" + std::to_string(id));
    }

    uint32_t dist = i.GetDistanceFrom(start);
    NS_ASSERT(dist == GetSerializedSize());
    return dist;
}

void
RequestData::Print(std::ostream &os) const
{
    os << "RequestData";
}

std::ostream &
operator << (std::ostream &os, RequestData const &h)
{
    h.Print(os);
    return os;
}


NS_OBJECT_ENSURE_REGISTERED(UserData);
TypeId UserData::GetTypeId ()
{
	static TypeId tid = TypeId("ns3::simple::UserData").SetParent<Header>().AddConstructor<UserData>();
	return tid;
}

TypeId UserData::GetInstanceTypeId () const
{
	return GetTypeId();
}

uint32_t UserData::GetSerializedSize () const
{
    // eReiÅÀÛÉ«o·vfðvZ·é
    uint32_t blocked_count = 0;
    for (auto &s : m_blocked_set) if(!s.empty()) blocked_count++;

    uint32_t fullexit_count = 0;
    for (auto &s : m_fullexit_set) if(!s.empty()) fullexit_count++;

    uint32_t aoi_exit_count = m_aoi_exitinfo.size(); // ·×Ä­Oñ
    uint32_t aoi_block_count = 0;
    for (auto &p : m_aoi_blockinfo) if(!p.first.empty()) aoi_block_count++;

    // my_patrol: always write two u32 (ereaid, time). If empty, ereaid becomes 0.
    // other_UAV: count of entries
    uint32_t other_uav_count = 0;
    for (auto &p : other_UAV_patrol_erea_and_id) if(!p.first.empty()) other_uav_count++;

	uint32_t patrol_ereaid_and_time_count = 0;
	for (auto &p : patrol_ereaid_and_time) if(!p.first.empty()) patrol_ereaid_and_time_count++;


    // \¬ioCgj:
    // m_broadcast_id (4) + m_userId (4) + dst (WriteTo/ReadFromÌTCYÍùmÆ¼è) +
    // blocked: 4(size) + 4 * blocked_count
    // fullexit: 4(size) + 4 * fullexit_count
    // aoi_exitinfo: 4(size) + (4 + 4 + 4) * aoi_exit_count
    // aoi_blockinfo: 4(size) + (4 + 4) * aoi_block_count
    // my_patrol: 4 + 4
    // other_uav: 4(size) + (4 + 4) * other_uav_count
    uint32_t size = 0;
    size += 4; // m_broadcast_id
    size += 4; // m_userId
    size += 4; // (WriteTo/ReadFrom ÅÅè·Æ¼èÅ«éªª éêÍ±±ð²®µÄ­¾³¢)
              // Ó: WriteTo/ReadFrom(Ipv4Address) ÌoCgªª©ÁÄ¢êÎ»êðg¤
    // blocked
    size += 4 + 4 * blocked_count;
    // fullexit
    size += 4 + 4 * fullexit_count;
    // aoi_exitinfo
    size += 4 + (4 + 4 + 4) * aoi_exit_count;
    // aoi_blockinfo
    size += 4 + (4 + 4) * aoi_block_count;
    // my_patrol
    size += 4 + 4;
    // other_uav
    size += 4 + (4 + 4) * other_uav_count;

	size += 4 + (4 + 4) * patrol_ereaid_and_time_count;

    return size;
}


uint64_t UserData::GetSerializedSize (bool flag) const
{
	uint32_t size = 4+4+4+4+(4*m_blocked_set.size())+4+(4*m_fullexit_set.size())+4+(12*m_aoi_exitinfo.size())+4+(8*m_aoi_blockinfo.size())+8+(8*other_UAV_patrol_erea_and_id.size());
    return size;

}

void UserData::Serialize (Buffer::Iterator i) const
{
    i.WriteU32(m_broadcast_id);
    i.WriteU32(m_userId);
    WriteTo(i, m_dst);

    // blocked_set: count only non-empty, then write each id
    uint32_t blocked_count = 0;
    for (auto &s : m_blocked_set) if (!s.empty()) blocked_count++;
    i.WriteU32(blocked_count);
    for (auto &s : m_blocked_set) {
        if (s.empty()) continue;
        i.WriteU32(JSON_Data::NodeIDstoi(s));
    }

    // fullexit_set
    uint32_t fullexit_count = 0;
    for (auto &s : m_fullexit_set) if (!s.empty()) fullexit_count++;
    i.WriteU32(fullexit_count);
    for (auto &s : m_fullexit_set) {
        if (s.empty()) continue;
        i.WriteU32(JSON_Data::NodeIDstoi(s));
    }

    // aoi_exitinfo
    uint32_t aoi_exit_count = m_aoi_exitinfo.size();
    i.WriteU32(aoi_exit_count);
    for (auto &p : m_aoi_exitinfo) {
        // p.first = node id string
        i.WriteU32(JSON_Data::NodeIDstoi(p.first));
        i.WriteU32(p.second.first);
        i.WriteU32(p.second.second);
    }

    // aoi_blockinfo (float -> uint32_t rbgRs[)
    uint32_t aoi_block_count = 0;
    for (auto &p : m_aoi_blockinfo) if (!p.first.empty()) aoi_block_count++;
    i.WriteU32(aoi_block_count);
    for (auto &p : m_aoi_blockinfo) {
        if (p.first.empty()) continue;
        i.WriteU32(JSON_Data::NodeIDstoi(p.first));
        uint32_t tmp;
        static_assert(sizeof(tmp) == sizeof(float), "sizes must match");
        memcpy(&tmp, &p.second, sizeof(float));
        i.WriteU32(tmp);
    }

    // my_patrol: write node id (or 0 if empty) and time
    uint32_t patrolEreaId = 0;
    if (!my_patrol_erea_and_start_time.first.empty()) {
        patrolEreaId = JSON_Data::NodeIDstoi(my_patrol_erea_and_start_time.first);
    }
    i.WriteU32(patrolEreaId);
    i.WriteU32(my_patrol_erea_and_start_time.second);

    // other_UAV_patrol_erea_and_id
    uint32_t other_count = 0;
    for (auto &p : other_UAV_patrol_erea_and_id) if (!p.first.empty()) other_count++;
    i.WriteU32(other_count);
    for (auto &p : other_UAV_patrol_erea_and_id) {
        if (p.first.empty()) continue;
        i.WriteU32(JSON_Data::NodeIDstoi(p.first));
        i.WriteU32(p.second);
    }

	uint32_t ereaid_count = 0;
    for (auto &p : patrol_ereaid_and_time) if (!p.first.empty()) ereaid_count++;
    i.WriteU32(ereaid_count);
    for (auto &p : patrol_ereaid_and_time) {
        if (p.first.empty()) continue;
        i.WriteU32(JSON_Data::NodeIDstoi(p.first));
        i.WriteU32(p.second);
    }
}

uint32_t UserData::Deserialize (Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    m_broadcast_id = i.ReadU32();
    m_userId = i.ReadU32();
    ReadFrom(i, m_dst);

    // blocked_set
    uint32_t blocked_count = i.ReadU32();
    m_blocked_set.clear();
    for (uint32_t k = 0; k < blocked_count; ++k) {
        uint32_t id = i.ReadU32();
        m_blocked_set.insert("node/" + std::to_string(id));
    }

    // fullexit_set
    uint32_t fullexit_count = i.ReadU32();
    m_fullexit_set.clear();
    for (uint32_t k = 0; k < fullexit_count; ++k) {
        uint32_t id = i.ReadU32();
        m_fullexit_set.insert("node/" + std::to_string(id));
    }

    // aoi_exitinfo
    uint32_t aoi_exit_count = i.ReadU32();
    m_aoi_exitinfo.clear();
    for (uint32_t k = 0; k < aoi_exit_count; ++k) {
        uint32_t id = i.ReadU32();
        uint32_t can_exit_num = i.ReadU32();
        uint32_t time = i.ReadU32();
        m_aoi_exitinfo.insert(std::make_pair("node/" + std::to_string(id), std::make_pair(can_exit_num, time)));
    }

    // aoi_blockinfo
    uint32_t aoi_block_count = i.ReadU32();
    m_aoi_blockinfo.clear();
    for (uint32_t k = 0; k < aoi_block_count; ++k) {
        uint32_t id = i.ReadU32();
        uint32_t tmp = i.ReadU32();
        float f;
        memcpy(&f, &tmp, sizeof(float));
        m_aoi_blockinfo.insert(std::make_pair("node/" + std::to_string(id), f));
    }

    // my_patrol
    uint32_t ereaid = i.ReadU32();
    uint32_t starttime = i.ReadU32();
    if (ereaid == 0) {
        my_patrol_erea_and_start_time = std::make_pair(String(""), (uint32_t)0);
    } else {
        my_patrol_erea_and_start_time = std::make_pair(String("node/" + std::to_string(ereaid)), starttime);
    }

    // other_UAV_patrol_erea_and_id  © ±±ð³µ­ other_UAV Éüêé
    uint32_t other_count = i.ReadU32();
    other_UAV_patrol_erea_and_id.clear();
    for (uint32_t k = 0; k < other_count; ++k) {
        uint32_t id = i.ReadU32();
        uint32_t uavid = i.ReadU32();
        other_UAV_patrol_erea_and_id.insert(std::make_pair(String("node/" + std::to_string(id)), uavid));
    }

	uint32_t ereaid_count = i.ReadU32();
    patrol_ereaid_and_time.clear();
    for (uint32_t k = 0; k < ereaid_count; ++k) {
        uint32_t id = i.ReadU32();
        uint32_t time = i.ReadU32();
        patrol_ereaid_and_time.insert(std::make_pair(String("node/" + std::to_string(id)), time));
    }

    uint32_t dist = i.GetDistanceFrom(start);
    uint32_t expectedSize = GetSerializedSize();
    NS_ASSERT(dist == expectedSize);
    return dist;
}


void UserData::Print (std::ostream &os) const
{
	// ..
}

void UserData::PrintList () const
{

}



std::ostream &operator << (std::ostream &os,UserData const &h)
{
	h.Print(os);
	return os;
}

bool UserData::operator== (UserData const &o) const
{
	return (m_userId==o.m_userId && m_dst==o.m_dst && m_blocked_set == o.m_blocked_set && m_fullexit_set == o.m_fullexit_set && m_aoi_exitinfo == o.m_aoi_exitinfo && m_aoi_blockinfo == o.m_aoi_blockinfo && m_broadcast_id == o.m_broadcast_id && my_patrol_erea_and_start_time == o.my_patrol_erea_and_start_time && other_UAV_patrol_erea_and_id == o.other_UAV_patrol_erea_and_id && patrol_ereaid_and_time == o.patrol_ereaid_and_time);
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
	return 4+4+4;
}

//パケットを送出する際に使用されるカプセル化関数
void Hello::Serialize(Buffer::Iterator i) const
{
	WriteTo (i, m_id);
	i.WriteU32(m_broadcast_id);
	i.WriteU32(m_userID);
}

//パケットを受信する際に使用されるデカプセル化関数
uint32_t Hello::Deserialize(Buffer::Iterator start)
{
	Buffer::Iterator i=start;
	ReadFrom (i, m_id);
	m_broadcast_id=i.ReadU32();
	m_userID = i.ReadU32();

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
	return (m_id==o.m_id && m_broadcast_id==o.m_broadcast_id && m_userID==o.m_userID);
}


TypeId Hop::GetTypeId()
{
    static TypeId tid = TypeId("ns3::simple::Hop")
                            .SetParent<Header>()
                            .AddConstructor<Hop>();
    return tid;
}

TypeId Hop::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t Hop::GetSerializedSize() const
{
    return 4 + (16 * m_hop.size());
}

void Hop::Serialize(Buffer::Iterator i) const
{
    uint32_t size = m_hop.size();
    i.WriteU32(size);
    // std::cout << "[Serialize] Size: " << size << std::endl;

    for (auto hop : m_hop) {
        i.WriteU32(hop.first);
		if(hop.second.first.first == ""){
			std::cout<<"中身から！！"<<std::endl;
		}
		i.WriteU32(JSON_Data::NodeIDstoi(hop.second.first.first));
		i.WriteU32(hop.second.first.second);
		i.WriteU32(hop.second.second); //情報生成時間（AoI）
        // std::cout << "[Serialize] Hop: " << hop << std::endl;
    }
}


uint32_t Hop::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    uint32_t size = i.ReadU32();
    // std::cout << "[Deserialize] Size: " << size << std::endl;
	// if(size > 0){
		// std::cout<<"[Deserialize] Hop: -----------------------------"<<std::endl;
	for (uint32_t j = 0; j < size; ++j) {
		uint32_t userid = i.ReadU32();
		uint32_t nodeid = i.ReadU32();
		uint32_t required_time = i.ReadU32();
		uint32_t generation_time = i.ReadU32();
		std::string data;
		data = "node/" + std::to_string(nodeid);
		m_hop.insert(std::make_pair(userid, std::make_pair(std::make_pair(data, required_time),generation_time)));
		// std::cout << ad << std::endl;
	}
		// std::cout<<"------------------------------------------------"<<std::endl;
	// }
    uint32_t dist = i.GetDistanceFrom(start);
    // std::cout << "[Deserialize] Distance: " << dist << std::endl;
    NS_ASSERT(dist == GetSerializedSize());
    return dist;
}


void Hop::Print(std::ostream &os) const
{

}

std::ostream &operator<<(std::ostream &os, Hop const &h)
{
    h.Print(os);
    return os;
}

bool Hop::operator==(Hop const &o) const
{
    return (m_hop == o.m_hop);
}


}}


