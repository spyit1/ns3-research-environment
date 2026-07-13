/*
 * unicast-data-container.h
 *
 *  Created on: Jan 20, 2017
 *      Author: terami
 */

#ifndef SCRATCH_MY_CLUSTER_UNICAST_DATA_CONTAINER_H_
#define SCRATCH_MY_CLUSTER_UNICAST_DATA_CONTAINER_H_

#include <unordered_set>
#include "cluster-list.h"

namespace ns3{
namespace simple{

class UnicastDataContainer : public std::unordered_set<DataKey,DataKey::Hash>{
public:
	void PushDataKey(DataKey dk){
		this->emplace(dk);
	}

	bool GetDataKey(DataKey dk){
		if(this->find(dk)!=this->end()){
			this->erase(dk);
			return true;
		}else{
			return false;
		}
	}
};


}
}



#endif /* SCRATCH_MY_CLUSTER_UNICAST_DATA_CONTAINER_H_ */
