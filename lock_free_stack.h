
#ifndef LOCK_FREE_STACK
#define LOCK_FREE_STACK

#include <memory>
#include <atomic>

namespace parallel {

// From "Concurrency in action" by A. Williams
template <class T>
class LockFreeStack {
public:
  LockFreeStack() {
    head.store(CountedNodePtr(), std::memory_order_relaxed);
  }

  ~LockFreeStack() {
    while (pop());
  }
  
  void push(const T& data) {
    CountedNodePtr new_node;
    new_node.ptr = new Node(data);
    new_node.external_count = 1;
    new_node.ptr->next = head.load(std::memory_order_relaxed);
    while (!head.compare_exchange_weak(new_node.ptr->next, new_node,
                                       std::memory_order_release,
                                       std::memory_order_relaxed));
  }
  
  std::shared_ptr<T> pop() {
    CountedNodePtr old_head = head.load(std::memory_order_relaxed);
    while (true) {
      IncreaseHeadCount(old_head);
      Node * const ptr = old_head.ptr;
      if (!ptr) {
        return std::shared_ptr<T>();
      }
      if (head.compare_exchange_strong(old_head, ptr->next,
                                       std::memory_order_relaxed)) {
        std::shared_ptr<T> result;
        result.swap(ptr->data);
        const int count_increase = old_head.external_count - 2;
        if (-count_increase == ptr->internal_count.fetch_add(
              count_increase, std::memory_order_release)) {
            delete ptr;
        }
        return result;
      } else if (1 == ptr->internal_count.fetch_add(
                      -1, std::memory_order_relaxed)) {
        ptr->internal_count.load(std::memory_order_acquire);
        delete ptr;
      }
    }
  }
  

private:
  struct Node;
  
  struct CountedNodePtr {
    int external_count;
    Node * ptr;
    CountedNodePtr() noexcept : external_count(0), ptr(nullptr) {}
  };
  
  struct Node {
    std::shared_ptr<T> data;
    std::atomic<int> internal_count;
    CountedNodePtr next;
    Node(const T& data_) : 
      data(std::make_shared<T>(data_)), internal_count(0), next() {}
  };

  std::atomic<CountedNodePtr> head;
  
  void IncreaseHeadCount(CountedNodePtr& old_counter) {
    CountedNodePtr new_counter;
    do {
      new_counter = old_counter;
      ++new_counter.external_count;
    } while (!head.compare_exchange_strong(old_counter, new_counter, 
                                           std::memory_order_acquire,
                                           std::memory_order_relaxed));
    old_counter.external_count = new_counter.external_count;
  }
};
}

#endif


