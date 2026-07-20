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
	uint32_t size = 4 + 4 + 4 + 4*m_blocked_set.size() + 4 + 4*m_fullexit_set.size() + 4 + (3 * 4 * m_aoi_exitinfo.size()) + 4 + (4 * 4 * m_aoi_blockinfo.size());

    return size;
}

uint64_t UserData::GetSerializedSize (bool flag) const
{
	uint32_t size = 4 + 4 + 4 + 4*m_blocked_set.size() + 4 + 4*m_fullexit_set.size() + 4 + (3 * 4 * m_aoi_exitinfo.size()) + 4 + (4 * 4 * m_aoi_blockinfo.size());
    return size;

}

void UserData::Serialize (Buffer::Iterator i) const
{
	//std::cout<<&i<<std::endl;
	i.WriteU32(m_userId);
	WriteTo(i,m_dst);
	std::set<String> b_buf = m_blocked_set;
	uint32_t size = b_buf.size();
	// std::cout<<size<<std::endl;
	i.WriteU32(size);
	if(!b_buf.empty()){
		for (auto abc : b_buf) {
			//std::cout << abc <<", ";
			if(!abc.empty()){
				// uint32_t size = abc.size();
				// i.WriteU32(size);
				// std::cout<<&i<<std::endl;
				// i.Write(reinterpret_cast<const uint8_t*>(abc.c_str()), size);
				// std::cout<<&i<<std::endl;
				// std::cout<<JSON_Data::NodeIDstoi(abc)<<std::endl;
				i.WriteU32(JSON_Data::NodeIDstoi(abc)); //node/12345の形を12345のintに変換
			}
		}
		//std::cout<<std::endl;
	}

	uint32_t size2 = m_fullexit_set.size();
	i.WriteU32(size2); 

	for(auto x : m_fullexit_set){
		// std::cout<<JSON_Data::NodeIDstoi(x)<<std::endl;
		i.WriteU32(JSON_Data::NodeIDstoi(x));
	}

	uint32_t size3 = m_aoi_exitinfo.size();
	
	// std::cout<<m_userId<<std::endl;
	i.WriteU32(size3);

	// for(auto y : m_aoi_exitinfo){
	// 	std::cout<<y.first<<std::endl;
	// }

	for(auto y : m_aoi_exitinfo){
		// std::cout<<y.first<<std::endl;
		i.WriteU32(JSON_Data::NodeIDstoi(y.first)); //避難所ID
		i.WriteU32(y.second.first); //収容可能人数
		i.WriteU32(y.second.second); //情報の生成された時間
	}

	uint32_t size4 = m_aoi_blockinfo.size();
	i.WriteU32(size4);

	for(auto a : m_aoi_blockinfo){
		i.WriteU32(JSON_Data::NodeIDstoi(a.first)); //避難所ID
		// std::cout<<a.second<<std::endl;
		i.WriteU32(a.second);
	}
	// std::cout<<"シリアライズ終了"<<std::endl;
}

uint32_t UserData::Deserialize (Buffer::Iterator start)
{
	Buffer::Iterator i = start;
	//std::cout<<&i<<std::endl;
	m_userId = i.ReadU32();
	ReadFrom(i,m_dst);

	uint32_t size = i.ReadU32();

	while(size > 0){
		//std::cout<<"OK!!"<<std::endl;
		uint32_t ID;
		ID = i.ReadU32();
		// std::cout<<ID<<std::endl;
		std::string data;
		data = "node/" + std::to_string(ID);
		// std::cout<<data;
		m_blocked_set.insert(data);
		size--;
	}
	// std::cout<<std::endl;

	uint32_t size2 = i.ReadU32();

	while(size2 > 0){
		uint32_t ID;
		ID = i.ReadU32();
		std::string data;
		data = "node/" + std::to_string(ID);
		m_fullexit_set.insert(data);
		size2--;
	}

	uint32_t size3 = i.ReadU32();
	m_aoi_exitinfo.clear(); //map初期化
	while(size3 > 0){
		uint32_t ID;
		uint32_t can_exit_num;
		uint32_t time;
		std::pair<u_int32_t, uint32_t> pair;

		ID = i.ReadU32();
		can_exit_num = i.ReadU32();
		time = i.ReadU32();
		pair = std::make_pair(can_exit_num, time);
		std::string data;
		data = "node/" + std::to_string(ID);
		m_aoi_exitinfo.insert(std::make_pair(data, pair));
		size3--;
	}


	uint32_t size4 = i.ReadU32();
	m_aoi_blockinfo.clear(); //map初期化
	while(size4 > 0){
		uint32_t ID;
		float time;

		ID = i.ReadU32();
		time = i.ReadU32();
		// std::cout<<time<<std::endl;
		std::string data;
		data = "node/" + std::to_string(ID);
		m_aoi_blockinfo.insert(std::make_pair(data, time));
		size4--;
	}


	uint32_t dist = i.GetDistanceFrom(start);
	// std::cout<< dist <<"!!!!!!!!!!!" <<std::endl;
	// uint32_t buf = GetSerializedSize();
	// std::cout<< buf <<std::endl;
	// NS_ASSERT(dist == GetSerializedSize());
	// std::cout<<"デシリアライズ終了"<<std::endl;
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
	return (m_userId==o.m_userId && m_dst==o.m_dst && m_blocked_set == o.m_blocked_set && m_fullexit_set == o.m_fullexit_set && m_aoi_exitinfo == o.m_aoi_exitinfo);
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



}}


