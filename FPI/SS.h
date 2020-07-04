#ifndef WINDOW_SKETCH_SS_H
#define WINDOW_SKETCH_SS_H

/*
 * Small-Space
 */

#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class SS : public Abstract<DATA_TYPE, COUNT_TYPE> {
public:
    struct PCounter {
        COUNT_TYPE window;
        COUNT_TYPE count;

        PCounter(COUNT_TYPE _window = 0, COUNT_TYPE _count = 0):
            window(_window), count(_count){}
    };

    typedef std::unordered_map<DATA_TYPE, PCounter> SSMap;
    typedef std::unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    SS(double _EPSILON):EPSILON(_EPSILON){}

    ~SS(){
	std::cout << "Memory: " << ssmap.size() * 5 * (sizeof(PCounter) + sizeof(DATA_TYPE)) << std::endl;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        if(ssmap.find(item) != ssmap.end()){
            if(ssmap[item].window < window){
                ssmap[item].window = window;
                ssmap[item].count += 1;
            }
        }
        else{
            uint8_t str[sizeof(DATA_TYPE) + sizeof(COUNT_TYPE)];
            memcpy(str, &item, sizeof(DATA_TYPE));
            memcpy(str + sizeof(DATA_TYPE), &window, sizeof(COUNT_TYPE));

            if(Hash::BOBHash32(str, sizeof(DATA_TYPE) + sizeof(COUNT_TYPE), 0)
               / (double)(UINT_MAX) < EPSILON){
                ssmap[item] = PCounter(window, 1);
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        if(ssmap.find(item) == ssmap.end())
            return 0;
        return ssmap[item].count + (1 / EPSILON);
    }

    void NewWindow(const COUNT_TYPE window){}

private:

    const double EPSILON;
    SSMap ssmap;
};

#endif //SS_H
