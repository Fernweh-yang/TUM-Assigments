https://www.openmp.org/spec-html/5.1/openmp.html

# 一个简单的OpenMP程序

```C
#include <omp.h>
#include <stdio.h>

int main(){
   #pragma omp parallel
    printf("Hello world\n");
    return 0;
}
```

编译时需要用到 -fopenmp选项

```
gcc -g -Wall -fopenmp -o test test.c

1.
./test 4  #用4个线程

2.也可以先定义用几个线程
export OMP_NUM_THREADS=4
./test
```

# 基本语法syntax

##  结构construct

- 需要包含头文件`omp.h`,命名空间`omp_`

```c
#pragma omp construct [clause [clause]…]
如
#pragma omp parallel [parameters]
{
    block
}
```

并行可以有好几个部分

```c
#pragma omp sections [parameters]
{
	#pragma omp section
	{ … block … }
	#pragma omp section
	{ … block … }
	...
}
```

子句`num_threads`:允许程序员指定执行后代码块的线程数

```
# pragma omp parallel num_threads(thread_count)
```

### 一些重要的runtime routines：

- `omp_set_num_threads(count)`设置并行代码段用到的线程数。只能在串行代码区域使用，修改的是环境变量OMP_NUM_THREDS

  ```
  omp_set_num_threads(3)
  ```

- `omp_get_max_threads`()获得当前能创建的线程组team的最大线程数。

  ```
  numthreads = omp_get_max_threads()
  ```

- `omp_get_thread_num()`可以得到每个线程的编号
	
	```c
	int id;
	id = omp_get_thread_num();
	```
	
- `omp_get_num_threads()`可以获得当前线程组team的线程数量。

  ```
  int c;
  c = omp_get_num_threads()
  ```

- `omp_get_num_procs()`获得处理器数

  ```
  numprocs = omp_get_num_procs()
  ```

### 相关的环境变量

- `OMP_NUM_THREADS=4`并行代码区域内线程组的线程数
- `OMP_SCHEDULE=”dynamic” ` `OMP_SCHEDULE=”GUIDED,4“` 用于指定do循环的调度方式
  - 选择在运行时的调度策略scheduling strategy
  - 调度条款在代码中有优先权
- `OMP_DYNAMIC=TRUE`允许运行时系统自己确定线程数
- `OMP_NESTED=TRUE`允许嵌套nesting并行

### 除了c/c++也可以用Fortran语言(公式翻译语言)

	```fortran
	!$OMP directive name [parameters]
	如：
	!$OMP PARALLEL DEFAULT(SHARED)
		write(*,*) 'Hello world'	
	!$OMP END PARALLEL
	```

- 不管C/C++还是Fortran，指令都是写在一行内的。如果一行不够写指令需要：

  ```
  // C/C++用 \
  #pragma omp parallel private(i) \
  	private(j)
  	
  // Fortran用 &
  !$OMP directive name first_part &
  !$OMP continuation_part
  ```


## Parallel Loop并行循环

- 语法

  ```
  #pragma omp for [parameters]
  	for...
  	
  如：
  main (){
  int a[100];
  #pragma omp parallel for
  	for (int i= 1; i<n;i++)
  		a(i) = i;
  }
  
  或者：
  main (){
  int a[100];
  #pragma omp parallel
  	{
  		#pragma omp for
  			for (int i= 1; i<n;i++)
  				a(i) = i;
  	}
  }
  ```

  - 迭代必须是独立的
    1. No data dependencies
    2. Can be executed in any order
    3. Programmer responsibility

### 循环调度：如何split up划分工作？

  - Loop Schedule安排

    1. Defines how iterations are split up
       - Leads to the creation of “Chunks”区块
       - 如，循环100次的代码，每25次为一区块，划分4个Chunks，用4个线程来执行。
    2. Defines how chunks are mapped to threads

  - OpenMP 用schedule参数来提供不同的循环平行选择

    ```
    #pragma omp for schedule(parameter)
    ```

    1. static

       - Fix sized chunks (default size is about n/t)
       - Distributed in a round-robin fashion轮叫的形式

    2. dynamic

       - Fix sized chunks (default size is 1)
       - Distributed one by one at runtime as chunks finish

    3. guided

       - Start with large chunks, then exponentially指数的 decreasing size
       - Distributed one by one at runtime as chunks finish

    4. runtime

       - Controlled at runtime using control variable

    5. auto

       - Compiler/Runtime can choose


## 私有数据/共享数据

- 区别：

  The default for global variables: shared

  - Shared data
    - Accessible by all threads
    -  One copy for all threads
    -  Example: a reference a[5] to a shared array accesses

  - Private data
    - Accessible only by a single thread
    -  Each thread has its own copy
    -  Example: iterator variables

