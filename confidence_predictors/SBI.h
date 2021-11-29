#ifndef SBI_H
#define SBI_H

#include <stdio.h>
#include <stdlib.h>

#define CHT_SIZE 16384

inline uint16_t clamp(int16_t value, uint16_t max)
{
  if (value < 0)
    value = 0;
  else 
  if (value > max)
    value = max;
  return value;
}
 
/* How SBI works:
    - We access a conventional branch predictor, such as a Gshare or Bi-Mode predictor, in parallel with a confidence
    estimator.
    
    - The branch prediction counter is compared to the prediction threshold to predict taken or not-taken. The confidence counter is
    compared to the confidence threshold
    
    - If the confidence counter is greater than or equal to the threshold, the branch prediction is labeled high-confidence, meaning that the branch prediction is likely to be correct. Otherwise, the prediction is labeled low-confidence and the prediction is believed to be incorrect

    - The branch prediction is inverted for all low-confidence branches.

References: 
    https://link.springer.com/content/pdf/10.1023/A:1026436021514.pdf
*/

class SBI {
    uint16_t CONF_THRESHOLD; //confidence threshold
    uint16_t PRED_THRESHOLD; //prediction threshold

    /* Confidence table:
        - 0: Not taken High Confidence
        - 1: Not Taken Low Confidence
        - 2: Taken Low Confidence
        - 3: Taken High Confidence
    */    
    int CHT[CHT_SIZE];


public:
    SBI(uint16_t confidence, uint16_t prediction) {
      CONF_THRESHOLD = confidence;
      PRED_THRESHOLD = prediction;
      for(int i = 0; i < CHT_SIZE; i++){
        CHT[i] = 0;
      }
    }

    ~SBI() {
    }
    uint SBI_hash(uint64_t ip, int SBI_vector) {
      unsigned int sbi_mask = (1 << CHT_SIZE) - 1;
      unsigned int hash = (ip&(sbi_mask) | (SBI_vector&sbi_mask));
      hash = hash%CHT_SIZE;
      return hash;
    }

    uint SBI_inverse(uint64_t ip, int hash, int prediction) 
    {
      return (~prediction * ~ip + prediction * hash) % 2;
    }

    uint predict(uint64_t ip, int prediction) {
      int sbi_hash = SBI_hash(ip,prediction);
      if(CHT[sbi_hash] >= CONF_THRESHOLD) {
        return prediction;
      } else {
        return SBI_inverse(ip,sbi_hash,prediction); 
      }
    }

    int get_confidence(uint64_t ip, int prediction) {
      int sbi_hash = SBI_hash(ip,prediction);
      if(CHT[sbi_hash] == 0 || CHT[sbi_hash] == 3){
        return 1;
      }
      return 0;
    }

    void update_confidence(uint64_t ip, uint8_t taken) {
        int sbi_hash = SBI_hash(ip,taken);
        // Correct prediction 
        if(taken == 1) {
          // Increase confidence
          if (CHT[sbi_hash] < 3) {
            CHT[sbi_hash]++;
          }
        }
        else 
        { // Lower confidence 
          if(CHT[sbi_hash] > 0){
              CHT[sbi_hash]--;
          }
        }
    }
};

#endif