#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_sigalarm(void)
{
  int ticks;
  uint64 handler;
  // Get first argument
  argint(0, &ticks);
  if (ticks < 0)
    return -1;
  // Get second argument
  argaddr(1, &handler);
  if (handler < 0)
    return -1;
  // Save the arguments into proc struct
  struct proc *p = myproc();
  p->interval = ticks;
  p->handler = handler;

  return 0;
}

/* Return the previous execution properly */
uint64 sys_sigreturn(void)
{
  struct proc *p = myproc();

  // Recover from handler function
  p->trapframe->ra = p->ra;
  p->trapframe->sp = p->sp;
  p->trapframe->gp = p->gp;
  p->trapframe->tp = p->tp;
  p->trapframe->t0 = p->t0;
  p->trapframe->t1 = p->t1;
  p->trapframe->t2 = p->t2;
  p->trapframe->s0 = p->s0;
  p->trapframe->s1 = p->s1;
  p->trapframe->a0 = p->a0;
  p->trapframe->a1 = p->a1;
  p->trapframe->a2 = p->a2;
  p->trapframe->a3 = p->a3;
  p->trapframe->a4 = p->a4;
  p->trapframe->a5 = p->a5;
  p->trapframe->a6 = p->a6;
  p->trapframe->a7 = p->a7;
  p->trapframe->s2 = p->s2;
  p->trapframe->s3 = p->s3;
  p->trapframe->s4 = p->s4;
  p->trapframe->s5 = p->s5;
  p->trapframe->s6 = p->s6;
  p->trapframe->s7 = p->s7;
  p->trapframe->s8 = p->s8;
  p->trapframe->s9 = p->s9;
  p->trapframe->s10 = p->s10;
  p->trapframe->s11 = p->s11;
  p->trapframe->t3 = p->t3;
  p->trapframe->t4 = p->t4;
  p->trapframe->t5 = p->t5;
  p->trapframe->t6 = p->t6;
  p->trapframe->epc = p->epc;

  // Set handler_on state to false
  p->handler_on = 0;

  return p->trapframe->a0;
}