- 私有数据：

  ```c
  int i=3;
  #pragma omp parallel for private(i)
  	for (int j=0; j<4; j++)
  	{ 
  		i=i+1;
  		printf("-> i=%d\n", i); }
  		printf("Final Value of I=%d\n", i);
  	}	
  ```

  - 循环结束后，i还是3，因为在并行代码中,i是私有的数据。

  - 可以选First/Last Private Data

    - firstprivate: i还是3，因为第一个量就被定为私有的

    ```c
    int i=3;
    #pragma omp parallel for firstprivate(i)
    	for (int j=0; j<4; j++)
    	{	
    		i=i+1;
    		printf("-> i=%d\n", i); }
    		printf("Final Value of I=%d\n", i);
    	}
    ```

    - lastprivate: i=7，最后一个量被定位私有的

    ```c
    int i=3;
    #pragma omp parallel for lastprivate(i)
    	for (int j=0; j<4; j++)
    	{ 
    		i=i+1;
    		printf("-> i=%d\n", i); }
    		printf("Final Value of I=%d\n", i);
    	}
    ```

- 变量的sharing Attributes 分享属性

  - private(var-list)
    - Variables in var-list are private
  - shared(var-list)
    - Variables in var-list are shared
  - default(private | shared | none)
    - Sets the default for all variables in this region
  - firstprivate(var-list)
    - Initialized with the value of the shared copy before the region.
  - lastprivate(var-list)
    - Value of the thread executing the last iteration of a parallel loop in sequential order is copied to the variable outside of the region.

## 同步数据

### Barrier

- Key synchronization construct

  - Synchronizes all the threads in a team
  - Each thread waits until all other threads in the team have reached the barrier

- Each parallel region has an implicit barrier隐式障碍 at the end

  - Synchronizes the end of the region
  - Can be switched off by adding nowait

- Additional barriers can be added when needed

  - `pragma omp barrier`
- Warning
  - Can cause load imbalance负载失衡
  - Use only when really needed

### Master Region

```
#pragma omp master
block
```

- A master region enforces that only the master executes the code block
  - Other threads skip the region
  - No synchronization at the beginning of region

- Possible uses
  - Printing to screen
  - Keyboard entries
  - File I/O

### Single Region

```
#pragma omp single [parameter]
block
```

- A single region enforces that only a (arbitrary) single thread executes the code block
  - Other threads skip the region
  - Implicit barrier synchronization at the end of region
    (unless nowait is specified)
- Possible uses
  - Initialization of data structures

### Critical Section

```
#pragma omp critical [(Name)]
block
```

- Mutual exclusion
  - A critical section is a block of code can only be executed by only one thread at a time.
  - Compare to Pthreads locks
- Critical section name identifies确认 the specific critical section
  - A thread waits at the beginning of a critical section until its available
  - All unnamed critical directives map to the same name
- Keep in mind
  - Critical section names are global entities实体 of the program
  - If a name conflicts with any other entity, program behavior is unspecified不明确的
  - Avoid long critical sections for performance reasons

### Atomic Statements

```
#pragma omp atomic
	expression-stmt
```

- The ATOMIC directive指令 ensures that a specific memory location is updated atomically

  ```c
  - x binop= expr
  - x++ or ++x
  - x-- or --x
  ```

  - where x is an lvalue左值 expression with scalar type标量
  - and expr does not reference涉及 the object designated指定 by x,不能引用x
  - binop:二元操作符`+,-,*,/,&,^,|,<<,>>`

- Equivalent等同于 to using critical section to protect the update
- Useful for simple/fast updates to shared data structures
  - Avoids locking
  - Often implemented directly by native instructions

### Simple Runtime Locks

- In addition to pragma based options, OpenMP also offers runtime locks
  - Same concept as Pthread mutex
  - Locks can be held by only one thread at a time.
  - A lock is represented by a lock variable of `type omp_lock_t`.

- Operations

  - `omp_init_lock(&lockvar)` 			initialize a lock
  - `omp_destroy_lock(&lockvar)`       destroy a lock
  - `omp_set_lock(&lockvar)`               set lock
  - `omp_unset_lock(&lockvar)`           free lock
  - `logicalvar = omp_test_lock(&lockvar)`  check lock and possibly set lock
    returns true if lock was set by the executing thread.

- Example

  ```c
  #include <omp.h>
  int id;
  omp_lock_t lock;
  omp_init_lock(lock);
  
  #pragma omp parallel shared(lock) private(id)
  {	
      /*locked*/
  	id = omp_get_thread_num();
  	omp_set_lock(&lock); //Only a single thread writes
  		printf(“My Thread num is: %d”, id);
  	omp_unset_lock(&lock);
  	
      /*locked*/
      while (!omp_test_lock(&lock))
  		other_work(id); //Lock not obtained
  	real_work(id); //Lock obtained
  	omp_unset_lock(&lock); //Lock freed
  }
  omp_destroy_lock(&lock);
  ```

### Nestable Locks

