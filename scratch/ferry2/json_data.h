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
using Pos = std::pair<double, double>;
using Weight = double;
const Weight inf = std::numeric_limits<Weight>::max();

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
	void SetRescueflag(bool flag){need_rescue = flag;}
	void SetCongestionflag(bool flag){iscongestion = flag;}
	void SetNeighborNodes(std::map<std::pair<double,double>,double> n_nodes){neighbor_node_coordinate=n_nodes;}
	void SetNeighborNode(std::pair<double,double> n_co, double dist){
		neighbor_node_coordinate.insert(std::make_pair(n_co,dist));
	}
	void SetNeighborNodeforUAV(std::pair<double,double> n_co, double dist){
		neighbor_node_coordinate_for_uav.insert(std::make_pair(n_co,dist));
	}
	void SetWayNode(json co, String wayID, bool c_flag, bool b_flag, bool s_flag, bool r_flag, bool congestion_flag){
		SetCoordinate(co);
		SetWayIDs(wayID);
		SetCrossingflag(c_flag);
		SetBlockedflag(b_flag);
		SetStopflag(s_flag);
		SetRescueflag(r_flag);
		SetCongestionflag(congestion_flag);
	}

	std::pair<double,double> GetCoordinate(){return Coordinate;}
	std::list<String> GetWayIDs(){return wayIDs;}
	String GetNodeID(){return nodeID;}
	bool GetCrossingflag()const{return isCrossing;}
	bool GetBlockedflag(){return isBlocked;}
	bool GetStopflag(){return isStop;}
	bool GetExitflag(){return isExit;}
	bool GetRescueflag(){return need_rescue;}
	bool GetCongestionflag(){return iscongestion;}
	std::map<std::pair<double,double>,double> GetNeighborNodes()const{return neighbor_node_coordinate;}
	std::map<std::pair<double,double>,double> GetNeighborNodesforUAV()const{return neighbor_node_coordinate_for_uav;}
	void SetShortestPath(const std::vector<std::pair<double,double>>& path) { 
		shortest_path_to_nearest_shelter = path; 
	}
	void SetNearestShelter(std::pair<double,double> shelter_coord) { 
		nearest_shelter_coord = shelter_coord; 
	}
	const std::vector<std::pair<double,double>>& GetShortestPath() const { 
		return shortest_path_to_nearest_shelter; 
	}
	std::pair<double,double> GetNearestShelter() const { 
		return nearest_shelter_coord; 
	}


	void Print();

	way_node(): Coordinate(), wayIDs(),nodeID(),isCrossing(false),isBlocked(false),isStop(false),isExit(false),neighbor_node_coordinate(){}

private:
	std::pair<double,double> Coordinate;
	std::vector<std::pair<double,double>> shortest_path_to_nearest_shelter; //最寄りの避難所への最短経路（通行止めがない場合）
	std::pair<double,double> nearest_shelter_coord; //最寄りの避難所の座標
	std::list<String> wayIDs;
	String nodeID;
	bool isCrossing;			// crossing or not
	bool isBlocked;
	bool isStop;
	bool isExit;
	bool need_rescue; //要救助フラグ
	bool iscongestion; //混雑

	std::map<std::pair<double,double>,double> neighbor_node_coordinate; // neighbor node info (coordinate, distance)
	std::map<std::pair<double,double>,double> neighbor_node_coordinate_for_uav;

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
    // static void dijkstra_pair(Pos start, const std::map<Pos, way_node>& nodes_map, std::map<Pos, Weight>& dist, std::map<Pos, Pos>& parent);
	static void precalculate_shortest_paths();
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
	static void DisplayAllShortestPaths();



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
	static std::list<String> now_rescue;
	static std::list<String> now_congestion;

};


}
}



#endif /* SCRATCH_CLUSTER_EVACUATION_FOR_GEOGRAPHIC_INFO_JSON_DATA_H_ */
