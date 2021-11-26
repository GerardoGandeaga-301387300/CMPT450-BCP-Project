#ifndef BPRU_H
#define BPRU_H

#include <vector>
#include <algorithm>

#define TAG 0
#define REV 1
#define IS_TAKEN(x) x == 1

/*
Architecture of the BPRU:
  - We have a Reversal Table that has a tag to reduce aliasing and a saturating counter 
    which counts up/down for correct/incorrect branch predictions. 
  - We use the most significant bit og the RT value to take or not take a branch
  - We have a HASH function that hashed into the RT with input from the Path History Register
    and PC information. 
    The RT is N associative. For this implementation we have removed the PHR and replaced it with a
    boolean to compare previous branch result outcomes.
  - Branch predictor evaluation is XORed with RT output for the final branch prediction

References:
  - https://www.ditec.um.es/~jmgarcia/papers/bpru-hipc-final.pdf
  - https://american.cs.ucdavis.edu/academic/readings/papers/brpredreversal.pdf
*/
class BPRU 
{
private:
  // reversal table
  uint16_t **RT;
  uint64_t RT_SIZE; // max size of the reversal table
  uint16_t MAX_COUNTER; // max # saturating counter can reach
  uint16_t RT_SET_ASSOCIATIVITY; // # of ways per set
  uint16_t RT_N_TAG_BITS; // # number of tag bits to use

  uint8_t COUNTER_MASK, COUNTER_SHIFT;

  // previous BPRU result
  uint8_t recent_prediction, was_reversed;

  inline uint64_t RT_tag(uint64_t hash)
  {
    uint64_t mask = (uint64_t)(1 << RT_N_TAG_BITS) - (uint64_t)1;
    return hash & mask;
  }

  /*
  To hash into the reversal table, we need to use the following:
    - The path history register
    - The current PC
    - The prediction from the predictor
  */
  uint64_t RT_hash(uint64_t ip) 
  {
    // index into the RT table
    uint64_t hash = ip;
    
    // we then use this hash has to map into a K-Associative RT table
    hash %= RT_SIZE / RT_SET_ASSOCIATIVITY; // hash will equal to a set number
    
    // iterate through the set to find the entry with our tag
    uint64_t tag = RT_tag(ip);
    uint64_t end = (hash + 1) * RT_SET_ASSOCIATIVITY; // stop once we get to the next set
    uint64_t max_i = hash; // keep track of th max value in the set
    for (uint64_t i = hash * RT_SET_ASSOCIATIVITY; i < end; i++)
    {
      // if we find our tag or the first "empty" entry in set
      if (RT[TAG][i] == tag || RT[TAG][i] == 0x0)
      {
        return i;
      }
      // keep track of highest reversal counter index
      if (RT[REV][i] > RT[REV][max_i])
      {
        max_i = i;
      }
    }

    // if the tag entry doesnt exist or no entries are empty then 
    // we will use the entry with the highest reversal count (ones that are more likely to reverse)
    return max_i;
  }

public:
  /*
  Creates a Branch Predictor Reversal Unit with the spcecified 
  Path history register size and Reversal Table size.
  
  rt_size: Size of the reversal table (use powers of 2)
  max_counter: Max value of the saturating counter (2^n-1, default = 2^3-1)
  counter_mask: Mask to get the top bit from the saturating counter (if max_counter = 7 -> some states: 111, 101, 010, 000... | counter_mask must be 4 or bits: 100)
                (if max_counter = 3 [11] then counter_mask = 2 [10])
  counter_shift: this is the number bits to shift the masked counter. this relies on the counter_mask
                 if counter_mask = 3 [100] then shift should be 2, if counter_mask = 2 [10] then shift is 1.
                 Basically, we shift left to remove the 0s.
  rt_set_assoc: Reversal table set associativity
  rt_n_tag_bits: Number of tag bits reversal table will use from the IP
  */
  BPRU(uint64_t rt_size = 1024, 
       uint8_t max_counter = 7,
       uint8_t counter_mask = 4,
       uint8_t counter_shift = 2,
       uint64_t rt_set_assoc = 8,
       uint64_t rt_n_tag_bits = 13) 
  {
    RT_SIZE = rt_size;
    MAX_COUNTER = max_counter; //94.7075%
    COUNTER_MASK = counter_mask;
    COUNTER_SHIFT = counter_shift;
    RT = new uint16_t*[2];
    RT[TAG] = new uint16_t[RT_SIZE]; // tag line
    RT[REV] = new uint16_t[RT_SIZE]; // reversal counter line

    for (int i = 0; i < RT_SIZE; i++) 
    {
      RT[TAG][i] = 0x0; // tag
      RT[REV][i] = 0x0; // counter -> set all predictions to be 100% correct
    }

    RT_SET_ASSOCIATIVITY = rt_set_assoc;
    RT_N_TAG_BITS = rt_n_tag_bits;
  }

