#ifndef PIE_H
#define PIE_H

#include <vector>
#include <random>

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class PIE : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:

    struct Cell{
        uint8_t isOccupy : 1;
        uint8_t raptor : 1;
        uint8_t fprint : 6;

#define MASK_PRINT 0x3f
        inline bool Empty(){
            return (*((uint8_t*)this)) == 0;
        }

        inline bool Valid(){
            return *((uint8_t*)this) != INT8_MAX;
        }

        inline void InValidate(){
            *((uint8_t*)this) = INT8_MAX;
        }

        void Set(const DATA_TYPE item, const DATA_TYPE randnum){
            isOccupy = 1;
            raptor = Encode(item, randnum);
            fprint = (hash(item, 101) & MASK_PRINT);
        }

        bool Equal(const DATA_TYPE item, const DATA_TYPE randnum){
            return (isOccupy == 1) &&
                   (raptor == Encode(item, randnum)) &&
                   (fprint == (hash(item, 101) & MASK_PRINT));
        }

        inline uint8_t Encode(const DATA_TYPE item, const DATA_TYPE randnum){
            return OddOnes(randnum & item);
        }

        inline uint8_t OddOnes(uint32_t x){
            x ^= (x >> 1);
            x ^= (x >> 2);
            x ^= (x >> 4);
            x ^= (x >> 8);
            x ^= (x >> 16);
            return x & 1;
        }

        inline uint32_t hash(DATA_TYPE item, uint32_t seed = 0){
            return Hash::BOBHash32((uint8_t*)&item, sizeof(DATA_TYPE), seed);
        }
    };

    struct STBF{

        STBF(uint32_t _length, uint32_t _hash_num, COUNT_TYPE window):
            length(_length), hash_num(_hash_num){
            cell = new Cell [length];
            memset(cell, 0, length * sizeof(Cell));

            std::uniform_int_distribution<DATA_TYPE> dist;
            std::default_random_engine rd(window);
            randnum = dist(rd);
        }

        void Insert(const DATA_TYPE item){
            for(uint32_t i = 0;i < hash_num;++i){
                uint32_t pos = hash(item, i) % length;

                if(cell[pos].Empty())
                    cell[pos].Set(item, randnum);
                else if(!cell[pos].Equal(item, randnum))
                    cell[pos].InValidate();
            }
        }

        bool Query(const DATA_TYPE item){
            uint32_t invalid = 0;

            for(uint32_t i = 0;i < hash_num;++i){
                uint32_t pos = hash(item, i) % length;

                if(cell[pos].Empty())
                    return false;
                else if(cell[pos].Valid() && (!cell[pos].Equal(item, randnum))){
                    return false;
                }

                invalid += (!cell[pos].Valid());
            }

            return invalid != hash_num;
        }

        inline uint32_t hash(DATA_TYPE data, uint32_t seed = 0){
            return Hash::BOBHash32((uint8_t*)&data, sizeof(DATA_TYPE), seed);
        }

        Cell* cell;
        DATA_TYPE randnum;
        const uint32_t length;
        const uint32_t hash_num;

    };

    PIE(uint64_t memory, uint32_t T, uint32_t _hash_num = 1):
            length((double)memory / (T * sizeof(Cell))),
            hash_num(_hash_num){
        stbfs.emplace_back(length, hash_num, 0);
    }

    ~PIE(){
        for(STBF stbf : stbfs){
            delete [] stbf.cell;
        }
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        stbfs[window].Insert(item);
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        COUNT_TYPE ret = 0;

        for(STBF stbf : stbfs){
            ret += stbf.Query(item);
        }

        return ret;
    }

    void NewWindow(const COUNT_TYPE window){
        stbfs.emplace_back(length, hash_num, window);
    }

private:
    const uint32_t length;
    const uint32_t hash_num;

    std::vector<STBF> stbfs;
};

#endif //PIE_H
