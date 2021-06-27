# 预备知识

- MPI: Message-Passing Interface消息传递
  - MPI定义了一个可以被C、C++、Fortran程序调用的函数库。用于分布式内存系统的编程。
  - 在消息传递程序中，运行在一个 核+内存 对上的程序称之为一个进程。两个进程可以通过调用函数来进行通信：
    - 一个进程调用发送函数
    - 一个进程接收函数

## 一个简单的程序

```c
#include <stdio.h>
#include <string.h>
#include <mpi.h>

const int MAX_STRING = 100;

int main(void){
    char greeting[MAX_STRING];
    int comm_sz;    /* Number of process */
    int my_rank;    /* My process rank   */

    MPI_Init(NULL,NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
	
	/*
	if-else语句使程序使SPMD(Single Program, Multiple Data)的
	编写单个程序，让不同进程产生不同的动作。
	*/
	//0号进程外，每个进程产生一条发给0号进程的消息
    if(my_rank !=0){
        sprintf(greeting,"Greetings from process %d of %d",
            my_rank, comm_sz);
        MPI_Send(greeting,strlen(greeting)+1, MPI_CHAR,0,0,
            MPI_COMM_WORLD);
    }
    //0号进程：
    else{
        printf("Greetings from process %d of %d\n",my_rank,
            comm_sz);
        //接受其他进程发送来的消息并打印
        for(int q=1; q<comm_sz; q++){
            MPI_Recv(greeting, MAX_STRING, MPI_CHAR,q,
                0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%s\n",greeting);
        }
    }

    MPI_Finalize();
    return 0;
}

```

- 命名：

  - MPI定义的标识符都由字符串MPI_开始。
  - 下划线后第一个字母大写，表示函数名和MPI定义的类型
  - MPI定义的宏和常量的所有字母都是大写的，以此区分用户定义的常量和宏。

- **初始化与释放**

  `MPI_Init(int* argc_p,char*** argc_p)`: 进行必要的初始化设置

  - 初始化可能需要为消息缓冲区分配存储空间
  - 为进程指定进程号等
  - 在用户启动程序时，定义由用户启动的所有进程所组成的通信子MPI_COMM_WORLD
  - 两个参数是指向argc和argv的指针。不需要时设置为NULL

  `MPI_Finalize(void)`告知系统MPI已经使用完毕

  - 释放为MPI而分配的所有资源

- **通信子**：

  - 通信子communicator指一组可以互相发送信息的进程集合。

  `MPI_Comm_size(MPI_Comm comm, int* comm_sz_p)`

  - 第一个参数时一个通信子，类型为MPI_Comm
  - 第二个参数返回通信子的进程数
  - 在MPI_COMM_WORLD中常用comm_sz表示进程的数量

  `MPI_Comm_rank(MPI_Comm comm, int* my_rank_p)`

  - 第二个参数返回正在调用进程在通信子中的进程号
  - 在MPI_COMM_WORLD中常用my_rank来表示进程号

- **发送消息**

  ```c
  int MPI_Send(
  	void* 			msg_buf_p,
  	int				msg_size,
  	MPI_Datatype	msg_type,
  	int				dest,
  	int				tag,
  	MPI_Comm		communicator
  );
  ```

  - 第一个参数msg_buf_p：一个指向包含消息内容的内存块的指针
  - 第二个参数msg_size：指定发送的数据量的大小
  - 第三个参数msg_type：指定数据的类型
    - 由于C中的类型int、char等不能作为参数传递给函数，所以定义了一个特殊类型MPI_Datatype，用于参数msg_type。（使用时查表）
  - 第四个参数dest：制定了要接收消息的进程的进程号
  - 第五个参数tag：是个非负int型，用于区分看上去完全一样的信息
    - 比如tag为0的消息要打印，为1的用于计算
  - 第六个参数communicator：通信子，作用是指定了通信的范围。

- **接收消息**

  ```c
  int MPI_Recv(
  	void*			msg_buf_p,
  	int 			buf_size,
  	MPI_Datatype	buf_type,
  	int				source,
  	int				tag,
  	MPI_Comm		communicator,
  	MPI_Status		status_p
  );
  ```

  - 第四个参数source：指定了接收的消息应该从哪个进程发送而来
    - 如果0号进程知识简单的按顺序接收其他进程的结果，那么可能最先完成的commsz-1号就要等待其他所有进程的完成。为避免此问题，可赋值**MPI_ANY_SOURCE**:可以按照完成工作的顺序来接收结果了。
    - 注意：只有接收者可以使用通配符wildcard。
  - 第五个参数tag：要与发送消息的tag相匹配。
    - 类似上面，接收另一进程的不同标签的消息，不知道哪个消息先来。如果不在意顺序，那么为了避免等待，可以赋值**MPI_ANY_TAG**，
    - 注意：只有接收者可以使用通配符。
  - 第六个参数communicator：必须与发送进程所用的通信子相匹配
  - 第七个参数status_p：通常用不到，赋予MPI_STATUS_IGNORE就行
    - 有至少3个成员结构：MPI_SOURCE, MPI_TAG和MPI_ERROR
    - MPI_SOURCE, MPI_TAG用于确定发送者和标签。
      - 如此，使用通配符时，也能知道发送者和标签了。
      - `MPI_Get_count(&status,recv_type, &count)`得到这些值
  
