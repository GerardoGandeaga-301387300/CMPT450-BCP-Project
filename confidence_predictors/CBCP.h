#ifndef CBCP_H
#define CBCP_H

#include <stdint.h>
#include "BPRU.h"
#include "EJRS.h"
#include "UDCE.h"

class CBCP{
private:
    bool default_config;
    BPRU *estimator1;
    EJRS *estimator2;
    UDCE *estimator3;
    int THRESHOLD;

public:
    CBCP(){
        default_config = 1;
        estimator1 = NULL;
        estimator2 = NULL;
        estimator3 = NULL;
        // conf_est_1 = new BPRU(131072, 64, 7);
    }

    CBCP(BPRU &estimator1, EJRS &estimator2, UDCE &estimator3){
        default_config = 0;
        this->estimator1 = &estimator1;
        this->estimator2 = &estimator2;
        this->estimator3 = &estimator3;
        THRESHOLD = 3;
    }

    CBCP(EJRS &estimator2, UDCE &estimator3){
        default_config = 0;
        this->estimator1 = NULL;
        this->estimator2 = &estimator2;
        this->estimator3 = &estimator3;
        THRESHOLD = 2;
    }

    uint8_t get_confidence(uint64_t ip, uint8_t pred) {
        if(default_config){
            return 1;
        }

        int raw_output_1 = 0;
        if(estimator1){
            raw_output_1 = estimator1->reverse_prediction(ip, pred);
            raw_output_1 = (raw_output_1 == pred) ? 1 : 0;
        }

        int raw_output_2 = estimator2->get_confidence(ip, pred);
        int raw_output_3 = estimator3->get_confidence(ip, pred);
        uint64_t raw_output = raw_output_1 + raw_output_2 + raw_output_3;

        return (raw_output >= THRESHOLD) ? 1 : 0;
    }

    void update_confidence_estimators(uint64_t ip, uint8_t taken, int last_prediction){
        if(default_config){
            return;
        }
        
        if(estimator1)
            estimator1->update_RT(ip, taken);
        estimator2->update_table(ip, taken, last_prediction);
        estimator3->update_table(ip, taken, last_prediction);
    }   
};

#endif
