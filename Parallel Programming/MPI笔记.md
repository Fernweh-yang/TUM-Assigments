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