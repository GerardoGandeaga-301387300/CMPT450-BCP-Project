#ifndef EJRS_H
#define EJRS_H

#include <stdint.h>

class EJRS{
private:
    uint64_t MDC[4096];
    const int THRESHOLD = 15;
    int GHR_size;
    int bh_vector;
    uint64_t GLOBAL_HISTORY_MASK;

public:
    EJRS(){
        for(int i = 0; i < 4096; i++){
            MDC[i] = THRESHOLD;
        }
        GHR_size = 12;
        bh_vector = 0;
        GLOBAL_HISTORY_MASK = (1 << GHR_size) - 1;
    }

    EJRS(int GHR_size){
        for(int i = 0; i < 4096; i++){
            MDC[i] = THRESHOLD;
        }
        this->GHR_size = GHR_size;
        bh_vector = 0;
        GLOBAL_HISTORY_MASK = (1 << GHR_size) - 1;
    }

    uint64_t get_counter(uint64_t ip, uint64_t pred){
        uint64_t hashed_idx = ip^(ip>>GHR_size)^(ip>>(GHR_size*2))^bh_vector^pred;
        hashed_idx = hashed_idx%4096;

        return MDC[hashed_idx];
    }

    int get_confidence(uint64_t ip, uint64_t pred){
        uint64_t hashed_idx = ip^(ip>>GHR_size)^(ip>>(GHR_size*2))^bh_vector^pred;
        hashed_idx = hashed_idx%4096;

        int confidence_level = (MDC[hashed_idx] >= THRESHOLD) ? 1 : 0;
        return confidence_level;
    }

    void update_table(uint64_t ip, int taken, int last_pred){
        uint64_t hashed_idx = ip^(ip>>14)^(ip>>(14*2))^bh_vector^last_pred;
        hashed_idx = hashed_idx%4096;

        if(last_pred == taken){
            if(MDC[hashed_idx] < THRESHOLD)
                MDC[hashed_idx]++;
        } else{
            MDC[hashed_idx] = 0;
        }

        // update branch history vector
        bh_vector <<= 1;
        bh_vector &= GLOBAL_HISTORY_MASK;
        bh_vector |= taken;
    }
};

#endif