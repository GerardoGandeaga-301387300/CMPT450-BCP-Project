#ifndef BPRU_H
#define BPRU_H

#include <vector>
#include <algorithm>

inline uint16_t clamp(int16_t value, uint16_t max)
{
  if (value < 0)
    value = 0;
  else 
  if (value > max)
    value = max;
    
  return value;
}

/*
Architecture of the BPRU:
  - We have a Reversal Table that has a tag to reduce aliasing and a saturating counter 
    which counts up/down for correct/incorrect branch predictions. 
  - We use the most significant bit og the RT value to take or not take a branch
  - PHR = an N bit register than shifts 2 bits to the left and takes the least significant 
    2 bits from the PC and adds it to the PHR
  - We have a HASH function that hashed into the RT with input from the Path History Register
    and PC information. 
  - Branch predictor evaluation is XORed with RT output for the final branch prediction

References:
  - https://www.ditec.um.es/~jmgarcia/papers/bpru-hipc-final.pdf
  - https://american.cs.ucdavis.edu/academic/readings/papers/brpredreversal.pdf
*/
class BPRU 
{
private:
  // path history register
  uint64_t PHR_SIZE;
  uint64_t RT_SIZE;
  uint16_t MAX_COUNTER;

  vector<uint8_t> PHR;
  uint16_t **RT;

  uint64_t PHR_value() 
  {
    uint64_t value = 0x0;
    for (int i = 0; i < PHR_SIZE; i++) 
    {
      value <<= 1;
      value |= PHR[i];
    }
    return value;
  }

  /*
  To hash into the reversal table, we need to use the following:
    - The path history register
    - The current PC
    - The prediction from the predictor
  */
  uint64_t RT_hash(uint64_t ip) 
  {
    uint64_t phr = PHR_value();
    // index into the RT table
    uint64_t hash = phr ^ ip;
    return hash % RT_SIZE;
  }

public:
  /*
  Creates a Branch Predictor Reversal Unit with the spcecified 
  Path history register size and Reversal Table size.
  */
  BPRU(uint64_t phr_size, uint64_t rt_size, uint8_t max_counter = 7) 
  {
    PHR_SIZE = phr_size;
    RT_SIZE = rt_size;
    MAX_COUNTER = max_counter;
    PHR = vector<uint8_t>(PHR_SIZE, 0);
    RT = new uint16_t*[2];
    RT[0] = new uint16_t[RT_SIZE]; // tag line
    RT[1] = new uint16_t[RT_SIZE]; // reversal counter line

    for (int i = 0; i < RT_SIZE; i++) 
    {
      RT[0][i] = 0x0; // tag
      RT[1][i] = 0x3; // counter -> set all predictions to be 100% correct
    }
  }

  ~BPRU() 
  {
    delete[] RT[0];
    delete[] RT[1];
    delete[] RT;
  }

  /*
  CALL IN "O3_CPU::predict_branch"
  Pass the prediction in here so the cofidence predictor can
  put a confidence on it.
  */
  uint64_t reverse_prediction(uint64_t ip, uint8_t pred) 
  {
    uint64_t hash = RT_hash(ip);

    // find the value of the
    uint8_t reversed = (RT[1][hash] & 0x2) >> 1;

    // taken the prediction
    if (reversed == 0x0) 
      return pred;
    else
      // reverse the prediction
      return ~pred & 0x1;
  }

  /*
  CALL IN "O3_CPU::last_branch_result"
  Pass the instruction pointer and if the branch was taken or not taken
  from the prediction.
  */
  void update_RT(uint64_t ip, uint8_t taken) 
  {
    // update the RT
    uint64_t hash = RT_hash(ip);
    if (taken)
      RT[1][hash]--;
    else 
      RT[1][hash]++;
    RT[1][hash] = clamp(RT[1][hash], MAX_COUNTER);

    // Update the PHR
    // The paper describes shifting the PHR to the left by 2 and shifting in the least 2 significant bits
    rotate(PHR.begin(), PHR.begin() + 2, PHR.end()); // shift elements down by 2 to the left
    uint8_t bit1 = ip & 0x1; // least significant bit
    uint8_t bit2 = ip & 0x2; // 2nd least significant bit
    PHR[PHR_SIZE-1] = bit1;
    PHR[PHR_SIZE-2] = bit2;
  }
};

#endif
