
// listing 8.11 from 'Concurrency in action' by A. Williams

#include <vector>
#include <thread>
#include <atomic>
#include <iostream>
#include <numeric>

struct Barrier {
  std::atomic <unsigned int> count;
  std::atomic <unsigned int> spaces;
  std::atomic <unsigned int> generation;
  
  Barrier(unsigned int count_) : 
    count(count_), spaces(count_), generation(0) {}
  
  void wait() {
    const unsigned int local_generation = generation;
    
    if (!--spaces) {
      spaces = count.load();
      ++generation;
      
    } else {
      while (generation.load() == local_generation) {
        std::this_thread::yield();
      }
    }
  }
  
  void DoneWaiting() {
    --count;
    if (!--spaces) {
      spaces = count.load();
      ++generation;
    }
  }
};

template <class Iterator>
void ParallelPartialSum(Iterator first, Iterator last) {
  typedef typename Iterator::value_type ValueType;
  
  struct ProcessElement {
    void operator()(Iterator first, Iterator last, 
                    std::vector<ValueType>& buffer, 
                    unsigned int i, Barrier& barrier) {
      ValueType& ith_element = *(first + i);
      bool is_updating_source = false;
      if (i == 0) {
        buffer[i] = ith_element;
      }
      for (unsigned int step = 0, stride = 1; stride <= i; 
           ++step, stride *= 2) {
        const ValueType& source = (step % 2) ? buffer[i] : ith_element;
        ValueType& dest = (step % 2) ? ith_element : buffer[i];
        const ValueType addend = (step % 2) ? 
          buffer[i - stride] : *(first + i - stride);
        dest = source + addend;
        is_updating_source = !(step % 2);
        barrier.wait();
      }
      if (is_updating_source) {
        ith_element = buffer[i];
      }
      barrier.DoneWaiting();
    }
  };
    
  const unsigned long length = std::distance(first, last);
  
  if (length <= 1) {
    return;
  }
  
  std::vector<ValueType> buffer(length);
  Barrier barrier(length);
  
  std::vector<std::thread> threads(length - 1);
  
  Iterator block_start = first;
  for (unsigned long i = 0; i < (length - 1); ++i) {
    threads[i] = std::thread(ProcessElement(), first, last, 
                             std::ref(buffer), i, std::ref(barrier));
  }
  ProcessElement()(first, last, buffer, length - 1, barrier);
  for (auto& e: threads) {
    e.join();
  }
}

int main() {
  std::vector<int> v1, v2;
  for (int i = 0; i < 100; ++i) {
    v1.push_back(i);
    v2.push_back(i);
  }
  
  v1[0] = -100;
    
  ParallelPartialSum(v1.begin(), v1.end());
  std::partial_sum(v2.begin(), v2.end(), v2.begin());

  std::cout << std::boolalpha
            << std::equal(v1.begin(), v1.end(), v2.begin())
            << std::endl;

  return 0;
}