- **消息匹配**

  - 规则：
    - 如果recv_type = send_type,同时recv_buf_sz >= send_buf_sz，那么由source号进程发送的消息可以被r号进程成功接收。

## 编译/运行

- 用命令行来编译和运行程序

  编译：

  ```
  $ mpicc -g -Wall -o mpi_hello mpi_hello.c
  ```

  - mpicc 是C语言编译器的包装脚本wrapper script
  - 包装脚本的主要目的是运行某个程序。这里程序是编译器
  - 通过告知编译器从何处取得需要的头文件、库函数等，包装脚本可以简化编译器的运行

  运行：

  ```
  $ mpiexec -n <number of process> ./mpi_hello
  ```

## 潜在的陷阱

- MPI_Send
  - 存在一个默认的消息截止大小(cutoff message size)
  - 如果一条消息的大小<截止大小，将被缓冲
  - 如果一条消息的大小>截止大小，将被阻塞
- MPI_Recv
  - 一直是阻塞的，直到接收到一条匹配的消息。
- 进程悬挂：
  - 进程永远阻塞在那里。
  - 当一个进程试图接收消息时，但没有相匹配的消息，就会发生
  - 比如tag不匹配，进程号不匹配等等。

# 集合通信

- 全局求和
  - 有8个线程，1-7号算出的结果都交给0号来相加，会很浪费时间

## 树形结构通信

- 0-1；2-3；4-5；6-7两两的结果先相加
  - 然后再0-1—2-3；4-5—6-7的结果相加
  - 如此类推。不把所有求和的工作交给一个进程，从而节省了时间。

### MPI_Reduce

- 集合通信collective communication

  - 在MPI里涉及通信子中所有进程的通信函数。

- 点对点通信point-to-point communication

  - 想MPI_Send和MPI_Recv这样的函数

- 用单个函数实现集合通信可能的运算：求和，最小最大值。。。

  ```c
  int MPI_Reduce(
  	void* 			input_data_p,
  	void*			output_data_p,
  	int				count,
  	MPI_Datatype	datatype,
  	MPI_Op			operator,
  	int				dest_process,
  	MPI_Comm		comm
  );
  ```

  - 用第五个参数operator实现这些运算。

    | 运算符值 | 含义     | 运算符值   | 含义             |
    | -------- | -------- | ---------- | ---------------- |
    | MPI_MAX  | 求最大值 | MPI_LOR    | 逻辑或           |
    | MPI_MIN  | 求最小值 | MPI_BOR    | 按位或           |
    | MPI_SUM  | 求累加和 | MPI_LXOR   | 逻辑异或         |
    | MPI_PROD | 求累乘积 | MPI_BXOR   | 按位异或         |
    | MPI_LAND | 逻辑与   | MPI_MAXLOC | 求最大值及其位置 |
    | MPI_BAND | 按位与   | MPI_MINLOC | 求最小值及其位置 |

    例子：n维向量的加法

    ```c
    double local_x[N],sum[N];
    ...
    MPI_Reduce(local_x,sum,N,MPI_DOUBLE,MPI_SUM,0,
    	MPI_COMM_WORLD);
    ```

- 集合通信与点对点通信

  - 在通信子中所有进程都必须调用相同的集合通信函数。
    - 一个进程的MPI_Reduce调用和另一个进程的MPI_Recv调用匹配时会出错
  - 每个进程传递给MPI集合通信函数的参数必须时相容的。
    - 如一个进程将0作为dest_process的值传递给函数，另一个进程传递1，那么对MPI_Reduce调用所产生的结果是错误的
  - 参数ouput_data_p只用在dest_process上。然而，所有进程仍需要传递一个与ouput_data_p相对应的实际参数，即使值维NULL
  - 点对点通信函数是通过标签和通信子来匹配。集合通信函数不使用标签，只通过通信子和调用的顺序来进行匹配。

## 蝶形结构:MPI_Allreduce

- 当得到全局总和的结果后，可能所有进程都想得到总和的结果。

- 为此提供了一个函数

  ```
  int MPI_Allreduce(
  	void*			input_data_p,
  	void*			output_data_p,
  	int 			count,
  	MPI_Datatype	datatype,
  	MPI_Op			operator,
  	MPI_Comm		Comm
  );
  ```



