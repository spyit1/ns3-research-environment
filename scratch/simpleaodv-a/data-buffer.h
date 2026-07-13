/*
 * data-buffer.h
 *
 *  Created on: 2015/11/26
 *      Author: terami
 */

#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include <list>
#include <unordered_map>
#include "cluster-list.h"

namespace ns3{
namespace simple{

class DataBuffer : public std::unordered_map<DataKey,Data,DataKey::Hash> {
public:
	void AddData(Data data){
		std::cout<< "empty? before " << this->empty() << std::endl;
		this->emplace(data.GetKey(),data); 
		std::cout<< "empty? after " << this->empty() << std::endl;
		std::cout<< "size " << this->size() << std::endl;
		std::cout<< "find add " <<(this->find(data.GetKey())!=this->end()) << std::endl;
	}
	bool GetData(DataKey dk,Data &data){
		
		std::cout<< "find " <<(this->find(dk)!=this->end()) << std::endl;

		for(auto it = this->begin();it != this->end();it++){
			std::pair<DataKey,Data> pair=*it;
			if((dk.GetId() == pair.first.GetId()) && (dk.GetSeqNo() == pair.first.GetSeqNo())){
				data = pair.second;
				std::cout<<"true truetruetruetruetrue\n";
				return true;
			}				
		}
		if(this->find(dk)!=this->end()){ // Find function does not work here in ns-3.35. Why?
			data=this->at(dk);
			std::cout<<"true aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
			std::cout<<data<<" aaaaa\n";
			return true;
		}else{
			std::cout<<"false aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
			std::cout<<data<<" aaaaa\n";
			return false;
		}
	}
	DataKeyList MakeKeyList(){
		DataKeyList dkl;
		DataBuffer::iterator dbi;
		for(dbi=this->begin();dbi!=this->end();dbi++){
			std::pair<DataKey,Data> pair=*dbi;
			dkl.emplace(pair.first);
		}
		return dkl;
	}
	void Print(){
		DataKeyList dkl=this->MakeKeyList();
		dkl.Print();
	}
};

}
}

#endif /* DATA_BUFFER_H_ */
