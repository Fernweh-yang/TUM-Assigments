# 基本知识 

- OpenMP:
  - 官网https://www.openmp.org/spec-html/5.1/openmp.html
  - 文档https://docs.oracle.com/cd/E71940_01/html/E71962/index.html
  - 一个针对共享内存并行编程的API
  - MP代表多处理
- 与Pthreads比较：
  - Pthreads:要求程序员显式的明确每个线程的行为
  - OpenMP:秩序简单的声明哪一块代码应该并行执行，由编译器和运行时系统来决定哪个线程具体执行哪个任务
    - 所以OpenMP还额外的要求，编译器支持某些操作。
- 专业术语
  - 线程组team:执行并行块的线程集合
  - 主线程master:原始线程
  - 从线程slave:额外的线程

## 一个简单的OpenMP程序

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


##  结构construct

- 需要包含头文件`omp.h`,命名空间`omp_`

```c
#pragma omp construct [clause [clause]…]
如
#pragma omp parallel num_threads(thread_count)
{
    block
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

# OpenMP编程

## Parallel section指令

- section指令用于非迭代的多线程共享区.它指定各个section代码段分配给一组线程中部分线程。多个独立的section指令嵌套在sections指令中，每个section由于其中一个线程执行一次。不同的section可以由不同的线程执行。当然对于一个线程来说，如果它运行足够快，是有可能执行多个section。

- 语法格式

  ```c
  #pragma omp sections [clause ...]  
                       private (list) 
                       firstprivate (list) 
                       lastprivate (list) 
                       reduction (operator: list) 
                       nowait
    {
  
    #pragma omp section   
  
       structured_block
  
    #pragma omp section   
  
       structured_block
  
    }
  ```

- 例子:

  ```c
  	int nLoopNum = 10;
  	int i;
  
  #pragma omp parallel shared(nLoopNum) private(i)
  	{
  		printf("thread %d start\n", omp_get_thread_num());
  	#pragma omp sections nowait
  		{
  			#pragma omp section
  			for (i=0; i < nLoopNum/2; i++)
  			{
  				printf("section1 thread %d excute i = %d\n", omp_get_thread_num(), i);
  			}
  
  			#pragma omp section
  			for (i=nLoopNum/2; i < nLoopNum; i++)
  			{
  				printf("section2 thread %d excute i = %d\n", omp_get_thread_num(), i);
  			} 
  		}
  
  	}
  ```

  

## Parallel for指令

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

### 循环调度：schedule子句

- 在OpenMP中,将循环分配给线程称为调度schedule.

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

       - 迭代能够在循环执行前分配给线程

       - 比如:由12个迭代,3个线程

         - `schedule(static,1)`

           Thread 0 : 0,3,6,9

           Thread 1 : 1,4,7,10

           Thread 2 : 2,5,8,11

         - schedule(static,4)

           Thread 0 : 0,1,2,3

           Thread 1 : 4,5,6,7

           Thread 2 : 8,9,10,11

         - 当chunksize省略时,chunk的个数是总的迭代数/线程数.这里是4.

    2. dynamic

       - Fix sized chunks (default size is 1)
       - Distributed one by one at runtime as chunks finish
       - 迭代在循环执行时被分配给线程.因此在一个线程完成了它的当前迭代集合后,它能从运行时系统中请求更多.
       - 迭代被分成chunksize个连续迭代的块.每个线程执行一块,并且每当一个线程完成一块时,它将从运行时系统请求另一块,直到所有的迭代完成.Chunksize被省略时,默认为1.

    3. guided

       - Start with large chunks, then exponentially指数的 decreasing size
       - Distributed one by one at runtime as chunks finish
       - 迭代在循环执行时被分配给线程.因此在一个线程完成了它的当前迭代集合后,它能从运行时系统中请求更多.
       - 每个线程执行一块,并且每当一个线程完成一块时,从运行时系统请求另一块.不过新块的大小会变小.块的大小近似=剩余的迭代数/线程数.
         - 不指定chunksize,默认块的大小为1.
         - 指定了chunksize,则块的大小就是chunksize,除了最后一块的大小可以比chunksize小.

    4. runtime

       - Controlled at runtime using control variable
       - 调度在运行时决定
       - 系统使用环境变量OMP_SCHEDULE在运行时来决定如何调度循环.

    5. auto

       - Compiler/Runtime can choose
       - 编译器和运行时系统决定调度方式

- 调度的选择:

  - 如果循环的每次迭代需要几乎相同的计算量,那么可能默认的调度方式能提供最好的性能
  - 如果随着循环的进行,迭代的计算量线性递增/递减,那么采用比较小的chuncksize的static调度可能会提供最好的性能
  - 如果每次迭代的开销实现不能确定,需要尝试不同的调度策略.这是硬要使用runtime,通过赋予环境变量OMP_SCHEDULE不同的值来比较不同策略下程序的性能.

## default子句

- 该子句显式的要求程序员自己明确变量的作用域

- `default(none)`要求程序员明确:

  - 这个parallel指令下的代码块中使用的每个变量的作用域
  - 在这个块之前申明的所有变量的作用域

- 例子

  ```
  	double sum =0.0
  #	pragma omp parallel for num_threads(thread_count)\
  		default(none) reduction(+:sum) private(k,factor)\
  		shared(n)
  	for(k=0;k<n;k++){
  		if(k%2==0)
  			factor =1.0;
  		else
  			factor =-1.0;	
  		sum += factor/(2*k+1);
  	}
  ```

  

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

- 基本判断：

  - 在parallel指令前已经被声明的变量，缺省情况下拥有在线程组中所有线程间的共享作用域
  - 在parallel下的块中声明的变量，则拥有私有作用域
  - parallel for指令中的循环变量的缺省作用域是私有的

- 私有数据：

  ```c
  int i=3;
  #pragma omp parallel for private(i)
  	for (int j=0; j<4; j++){ 
  		i=i+1;
  		printf("-> i=%d\n", i); 
      }
  printf("Final Value of I=%d\n", i);
  
  ```

  - 循环结束后，i还是3，因为在并行代码中,i是私有的数据。但输出的都是未知.

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
  
- Private,firstprivate和lastprivate的区别

  - private变量是没有被初始化的,在并行代码块中是随机值
  - firstprivate变量,用并行代码块前的值对其进行初始化,并一直是这个值
  - lastprivate变量没有被初始化,并行代码块执行完后的值为它最后的值.

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
- barrier指令给定一个显式的路障,线程组中所有的线程都到达了这个路障后,线程才会继续往下执行.

### Master指令

```
#pragma omp master
block
```

- A master region enforces that only the master executes the code block只有主线程可执行
  - Other threads skip the region其他线程跳过该代码块
  - No synchronization at the beginning of region

- Possible uses
  - Printing to screen
  - Keyboard entries
  - File I/O
- 本指令没有隐含的barrier,即其他线程不用在master区域结束处同步,可立即执行后续的代码.

### Single指令

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

### Critical指令

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
- 当给critical提供名字时,用不同名字的critical指令保护的代码块可以同时执行.

### Atomic指令

```
#pragma omp atomic
	expression-stmt
