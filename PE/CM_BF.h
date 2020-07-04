#ifndef CM_BF_H
#define CM_BF_H

/*
 * Count-Min sketch with a Bloom filter
 */

#include "bitset.h"
#include "Abstract.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class CM_BF : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    struct BF{
#define BFSEED 101

        BF(uint32_t _hash_num, uint32_t _length):
                hash_num(_hash_num), length(_length){
            bf = new BitSet(length);
        }

        ~BF(){
            delete bf;
        }

        bool find(const DATA_TYPE item){
            for(uint32_t i = 0;i < hash_num;++i){
                uint32_t pos = hash(item, i + BFSEED) % length;
                if(!bf->Get(pos))
                    return false;
            }
            return true;
        }

        void insert(const DATA_TYPE item){
            for(uint32_t i = 0;i < hash_num;++i){
                uint32_t pos = hash(item, i + BFSEED) % length;
                bf->Set(pos);
            }
        }

        void clear(){
            bf->Clear();
        }

        inline uint32_t hash(DATA_TYPE data, uint32_t seed = 0){
            return Hash::BOBHash32((uint8_t*)&data, sizeof(DATA_TYPE), seed);
        }

        BitSet* bf;

        const uint32_t hash_num;
        const uint32_t length;
    };

    CM_BF(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        bf = new BF(_hash_num * 2, _length * _hash_num * 8);
        counters = new COUNT_TYPE* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counters[i] = new COUNT_TYPE [length];
            memset(counters[i], 0, length * sizeof(COUNT_TYPE));
        }
    }

    ~CM_BF(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] counters[i];
        }
        delete bf;
        delete [] counters;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        if(!bf->find(item)){
            bf->insert(item);
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
        bf->clear();
    }

private:
    BF* bf;

    const uint32_t hash_num;
    const uint32_t length;

    COUNT_TYPE** counters;
};

#endif //CM_BF_H