## 广播broadcas: MPI_Bcast

- 广播：一个进程的数据被发送到通信子中的所有进程，这样的集合通信叫做广播。

  ```
  int MPI_Bcast(
  	void*			data_p,
  	int				count,
  	MPI_Datatype	datatype,
  	int				source_proc,
  	MPI_Comm		comm
  );
  ```

  - 进程号维source_proc的进程将data_p所引用的内存内容发送给了通信子comm中所有进程。

## 数据分发：

- 编写一个程序计算两个向量x+y相加。

  - x+y=(x1+x2....xn)+(y1+y2+...yn)

- 串行实现：

  ```c
  void vector_sum(double x[], double y[], double z[], int n){
  	int i;
  	for(i=0;i<n;i++){
  		z[i] = x[i] + y[i];
  	}
  }
  ```

- 并行：此时各任务间不需要通信，只需将各任务划分到核上

  - 假设有12个分量，3个进程

  - 块划分

    - 12/3=4个。所以前4个分量给第一进程，中间4个给第二进程。。。。

  - 循环划分

    - 0号进程-0号分量，1号进程-1号分量，2号进程-2号分量

      0-3，1-4，2-5.。。。。。

    - 以轮转的方式去分配向量分量

  - 块-循环划分

    - 比如以2个分量为1块
    - 0号进程-0，1号分量；1号进程-2，3号分量；2号进程-4，5号分量。。。
    - 如此轮转下去

  - 并行实现：每个进程值由local_n个分量

  ```c
  void Parallel_vector_sum(
  	double		local_x[],
  	double		local_y[],
  	double		local_z[],
  	int			local_n
  ){
  	int local_i;
  	
  	for(local_i = 0; local_i < local_n; local_i){
  		local_z[local_i] = local_x[local_i] + local_y[local_i];
  	}
  }
  ```

## 散射MPI_Scatter

- 0号进程读入整个向量，但只将分量发送给需要分量的其他进程

  ```c
  int MPI_Scatter(
  	void*			send_buf_p,
  	int				send_count,
  	MPI_Datatype	send_type,
  	void*			recv_buf_p,
  	int				recv_count,
  	MPI_Datatype	recv_type,
  	int				src_proc,
  	MPI_Comm		comm
  );
  ```

  - 还是向量相加问题：
  - 如果通信子comm包含comm_sz个进程，那么函数会将send_buf_p所引用的数据分成comm_sz份，第一份local_n = n/comm_sz个分量给0号进程，第二份给1号，第三份给3号。。。以此类推。
  - recv_buf_p的值：每个进程本地的向量
  - recv_count：local_n的值
  - send_type,recv_type：MPI_DOUBLE
  - src_proc:0
  - send_count:local_n
    - 因为它表示的是发送到每个进程的数据量，而不是send_buf_p所引用的内存的数据量。

## 聚集MPI_Gather

- 将向量的所有分量都收集到0号进程上

  ```c
  int MPI_Gather(
  	void* 			send_buf_p,
  	int				send_count,
  	MPI_Datatype	send_type,
  	void*			recv_buf_p,
  	int				recv_count,
  	MPI_Datatype	recv_type,
  	int				dest_proc,
  	MPI_Comm		comm
  );
  ```

  - 在0号进程中，由send_buf_p所引用的内存区的数据存储在recv_buf_p的第一个块中；
  - 在1号进程中，由send_buf_p所引用的内存区的数据存储在recv_buf_p的第二个块里。。以此类推。

## 全局聚集MPI_Allgather

- 蝶形的结构

  ```c
  int MPI_Allgather(
  	void*			send_buf_p,
  	int				send_count,
  	MPI_Datatype	send_type,
  	void*			recv_buf_p,
  	int				recv_count,
  	MPI_Datatype	recv_type,
  	MPI_Comm		comm
  );
  ```

  - 这个函数将每个进程的send_buf_p的内容串联起来，存储到每个进程的recv_buf_p参数中。
  - 通常recv_count值每个进程接收的数据量，所以通常与send_count值相同。

  

# 派生数据类型MPI_Type_create_struct

- 通信比本地计算开销大得多。所以，要尽可能的减少发送的消息数量

- MPI提供三种手段，来整合多条消息的数据
  - 不同通信函数中的count参数
    - 用于将连续的数组元素，聚集起来组成一条单独的消息
  - 派生数据类型
    - 通过同时存储数据项的类型以及他们在内存中的相对位置，派生数据类型可以用于表示内存中数据项的任意集合。
    - 如果发送数据的函数知道数据项的类型以及在内存中数据项集合的相对位置，就可以在数据项被发送出去之前在内存中的数据项聚集起来。
    - 接收数据的函数也可以在数据项被接收后将数据项分发到他们在内存中正确的目标地址上。
  - MPI_Pack/Unpack函数