```

- The ATOMIC directive指令 ensures that a specific memory location is updated atomically.语句必须是如下几种形式之一

  ```c
  - x <binop>= <expr>
  - x++ or ++x
  - x-- or --x
  ```

  - where x is an lvalue左值 expression with scalar type标量
  - and expr does not reference涉及 the object designated指定 by x,不能引用x
  - binop:二元操作符`+,-,*,/,&,^,|,<<,>>`

- Equivalent等同于 to using critical section to protect the update.相比于critical,它只能保护由一条C语言赋值语句所形成的临界区

- Useful for simple/fast updates to shared data structures
  - Avoids locking
  - Often implemented directly by native instructions
  
- 注意其只对x起保护作用

  ```c
  #	pragma omp atomic
  	x += y++;
  ```

  - 其他线程对x的更新必须等到该线程对x的更新结束后,但对Y的更新不保护,所以此程序的结果是不可预测的.

### 锁lock

- critical可以用不同的名字来保护不同的代码块,他们可以同时执行.但当想让访问不同队列的线程同时访问相同的代码块时,critical无法满足.此时要用锁,显式地强制对临界区进行互斥访问.
- 锁的数据结构被执行临界区的线程所共享,这些线程中的某个锁(如主线程)会初始化锁,而当所有的线程使用完锁后,某个线程会负责销毁锁.
- 一个线程进入临界区前时,它会尝试通过调用锁函数来上锁set,
  - 如果没有其他线程正在执行临界区代码则成功上锁并进入临界区.执行完临界区代码后,它会通过解锁函数来释放relinquish/unset.
  - 如果有其他线程正在执行临界区代码,则被锁函数堵塞.
- OpenMP有两种锁
  - 简单锁simple
  - 嵌套锁nested

#### 1.Simple Runtime Locks

- In addition to pragma based options, OpenMP also offers runtime locks
  - Same concept as Pthread mutex
  - Locks can be held by only one thread at a time.
  - A lock is represented by a lock variable of `type omp_lock_t`.

- Operations

  - `omp_init_lock(&lockvar)` 			initialize a lock
  - `omp_destroy_lock(&lockvar)`       destroy a lock
  - `omp_set_lock(&lockvar)`               set lock
    - 如果成功,调用该函数的线程可以继续执行
    - 如果失败,调用该函数的线程将被阻塞,直到锁被其他线程释放
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

#### 2.Nestable Locks

- Similar to simple locks.But, nestable locks can be set multiple times by a single thread.
  - Each set operation increments a lock counter
  - Each unset operation decrements the lock counter
  - If the lock counter is 0 after an unset operation, lock can be set by another thread

### critical指令,atomic指令,锁的比较

- atomic指令是实现互斥访问最快的方法
- critical和锁,在性能上没太大区别
  - 所以在atomic不能用,critical能用时,使用critical
- 锁适用于需要互斥的是某个数据结构而不是代码块的情况.

### Ordered指令

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
- ordered指令指令的区域的循环迭代将按串行顺序执行.只能用在for或parallel for中.

```c
#pragma omp parallel
	{
	#pragma omp for ordered
		for (int i = 0; i < 10; ++i)
		{
		#pragma omp ordered
			{
				printf("thread %d excute i = %d\n",omp_get_thread_num(), i);
			}
		}
	} 

