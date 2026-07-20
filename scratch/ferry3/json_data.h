/*
 * json_data.h
 *
 *  Created on: 2021/07/05
 *      Author: matsunaga
 */

#ifndef SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_JSON_DATA_H_
#define SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_JSON_DATA_H_

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <math.h>
#include <string>
#include <sstream>
#include <list>
#include <unordered_map>

#include <nlohmann/json.hpp>
#include "hubeny.h"
#include "ns3/timer.h"
#include "ns3/rng-seed-manager.h"

using json = nlohmann::json;
using String =std::string;

extern json j;

namespace ns3{
namespace simple{

class way_node{
public:
	void SetCoordinate(json co){
		Coordinate=std::make_pair((double)co[json::json_pointer("/0")],(double)co[json::json_pointer("/1")]);
	}
	void SetWayIDs(String wayID){wayIDs.push_back(wayID);}
	void SetNodeID(String id){nodeID=id;}
	void SetCrossingflag(bool flag){isCrossing=flag;}
	void SetBlockedflag(bool flag){isBlocked=flag;}
	void SetStopflag(bool flag){isStop=flag;}
	void SetExitflag(bool flag){isExit=flag;}
	void SetNeighborNodes(std::map<std::pair<double,double>,double> n_nodes){neighbor_node_coordinate=n_nodes;}
	void SetNeighborNode(std::pair<double,double> n_co, double dist){
//		double dst;
//		Hubeny h;
//		h.hubeny_distance(n_co.first,n_co.second,Coordinate.first,Coordinate.second,dst);
		neighbor_node_coordinate.insert(std::make_pair(n_co,dist));
	}
	void SetWayNode(json co,String wayID,bool c_flag,bool b_flag,bool s_flag){
		SetCoordinate(co);
		SetWayIDs(wayID);
		SetCrossingflag(c_flag);
		SetBlockedflag(b_flag);
		SetStopflag(s_flag);
	}

	std::pair<double,double> GetCoordinate(){return Coordinate;}
	std::list<String> GetWayIDs(){return wayIDs;}
	String GetNodeID(){return nodeID;}
	bool GetCrossingflag(){return isCrossing;}
	bool GetBlockedflag(){return isBlocked;}
	bool GetStopflag(){return isStop;}
	bool GetExitflag(){return isExit;}
	std::map<std::pair<double,double>,double> GetNeighborNodes(){return neighbor_node_coordinate;}


	void Print();

	way_node(): Coordinate(), wayIDs(),nodeID(),isCrossing(false),isBlocked(false),isStop(false),isExit(false),neighbor_node_coordinate(){}

private:
	std::pair<double,double> Coordinate;
	std::list<String> wayIDs;
	String nodeID;
	bool isCrossing;			// crossing or not
	bool isBlocked;
	bool isStop;
	bool isExit;
	std::map<std::pair<double,double>,double> neighbor_node_coordinate; // neighbor node info (coordinate, distance)

};

class way_info{
public:
	void SetID(String id){w_id=id;}
	void SetProperties(json p){properties=p;}
	void SetWayLength(double l){w_length=l;}
	void SetNodes(way_node node){
		w_nodes.push_back(node);
//		ws.insert(std::make_pair(node.GetCoordinate(),node));
//		ws1.insert(std::make_pair(node.GetCoordinate(),node));
	}
	void SetNodeList(std::list<way_node> nodelist){
		w_nodes=nodelist;
//		ws.clear();
//		ws1.clear();
//		for(auto itr = nodelist.begin(); itr != nodelist.end(); ++itr){
//			way_node it = *itr;
//			ws.insert(std::make_pair(it.GetCoordinate(),it));
//			ws1.insert(std::make_pair(it.GetCoordinate(),it));
//		}
	}



	String GetID(){return w_id;}
	json GetProperties(){return properties;}
	std::list<way_node> GetNodes(){return w_nodes;}
	double GetWayLength(){return w_length;}
//	std::map<std::pair<double,double>,way_node> GetNodes1(){return ws;}

	void Print();

private:
	String w_id;
	json properties;
	std::list<way_node> w_nodes;
	double w_length;
//	std::map<std::pair<double,double>,way_node> ws;
//	std::unordered_map<std::pair<double,double>,way_node> ws1;

};


class JSON_Data{
public:
//	JSON_Data();
	static void json_input();
	void json_print(String s);

	//keido
	static double json_x(double lo);
	// ido
	static double json_y(double la);

	static double json_y_size(double la);//フィールドサイズを取得するためだけの関数

	static double calculate_distance(std::pair<double,double> first,std::pair<double,double> second);

//	void output_node_coordinates(std::ofstream fout, int nid);

	static double get_max_lo(){return longitude_max;}
	static double get_max_la(){return latitude_max;}
	static double get_min_lo(){return longitude_min;}
	static double get_min_la(){return latitude_max;}

	static std::list<json> get_features(){return features;}
	static std::map<String,way_info> get_linestrings(){return linestrings;}
	static std::map<std::pair<double,double>,way_node> get_waynodes(){return nodes;}
	static std::map<std::pair<double,double>,way_node> get_graph(){return graph;}
	static std::list<json> get_points(){return points;}
	static std::list<std::pair<double,double>> get_hinanjo(){return hinanjo;}
	static std::map<std::pair<double,double>, uint32_t> get_capacity(){return capacity;}

	static void update_mapinfo();
	static void update_linestrings();
	static void update_waynodes();
	static void update_graph();

	static Timer updTimer;
	static void UpdateMapinfoTimer (Time delay);
	static void SetNodenum(uint32_t n){nodenum=n;}
	static void SetNodeID(int id,std::pair<double,double> coord);
//	static void LinkIDtoCoordinate();
	// ここのmapでエラー
	static std::pair<double,double> GetCoordinatefromID(String id){return id_to_coordinate.at(id);}
	static String GetIDfromCoordinate(std::pair<double,double> coord){return nodes.at(coord).GetNodeID();};
	static uint32_t NodeIDstoi(String id);
	static uint32_t GetUpdateNum (void){return update_num;}

	static uint32_t get_rand_range(uint32_t min, uint32_t max);



private:
	static void SetGraph(way_node node);
//	static json js;
	static json j;
	static std::list<json> features;
	static std::map<String,way_info> linestrings;
//	std::list<way_node> nodes;
	static std::map<std::pair<double,double>,way_node> nodes;
	static std::map<std::pair<double,double>,way_node> graph;
	static std::map<std::pair<double,double>,way_node> s_graph;
	static std::list<json> points;
	static std::list<std::pair<double,double>> hinanjo;
	static std::map<std::pair<double,double>, uint32_t> capacity;

	// ido
	static double latitude_max;
	static double latitude_min;

	// keido
	static double longitude_max;
	static double longitude_min;

	static void Update_ClusterView(String id, String status);
	static uint32_t nodenum;

	static uint32_t update_num;

	static std::map<String,std::pair<double,double>> id_to_coordinate;
	static std::list<String> now_blocked;


};


}
}



#endif /* SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_JSON_DATA_H_ */
