#ifndef CM_HT_H
#define CM_HT_H

/*
 * Count-Min sketch with a Hash Table
 */

#include <unordered_set>

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class CM_HT : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    CM_HT(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        counters = new COUNT_TYPE* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counters[i] = new COUNT_TYPE [length];
            memset(counters[i], 0, length * sizeof(COUNT_TYPE));
        }
    }

    ~CM_HT(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] counters[i];
        }
        delete [] counters;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        if(st.find(item) == st.end()){
            st.insert(item);
            for(uint32_t i = 0;i < hash_num;++i){
                uint32_t pos = this->hash(item, i) % length;
                counters[i][pos] += 1;
            }
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
        st.clear();
    }

private:
    const uint32_t hash_num;
    const uint32_t length;

    std::unordered_set<DATA_TYPE> st;
    COUNT_TYPE** counters;
};

#endif //CM_HT_H