```



## 归约操作符reduction operator

- 定义一个归约变量来避免代码块的串行执行

  - 规约操作符是一个二元操作：如加法和减法。
  - 归约就是将相同的规约操作符重复的应用到操作数序列来得到一个结果的运算
  - 所有操作的中间结果存储在同一个变量里：归约变量reduction variable.

- reduction子句语法：

  `reduction(<operator>:<variable list)`

  - C语言中operator可以是：+,-,*,&,|,^,&&,||中任意一个

- 例子：

  ```c
  	global_result =0.0;
  // 指令应该只有1行，1行写不要用\来转义换行符
  #	pragma omp parallel num_threads(thread_count)\
  		reduction(+:global_result)
  	global_result += Local_trap(double a, double b, int n);
  ```

  - 代码明确了global_result是一个归约变量
  - OpenMP为每个线程有效地创建了一个私有变量，运行时系统在这个私有变量中存储每个线程的结果
  - OpenMP也创建了一个临界区，并在这个临界区将存储在私有变量中的值进行相加。

  等效于代码：

  ```c
  	global_result =0.0;
  #	pragma omp parallel num_threads(thread_count)
  	{
  		double my_result =0.0;	/*private*/
  		my_result += Local_trap(double a, double b, int n);
  #		pragma omp critical
  		global_result += my_result;
  	}
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

## 基本概念: 

- 任务构造定义一个显式的任务，可能会被遇到的线程马上执行，也可能被延迟给线程组内其他线程来执行。任务的执行，依赖于OpenMP的任务调度。

- 语法:

  ```c
  #pragma omp task [clause ...]  newline 
                     if (scalar expression) 
                     final (scalar expression) 
                     untied
                     default (shared | none)
                     mergeable
                     private (list) 
                     firstprivate (list) 
                     shared (list) 
       structured_block
  ```

- task指令指定显式任务.task指令定义了与任务及其数据环境关联的代码任务构造可以放置在程序中的任何位置，只要线程遇到任务构造，就会生成新任务。

- 当线程遇到任务构造时，可能会选择立即执行任务或延迟执行任务直到稍后某个时间再执行

  - 如果延迟执行任务，则任务会被放置在与当前并行区域关联的概念任务池中。
  - 当前组中的线程会将任务从该池中取出，并执行这些任务，直到该池为空.执行任务的线程可能与最初遇到该任务的线程不同

