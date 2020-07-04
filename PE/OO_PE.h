#ifndef OO_PE_H
#define OO_PE_H

/*
 * On-Off sketch on persistence estimation
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class OO_PE : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    OO_PE(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        counters = new COUNT_TYPE* [hash_num];
        bitsets = new BitSet* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counters[i] = new COUNT_TYPE [length];
            bitsets[i] = new BitSet(length);
            memset(counters[i], 0, length * sizeof(COUNT_TYPE));
        }
    }

    ~OO_PE(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] counters[i];
            delete bitsets[i];
        }
        delete [] counters;
        delete [] bitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            counters[i][pos] += (!bitsets[i]->SetNGet(pos));
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = INT_MAX;
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            ret = MIN(ret, counters[i][pos]);
        }
        return ret;
    }

    void NewWindow(const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
        }
    }

private:
    const uint32_t hash_num;
    const uint32_t length;

    BitSet** bitsets;
    COUNT_TYPE** counters;
};

#endif //OO_PE_H