  ~BPRU() 
  {
    delete[] RT[TAG];
    delete[] RT[REV];
    delete[] RT;
  }

  /*
  CALL IN "O3_CPU::predict_branch"
  Pass the prediction in here so the cofidence predictor can
  put a confidence on it.
  */
  uint8_t reverse_prediction(uint64_t ip, uint8_t pred) 
  {
    uint64_t hash = RT_hash(ip);
    uint64_t tag = RT_tag(ip);

    // if (hash >= RT_SIZE)
    // {
    //   cout << "hash size too large!" << hash << endl;
    // }

    if (RT[TAG][hash] != tag)
    {
      // if (RT[TAG][hash] != 0) cout << "Collision!";
      RT[TAG][hash] = tag; // set to our tag
      RT[REV][hash] = 0x0; // set to predict branch taken
    }

    // most significant bit for reversal
    uint8_t reversed = (RT[REV][hash] & COUNTER_MASK) >> COUNTER_SHIFT;

    // taken the prediction
    if (reversed == 0x0) 
    {
      recent_prediction = pred;
      was_reversed = 0;
    }
    else
    {
      // reverse the prediction
      recent_prediction = ~pred & 0x1;
      was_reversed = 1;
    }

    return recent_prediction;
  }

  /*
  CALL IN "O3_CPU::last_branch_result"
  Pass the instruction pointer and if the branch was taken or not taken
  from the prediction.
  */
  void update_RT(uint64_t ip, uint8_t taken) //53.0191
  {
    // update the RT
    uint64_t hash = RT_hash(ip);
    uint64_t tag = RT_tag(ip);

    // if (RT[TAG][hash] != tag)
    // {
    //   // cout << "Tag not set before update! " << RT[TAG][hash] << " : " << tag << endl;
    // }

    bool is_correct = IS_TAKEN(taken) == IS_TAKEN(recent_prediction);
    // if the BPRU reversed the last prediction...
    if (was_reversed)
    {
      // if the BPRU was correct, then we should keep on trusting the
      // BPRU
      if (is_correct && RT[REV][hash] < MAX_COUNTER)
      {
        RT[REV][hash]++;
      }
      else
      // inverse of the above comment, if the BPRU is incorrect, then
      // we should trust the result from the branch predictor more.
      if (!is_correct && RT[REV][hash] > 0)
      {
        RT[REV][hash]--;
      }
    }
    else
    {
      // if the prediction from the branch predictor was correct
      // then we will decrement the BPRU counter to indicate we will 
      // trust the predictor more. i.e. we dont need the BPRU.
      if (is_correct && RT[REV][hash] > 0)
      {
        RT[REV][hash]--;
      }
      else
      // the inverse of the above comment, if the branch predictor was
      // incorrect, then we will increment to start leaning towards using the BPRU.
      if (!is_correct && RT[REV][hash] < MAX_COUNTER)
      {
        RT[REV][hash]++;
      }
    }
  }
};

#endif
