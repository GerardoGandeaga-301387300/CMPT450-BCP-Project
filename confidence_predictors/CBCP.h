#ifndef CBCP_H
#define CBCP_H

#include <stdint.h>
#include "SBI.h"
#include "EJRS.h"
#include "UDCE.h"

class CBCP{
private:
    bool default_config;
    SBI *estimator1;
    EJRS *estimator2;
    UDCE *estimator3;

public:
    CBCP(){
        default_config = 1;
        // conf_est_1 = new BPRU(131072, 64, 7);
    }

    CBCP(SBI &estimator1, EJRS &estimator2, UDCE &estimator3){
        default_config = 0;
        this->estimator1 = &estimator1;
        this->estimator2 = &estimator2;
        this->estimator3 = &estimator3;
    }

    // Deallocation of BPRU and SBI should be handled in the program that created the object
    ~CBCP(){
        if(default_config)
            default_config = 1;
    }

    uint8_t get_confidence(uint64_t ip, uint8_t pred) {
        int raw_output_1 = estimator1->get_confidence(ip, pred);
        int raw_output_2 = estimator2->get_confidence(ip, pred);
        int raw_output_3 = estimator3->get_confidence(ip, pred);
        uint64_t raw_output = raw_output_1 + raw_output_2 + raw_output_3;

        // Current threshold is 1
        return (raw_output >= 3) ? 1 : 0;
    }

    void update_confidence_estimators(uint64_t ip, uint8_t taken, int last_prediction){
        estimator1->update_confidence(ip, taken);
        estimator2->update_table(ip, taken, last_prediction);
        estimator3->update_table(ip, taken, last_prediction);
    }   
};

#endif