- Similar to simple locks.But, nestable locks can be set multiple times by a single thread.
  - Each set operation increments a lock counter
  - Each unset operation decrements the lock counter
  - If the lock counter is 0 after an unset operation, lock can be set by another thread

### Ordered Construct

```
#pragma omp for ordered
	for (...)
		{ ...
			#pragma omp ordered
				{ ... }
			...
		}
```

- Construct must be within the dynamic extent of an omp for construct with an
  ordered clause.
- Ordered constructs are executed strictly严格的 in the order in which they would be
  executed in a sequential execution of the loop.保证这段代码一定是按照for中的顺序执行

# 缩小reductions

```
reduction(operator: list)
```

- This clause performs a reduction

  - on the variables that appear in list with the operator operator算子.
  - across the values “at thread end“/“last iteration“ of each thread (!)

- Variables must be shared scalars operator is one of the following:

  ```
  +, *, -, &, ˆ, |, &&, ||
  ```

- Reduction variable can only appear in statements with the following form:

  ```
  - x = x operator expr
  - x binop= expr
  - x++, ++x, x--, --x
  ```

  User defined reductions are an option in newer versions of OpenMP

- example

  ```c
  int a=0;
  #pragma omp parallel for reduction(+: a)
  for (int j=0; j<100; j++)
  {
  	a = j;
  }
  printf("Final Value of a=%d\n", a);
  /*
  OMP_NUM_THREADS=4
  
  Final value of a:
  24+49+74+99 = 246
  
  Thread 0: 0-24 last value is 24
  Thread 1: 25-49 last value is 49
  Thread 2: 50-74 last value is 74
  Thread 3: 75-99 last value is 99
  ```

  

# OpenMP Task

## 结构Task Constuct

- Explicit creation of tasks

  ```c
  #pragma omp parallel
  {
  	#pragma omp single {
  	for ( elem = l->first; elem; elem = elem->next)
  		#pragma omp task
  		process(elem)
  }
  // all tasks are complete by this point
  }
  ```

  - Task scheduling
    - Tasks can be executed by any thread in the team
  - Barrier
    - All tasks created in the parallel region have to be finished.

- Tasking Syntax in OpenMP

  ```
  #pragma omp task [clause list]
  { ... }
  ```

  - Select clauses

    `if (scalar-expression)`

    - FALSE: Execution starts immediately by the creating thread
    - The suspended task may not be resumed until the new task is finished.

    `untied`

    - Task is not tied to the thread starting its execution.
    - It might be rescheduled to another thread.

    `Default (shared|none), private, firstprivate, shared`

    - Default is firstprivate.

    `priority(value)`

    - Hint to influence order of execution
    - Must not be used to rely on task ordering

- Example

  ```c
  struct node {
  	struct node *left;
  	struct node *right;
  };
  void traverse( struct node *p ) {
  	if (p->left)
  		#pragma omp task // p is firstprivate by default
  		traverse(p->left);
  		if (p->right)
  			#pragma omp task // p is firstprivate by default
  			traverse(p->right);
  			process(p);
  }
  #pragma omp parallel
  #pragma omp single    //single区域的代码只能由线程组中的一个线程来执行
  traverse(root);
  ```

  

## Task Wait and Task Yield

- Task Wait

  ```
  #pragma omp taskwait
  { ... }
  ```

  - Waits for completion of immediate child tasks
    - Child tasks: Tasks generated since the beginning of the current task

- Task Yield

  ```
  #pragma omp taskyield
  { ... }
  ```

  - The taskyield construct specifies that the current task can be suspended
    - Explicit task scheduling point

- Implicit task scheduling points

  - Task creation
  -  End of a task
  - Taskwait
  - Barrier synchronization

## Task Dependencies(since OpenMP4.0)

- Defines in/out dependencies between tasks

  - Out: variables produced by this task
  - In: variables consumed by this task
  - Inout: variables is both in and out
  - Influences scheduling order

- Implemented as clause for task construct

  ```
  #pragma omp task depend(dependency-type: list)
  { ... }
  ```

- Example:

  ```
  #pragma omp task shared(x, ...) depend(out: x) // T1
  	preprocess_some_data(...);
  #pragma omp task shared(x, ...) depend(in: x) // T2
  	do_something_with_data(...);
  #pragma omp task shared(x, ...) depend(in: x) // T3
  	do_something_independent_with_data(...);
  ```

  

## Flush Directive指令

`#pragma omp flush [(list)]`

**强制刷新每个线程的临时视图，使其和内存视图保持一致，即：使线程中缓存的变量值和内存中的变量值保持一致**。

Thread1:

```
a = foo();
#pragma omp flush(a,flag)
flag = 1;
#pragma omp flush(flag)
```

Thread2:

```
while (flag)
{
#pragma omp flush(flag)
}
#pragma omp flush(a,flag)
b = a;
```

