#ifndef OO_SIMD_H
#define OO_SIMD_H

/*
 * On-Off sketch implemented by AVX2 SIMD instructions
 */

#include <x86intrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#include "OO_FPI.h"

template<>
struct OO_FPI<uint32_t,int32_t,8>::Bucket{
    uint32_t items[8];
    int32_t counters[8];

    inline uint32_t Match_Item(const uint32_t item){
        __m256i vec = _mm256_set1_epi32(item);
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)items));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline uint32_t Match_Counter(const int32_t counter){
        __m256i vec = _mm256_set1_epi32(counter);
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)counters));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline int32_t Query(const uint32_t item){
        uint32_t match = Match_Item(item);

        if (match != 0){
            uint32_t index = __builtin_ctz(match);
            return counters[index];
        }
        return 0;
    }
};


template<>
void OO_FPI<uint32_t,int32_t,8>::Insert(const uint32_t item, const int32_t window){
    uint32_t pos = hash(item) % length;

    uint32_t match = buckets[pos].Match_Item(item);

    if (match != 0){
        buckets[pos].counters[__builtin_ctz(match)] += bucketBitsets->SetByte(pos, match);
        return;
    }

    if(!sketchBitsets->Get(pos)){
        match = buckets[pos].Match_Counter(sketch[pos]);

        if (match == 0){
            sketch[pos] += 1;
            sketchBitsets->Set(pos);
        }
        else{
            uint32_t offset = __builtin_ctz(match);
            buckets[pos].items[offset] = item;
            buckets[pos].counters[offset] += 1;
            bucketBitsets->Set(pos, offset);
        }
    }
}

template<>
struct OO_FPI<uint64_t,int32_t,8>::Bucket{
    uint64_t items[8];
    int32_t counters[8];

    inline uint32_t Match_Item(const uint64_t item){
        __m256i vec = _mm256_set1_epi64x(item);
        __m256i cmp = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)items));
        __m256i cmp1 = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)(&items[4])));

        return (_mm256_movemask_pd((__m256d)cmp1) << 4) |
                         _mm256_movemask_pd((__m256d)cmp);
    }

    inline uint32_t Match_Counter(const int32_t counter){
        __m256i vec = _mm256_set1_epi32(counter);
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)counters));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline int32_t Query(const uint64_t item){
        uint32_t match = Match_Item(item);

        if (match != 0){
            uint32_t index = __builtin_ctz(match);
            return counters[index];
        }
        return 0;
    }
};


template<>
void OO_FPI<uint64_t,int32_t,8>::Insert(const uint64_t item, const int32_t window){
    uint32_t pos = hash(item) % length;

    uint32_t match = buckets[pos].Match_Item(item);

    if (match != 0){
        buckets[pos].counters[__builtin_ctz(match)] += bucketBitsets->SetByte(pos, match);
        return;
    }

    if(!sketchBitsets->Get(pos)){
        match = buckets[pos].Match_Counter(sketch[pos]);

        if (match == 0){
            sketch[pos] += 1;
            sketchBitsets->Set(pos);
        }
        else{
            uint32_t offset = __builtin_ctz(match);
            buckets[pos].items[offset] = item;
            buckets[pos].counters[offset] += 1;
            bucketBitsets->Set(pos, offset);
        }
    }
}

template<>
struct OO_FPI<uint32_t,int32_t,32>::Bucket{
    uint32_t items[32];
    int32_t counters[32];

    inline uint32_t Match_Item(const __m256i& vec, uint32_t offset){
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)(&items[offset << 3])));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline uint32_t Match_Counter(const __m256i& vec, uint32_t offset){
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)(&counters[offset << 3])));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline int32_t Query(const uint32_t item){
	__m256i vec = _mm256_set1_epi32(item);

    	for(uint32_t i = 0;i < 4;++i){
            uint32_t match = Match_Item(vec, i);
            if(match != 0){
            	uint32_t index = __builtin_ctz(match);
            	return counters[(i << 3) + index];
	    }
        }

        return 0;
    }
};


template<>
void OO_FPI<uint32_t,int32_t,32>::Insert(const uint32_t item, const int32_t window){
    uint32_t pos = hash(item) % length;
    __m256i vec = _mm256_set1_epi32(item);

    for(uint32_t i = 0;i < 4;++i){
	uint32_t match = buckets[pos].Match_Item(vec, i);
    	if(match != 0){
	    buckets[pos].counters[(i << 3) + __builtin_ctz(match)] += bucketBitsets->SetByte((pos << 2) + i, match);
	    return;
	}
    }

    if(!sketchBitsets->Get(pos)){
    	vec = _mm256_set1_epi32(sketch[pos]);
	for(uint32_t i = 0;i < 4;++i){
     	    uint32_t match = buckets[pos].Match_Counter(vec, i);
            if(match != 0){
            	uint32_t offset = __builtin_ctz(match);
            	buckets[pos].items[(i << 3) + offset] = item;
           	buckets[pos].counters[(i << 3) + offset] += 1;
            	bucketBitsets->Set((pos << 2) + i, offset);
		return;
            }
	}

        sketch[pos] += 1;
        sketchBitsets->Set(pos);
    }
}

template<>
struct OO_FPI<uint64_t,int32_t,32>::Bucket{
    uint64_t items[32];
    int32_t counters[32];

    inline uint32_t Match_Item(const __m256i& vec, uint32_t offset){
        __m256i cmp = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)(&items[offset << 3])));
        __m256i cmp1 = _mm256_cmpeq_epi64(vec, _mm256_loadu_si256((__m256i*)(&items[(offset << 3) + 4])));

	return (_mm256_movemask_pd((__m256d)cmp1) << 4) |
                         _mm256_movemask_pd((__m256d)cmp);
    }

    inline uint32_t Match_Counter(const __m256i& vec, uint32_t offset){
        __m256i cmp = _mm256_cmpeq_epi32(vec, _mm256_loadu_si256((__m256i*)(&counters[offset << 3])));
        return _mm256_movemask_ps((__m256)cmp);
    }

    inline int32_t Query(const uint64_t item){
	__m256i vec = _mm256_set1_epi64x(item);

        for(uint32_t i = 0;i < 4;++i){
            uint32_t match = Match_Item(vec, i);
            if(match != 0){
                uint32_t index = __builtin_ctz(match);
                return counters[(i << 3) + index];
            }
        }

        return 0;
    }
};


template<>
void OO_FPI<uint64_t,int32_t,32>::Insert(const uint64_t item, const int32_t window){
    uint32_t pos = hash(item) % length;
    __m256i vec = _mm256_set1_epi64x(item);

    for(uint32_t i = 0;i < 4;++i){
        uint32_t match = buckets[pos].Match_Item(vec, i);
        if(match != 0){
            buckets[pos].counters[(i << 3) + __builtin_ctz(match)] += bucketBitsets->SetByte((pos << 2) + i, match);
            return;
        }
    }

    if(!sketchBitsets->Get(pos)){
        vec = _mm256_set1_epi32(sketch[pos]);
        for(uint32_t i = 0;i < 4;++i){
            uint32_t match = buckets[pos].Match_Counter(vec, i);
            if(match != 0){
                uint32_t offset = __builtin_ctz(match); 
                buckets[pos].items[(i << 3) + offset] = item;
                buckets[pos].counters[(i << 3) + offset] += 1;
                bucketBitsets->Set((pos << 2) + i, offset);
                return;
            }
        }
        
        sketch[pos] += 1;
        sketchBitsets->Set(pos);
    }
}

#endif //OO_SIMD_H