- 与任务构造关联的代码将只被执行一次。

  - **绑定 (tied)** 任务:代码从始至终都由相同的线程执行
  - **非绑定 (untied)** 任务:代码可由多个线程执行，使得不同的线程执行代码的不同部分
  - 缺省情况下,任务为绑定任务,可用untied子句与task指令一起将任务设为非绑定.

- 为了执行不同的任务，允许线程在任务调度点暂停执行任务区域。

  - 如果暂停的任务为绑定 (tied) 任务，则同一线程稍后会恢复执行暂停的任务
  - 如果暂停的任务为非绑定 (untied) 任务，则当前组中的任何线程都可能会恢复执行该任务
  - 任务调度点有:
    - 遇到任务构造的点
    - 遇到任务等待构造的点
    - 遇到隐式或显式屏障的点
    - 任务的完成点

- 当 **task** 构造中存在 **if** 子句，并且标量表达式的值计算为 **false** 时，遇到任务的线程必须立即执行任务。

  - **if** 子句可用于避免生成许多细粒度任务以及将这些任务放在概念池中所造成的开销。

- 数据环境

  - 在任务内对 **shared** 子句中列出的变量的所有引用是指在 **task** 指令之前一看便知的同名变量。

  - 对于未在 **task** 构造的数据属性子句中列出以及未根据 OpenMP 规则预先确定的变量的数据共享属性，将按如下所述隐式确定：

    1. 在 **task** 构造中，如果不存在 **default** 子句，则被确定为在所有封闭构造（一直到并包括最深层封闭并行构造）中共享的变量为 **shared**。

    2. 在 **task** 构造中，如果不存在 **default** 子句，则其数据共享属性不根据规则 1 确定的变量为 **firstprivate**。
       - 如果 **task** 构造在词法上包括在 **parallel** 构造中，则在所有作用域（包括 **task** 构造）中共享的变量在生成的任务中仍保持共享状态。否则，变量将被隐式确定为 **firstprivate**。
       - 如果 **task** 构造是孤立的，则变量将被隐式确定为 **firstprivate**

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

  

## taskwait,taskgroup,taskyield

- Taskwait

  ```
  #pragma omp taskwait
  { ... }
  ```

  - Waits for completion of immediate child tasks
    - Child tasks: Tasks generated since the beginning of the current task
    
  - **taskwait** 指令指定在完成自当前（隐式或显式）任务开始以来生成的子任务时进行等待

    - 当线程遇到 `taskwait` 构造时，当前任务会暂停，直到它在 `taskwait` 区域前生成的所有子任务均已完成执行为止。

  - 例子:

    ```c
    #pragma omp task shared(i) firstprivate(n)
    i=fib(n-1);
    
    #pragma omp task shared(j) firstprivate(n)
    j=fib(n-2);
    
    #pragma omp taskwait
    return i+j;
    ```

    - taskwait指令保证了2个调用fib()得到i和j值的任务在返回i+j之前已完成.

- Taskgroup

  - 当线程遇到 `taskgroup` 构造时，它将开始执行 `taskgroup` 区域。在 `taskgroup` 区域末尾，当前任务暂停，直到它在 `taskgroup` 区域中生成的所有子任务及其所有子孙任务均已完成执行为止。
  - 请注意 `taskwait` 与 `taskgroup` 之间的不同。使用 `taskwait`，当前任务仅等待其子任务。使用 `taskgroup`，当前任务不仅等待在 `taskgroup` 中生成的子任务，还等待这些子任务的所有子孙任务。
  - https://docs.oracle.com/cd/E57201_01/html/E58572/gozsi.html

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

## Task Dependencies依赖性(since OpenMP4.0)

- 该子句会对任务调度强制使用其他约束。这些约束仅在同级任务之间建立依赖关系。同级任务是指属于同一任务区域的子任务的 OpenMP 任务。
- 当使用 `depend` 子句指定 `in` 依赖类型时，生成的任务将是所有以前生成的至少引用了 `out` 或 `inout` 依赖类型列表中一个列表项目的同级任务的从属任务。
- 在在 `depend` 子句中指定了 `out` 或 `inout` 依赖类型时，生成的任务将是所有以前生成的至少引用了 `in`、`out` 或 `inout` 依赖类型列表中一个列表项目的同级任务的从属任务。

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

- flush指令主要用于处理内存一致性问题

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

