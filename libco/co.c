#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>

static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg) : "memory"
#endif
  );
}

#define STACK_SIZE 64 * 1024
#define MAX_CO_NUM 128

enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};


struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

int cur_co_count = 0;
struct co* co_list[MAX_CO_NUM];
struct co* current_co;

void co_exit() {
  current_co->status = CO_DEAD;

  co_yield();
}

static void co_entry_wrapper(void *co) {
  struct co* self = (struct co*)co;

  self->func(self->arg);

  co_exit();
} 

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  // allocate space on heap
  struct co* co = (struct co*)malloc(sizeof(struct co));
  // init struct oc
  co->name = name;
  co->func = func;
  co->arg = arg;
  
  co->status = CO_NEW;
  co->waiter = NULL;

  co_list[cur_co_count++] = co;
  return co;
}

void co_wait(struct co *co) {
  current_co->status = CO_WAITING;

  while (co->status != CO_DEAD) {
    co_yield();
  }
  free(co);
}

void co_yield() {
  // a thread yield, we need to immediately save the register context
  int val = setjmp(current_co->context);
  if (val == 0) { /* this branch indicates that we just save the context */
    // complete register save, select next co to run
    for (int i = 0; i < MAX_CO_NUM; ++ i) {
      struct co* co = co_list[i];
      // selectable co status are NEW and RUNNING
      if (co->status == CO_NEW) {
        current_co = co;
        current_co->status = CO_RUNNING;
        stack_switch_call((void *)co->stack, co_entry_wrapper, (uintptr_t)co);
      }
      if (co->status == CO_RUNNING) {
        current_co = co;
        longjmp(co->context, 1);
      }
    }
  } else { /* this branch indicates that we decided to restore a RUNNING co's context */
    return;
  }
}

__attribute__((constructor)) void init_main() {
  struct co* main = (struct co*)malloc(sizeof(struct co));
  main->name = "main";
  main->func = NULL;
  main->arg = NULL;

  main->status = CO_RUNNING;
  main->waiter = NULL;

  co_list[cur_co_count++] = main;
  current_co = main;
}
