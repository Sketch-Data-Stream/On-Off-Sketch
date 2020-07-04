#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/stat.h>

#include <chrono>

#include "SS.h"
#include "PIE.h"
#include "OO_FPI.h"
#include "OO_SIMD.h"

#include "CM_HT.h"
#include "CM_BF.h"
#include "OO_PE.h"


template<typename DATA_TYPE,typename COUNT_TYPE>
class BenchMark{
public:

    typedef std::vector<Abstract<DATA_TYPE, COUNT_TYPE>*> AbsVector;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    BenchMark(const char* _PATH, const COUNT_TYPE _T):
            PATH(_PATH){
	    struct stat statbuf;
	    stat(PATH, &statbuf);
	    LENGTH = ceil(statbuf.st_size / (double)(_T * sizeof(DATA_TYPE)));

        FILE* file = fopen(PATH, "rb");

        COUNT_TYPE number = 0;
        DATA_TYPE item;
        HashMap record;

        TOTAL = 0;
        T = 0;

        while(fread(&item, sizeof(DATA_TYPE), 1, file) > 0){
            if(number % LENGTH == 0)
                T += 1;
            number += 1;

            if(record[item] != T){
                record[item] = T;
                mp[item] += 1;
                TOTAL += 1;
            }
        }

        fclose(file);
    }

    void SketchError(uint32_t section){
        AbsVector PEs = {
                new OO_PE<DATA_TYPE, COUNT_TYPE>(3, 20000000 / 3.0 / (BITSIZE + sizeof(COUNT_TYPE))),
        };

        BenchInsert(PEs);

        for(auto PE : PEs){
            CheckDistribution(PE, section);
            PECheckError(PE);
            delete PE;
        }
    }

    void TopKError(double alpha){
        AbsVector FPIs = {
                new OO_FPI<DATA_TYPE, COUNT_TYPE, 8>(200000),
	    };

        BenchInsert(FPIs);

        for(auto FPI : FPIs){
            FPICheckError(FPI, alpha * TOTAL);
            delete FPI;
        }
    }

    void Thp(){
        for(uint32_t i = 0;i < 5;++i){
	        std::cout << i << std::endl;

	        AbsVector FPIs = {
	                new OO_PE<DATA_TYPE, COUNT_TYPE>(3, 20000000 / 3.0 / (BITSIZE + sizeof(COUNT_TYPE))),
                    new OO_FPI<DATA_TYPE, COUNT_TYPE, 8>(200000),
		    };

            for(auto FPI : FPIs){
                InsertThp(FPI);
                delete FPI;
            }
        }
    }

private:

    double TOTAL;
    COUNT_TYPE T;
    COUNT_TYPE LENGTH;

    HashMap mp;
    const char* PATH;

    typedef std::chrono::high_resolution_clock::time_point TP;

    inline TP now(){
        return std::chrono::high_resolution_clock::now();
    }

    inline double durationms(TP finish, TP start){
        return std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
    }

    void BenchInsert(AbsVector sketches){
        FILE* file = fopen(PATH, "rb");
        DATA_TYPE item;
        COUNT_TYPE number = 0, windowId = 0;

        while(fread(&item, sizeof(DATA_TYPE), 1, file) > 0){
            if(number % LENGTH == 0){
                windowId += 1;
                for(auto sketch : sketches)
                    sketch->NewWindow(windowId);
            }
            number += 1;

            for(auto sketch : sketches)
                sketch->Insert(item, windowId);
        }

        fclose(file);
    }

    void InsertThp(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        TP start, finish;

        FILE* file = fopen(PATH, "rb");
        DATA_TYPE item;
        COUNT_TYPE number = 0, windowId = 0;
        HashMap record;

        start = now();
        while(fread(&item, sizeof(DATA_TYPE), 1, file) > 0){
            if(number % LENGTH == 0){
                windowId += 1;
                sketch->NewWindow(windowId);
            }
            number += 1;

            sketch->Insert(item, windowId);
        }
        finish = now();

        fclose(file);
	std::cout << "Thp: " << number / durationms(finish, start) << std::endl; 
    }

    void FPICheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch, COUNT_TYPE HIT){
        double real = 0, estimate = 0, both = 0;
        double aae = 0, cr = 0, pr = 0, f1 = 0;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);

            if(value > HIT){
                estimate += 1;
                if(it->second > HIT) {
                    both += 1;
                    aae += abs(it->second - value);
                }
            }
            if(it->second > HIT)
                real += 1;
        }

        if(both <= 0){
            std::cout << "Not Find Real Persistent" << std::endl;
        }
        else{
            aae /= both;
        }

        cr = both / real;

        if(estimate <= 0){
            std::cout << "Not Find Persistent" << std::endl;
        }
        else{
            pr = both / estimate;
        }

        if(cr == 0 && pr == 0)
            f1 = 0;
        else
            f1 = (2*cr*pr)/(cr+pr);

	    std::cout << HIT << std::endl
		    << "AAE: " << aae << std::endl
		    << "F1: " << f1 << std::endl;
    }

    void PECheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        double aae = 0;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);

            aae += abs(it->second - value);
        }

	    std::cout << "AAE: " << aae / mp.size() << std::endl;
    }

    void CheckDistribution(Abstract<DATA_TYPE, COUNT_TYPE>* sketch, const uint32_t section){
        uint32_t* aae = new uint32_t[section];
        uint32_t* number = new uint32_t[section];

        memset(aae, 0, sizeof(uint32_t) * section);
        memset(number, 0, sizeof(uint32_t) * section);

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);
            uint32_t pos = (it->second * section - 1) / T;

            aae[pos] += abs(it->second - value);
            number[pos] += 1;
        }

        for(uint32_t i = 0;i < section;++i){
            if(number[i] != 0)
                std::cout << aae[i] / (double)number[i] << ",";
            else
                std::cout << "NULL,";
        }
        std::cout << std::endl;

        delete [] aae;
        delete [] number;
    }
};

#endif //BENCHMARK_H