- 一个派生数据类型组成

  - 一系列MPI基本数据类型

  - 每个数据类型的偏移

  - 例子如：

    `{(MPI_DOUBLE,0),(MPI_DOUBLE,16)(MPI_DOUBLE,24)}`

    - 分别是a,b,n变量。a地址24，b40，c48
    - 第二个元素是该数据项相对于起始位置(这里是a的24)的偏移。

- 用MPI_Type_create_struct函数创建不同的派生数据类型

  ```
  int MPI_Type_create_struct(
  	int				count,
  	int				array_of_blocklengths[],
  	MPI_Aint		array_of_displacements[],
  	MPI_Datatype	array_of_types[],
  	MPI_Datatype*	new_type_p	
  );
  ```

  - count指的是数据类型中元素的个数
    - 上面例子中，count就是3
    
  - array_of_blocklengths[]：允许单独的数据项是一个数组或子数组
  
    - 如果第一个元素是一个含5个元素的数组则：
  
      `array_of_blocklengths[0]=5`
  
    - 例子中没有元素是数组所以：
  
      `array_of_blocklengths[3] = {1,1,1};
  
  - array_of_displacements[]: 制定了距离消息起始位置的偏移量
  
    - 例子中
  
      `array_of_displacements[]={0,16,24}`
  
    - 可以通过MPI_Get_address来得到这里的地址
  
      ```c
      int MPI_Get_adress(
      	void*		location_p,
      	MPI_Aint*	address_p
      );
      ```
  
      - location_p:指向内存单元的地址
  
      - MPI_Aint：整型，长度足以表示系统的地址
  
      - 例子：
  
        ```c
        MPI_Aint a_addr,b_addr,n_addr;
        
        MPI_Get_address(&a, &a_addr);
        array_of_displacements[0] = 0;
        MPI_Get_address(&b, &b_addr);
        array_of_displacements[1] = b_addr - a_addr;
        MPI_Get_address(&n, &n_addr);
        array_of_displacements[2] = n_addr - a_addr;
        ```
  
  - array_of_datatypes[]存储元素的MPI数据类型
  
    `array_of_datatypes[3] = {MPI_DOUBLE, MPI_DOUBLE,MPI_INT}`
  
  - 初始化以上参数后，就可以建立新的数据类型了
  
    ```c
    MPI_Datatype input_mpi_t;
    ..
    MPI_Type_create_struct(3,array_of_blocklengths,
    	array_of_displacements,array_of_types,
    	&input_mpi_t);
    ```
  
    - 使用派生数据类型input_mpi_t前需要先用一个函数调用去指定他：
  
      `int MPI_Type_commit(MPI_Datatype* new_mpi_t_p);`
  
      它允许MPI实现为了在通信函数内使用这一数据类型，优化数据类型的内部表示。
  
    - 为了使用这个新类型，需要在每个进程上调用MPI_Bcast
  
      `MPI_Bcast(&a,1,input_mpi_t,0,comm);`
  
      此时我们可以像使用MPI的基本类型一样去使用input_mpi_t类型
  
  - 使用新数据类型时，MPI可能会分配额外的存储空间，如果要释放额外的存储空间：
  
    `int MPI_Type_free(MPI_Datatype* old_mpi_t_p);`

## MPI_Type_commit

- 这个routine是thread-safe的

  `int MPI_Type_commit(MPI_Datatype* datatype)`

  - 这个routine可以安全的被多个threads调用，不需要任何用户提供的thread locks

# MPI程序性能评估

## 计时MPI_Wtime

- 返回从过去某一时刻开始所经过的秒数

  ```
  double MPI_Wtime(void);
  ```

- 对一块MPI代码进行计时

  ```c
  double start,finish;
  ...
  start = MPI_Wtime();
  /*.MPI代码.*/
  finish = MPI_Wtime();
  printf("Proc %d > Elapsed time = %e seconds",my_rank,finish-start);
  ```

- 对一块串行代码进行计时

  ```c
  #include "timer.h"
  ...
  double start,finish;
  ...
  GET_TIME(start);
  /*.串行代码.*/
  GET_TIME(finish);
  printf("Elapsed time = %e seconds\n",finish-start);
  ```

### 集合通信函数MPI_Barrier

- 保证同一个通信子中的所有进程都完成调用该函数之前，没有进程能够提前返回

  `int MPI_Barrier(MPI_Comm comm);`

- 例子：

  ```c
  double local_start, local_finish, local_elapsed, elapsed;
  ...
  MPI_Barrier(comm);
  ....
  ```

  





