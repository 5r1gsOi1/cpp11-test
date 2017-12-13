
#include <iostream>
#include <thread>
#include <atomic>

#define USE_SYNC_FLAG  0
#define USE_CPU_FENCE  0

std::atomic<bool> sync_flag(false);
std::atomic<bool> thread1_do_flag(false), 
                  thread2_do_flag(false);

std::atomic<int> x, y, r1, r2;


void thread1() {
  while (true) {
    while(!thread1_do_flag.load(std::memory_order_relaxed));
    asm volatile("mfence" ::: "memory");
    
#if USE_SYNC_FLAG
    while(!sync_flag.load(std::memory_order_relaxed)) {
      std::this_thread::yield();
    }
#endif
   
    y.store(1, std::memory_order_relaxed);
#if USE_CPU_FENCE
    asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
    asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
    r1.store(x.load(std::memory_order_relaxed), std::memory_order_relaxed);
    
#if USE_SYNC_FLAG
    sync_flag.store(false, std::memory_order_relaxed);
#endif

    asm volatile("mfence" ::: "memory");
    thread1_do_flag.store(false, std::memory_order_relaxed);
  }
}

void thread2() {
  while (true) {
    while(!thread2_do_flag.load(std::memory_order_relaxed)) {
      std::this_thread::yield();
    }
    asm volatile("mfence" ::: "memory");
        
    x.store(1, std::memory_order_relaxed);
#if USE_CPU_FENCE
    asm volatile("mfence" ::: "memory");  // Prevent CPU reordering
#else
    asm volatile("" ::: "memory");  // Prevent compiler reordering
#endif
    r2.store(y.load(std::memory_order_relaxed), std::memory_order_relaxed);
    
#if USE_SYNC_FLAG
    sync_flag.store(true, std::memory_order_relaxed);
#endif

    asm volatile("mfence" ::: "memory");
    thread2_do_flag.store(false, std::memory_order_relaxed);
  }
}

int main() {
  std::thread thr1(thread1);
  std::thread thr2(thread2);
  
  size_t count = 0, iterations = 0;
  
  while (true) {
    x.store(0, std::memory_order_relaxed);
    y.store(0, std::memory_order_relaxed);
    
    thread1_do_flag.store(true, std::memory_order_relaxed);
    thread2_do_flag.store(true, std::memory_order_relaxed);
  
    while(thread1_do_flag.load(std::memory_order_relaxed) || 
          thread2_do_flag.load(std::memory_order_relaxed)) {
      std::this_thread::yield();      
    }

    ++iterations;
    if (r1.load(std::memory_order_relaxed) == 0 && r2.load(std::memory_order_relaxed) == 0) {
      ++count;
      //std::cout << count << " for " << iterations << " iterations\n"; 
    }
    
    if (iterations > 100000) {
      std::cout << "reordering percentage = " << float(count) / float(iterations) * 100.f << "%     \r"; 
      count = iterations = 0;
    }
  }std::this_thread::yield();   

  thr1.join();
  thr2.join();
}
  
