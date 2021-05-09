# CPthread

## 编译

```
gcc -pthread -o pthread pthread.c
./pthread <thread个数>
```

## 用法

- 创建线程

  ```c
  int pthread_create(pthread_t *thread,const pthread_attr_t *attr,void *(*start_routine)(void *), void *arg);
  ```

  - 第一个参数是一个指针，指向对应的pthread_t对象
    - 注意，必要再调用pthread_create函数前就为pthread_t对象分配内存空间
  - 第二个参数是新线程的属性。不用是把NULL传递给参数
  - 第三个参数指向新线程的调用，也就是器要运行的函数
  - 最后一个参数是一个指针，指向传给start_routine函数的参数
  - 如果成功，返回0

- 停止线程

  ```c
  int pthread_join(pthread_t thread, void **retval);
  ```

  - 第一个参数是一个指针，指向对应的pthread_t对象
  - 第二个参数可以接受任意由pthread_t对象所关联的那个线程产生的返回值。

- 例子

  ```c
  #include <pthread.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <assert.h>
  #define NUM_THREADS 5
  void* perform_work(void* argument)
  {
      int passed_in_value;
      passed_in_value = *((int*) argument);
      printf("Hello World! It's me, thread with argument%d!\n", passed_in_value);
      return NULL;
  }
  
  int main(int argc, char** argv)
  {
      pthread_t threads[NUM_THREADS];
      int thread_args[NUM_THREADS];
      int result_code; 
      unsigned index; 
      // create all threads one by one
      for (index = 0; index < NUM_THREADS; ++index)
      {
          thread_args[ index ] = index;
          printf("In main: creating thread %d\n", index);
          result_code = pthread_create(&threads[index],NULL,perform_work, &thread_args[index]);
          assert(!result_code);
      }
      // wait for each thread to complete
      for (index = 0; index < NUM_THREADS; ++index)
      {
          // block until thread 'index' completes
          result_code = pthread_join(threads[index], NULL);
          assert(!result_code);
          printf("In main: thread %d has completed\n",index);
      }
      printf("In main: All threads completed successfully\n");
      exit(EXIT_SUCCESS);
  } 
  ```


## 不同线程间的同步

当多个线程都要访问共享变量/文件,其中至少一个访问时更新操作时，这些访问可能会产生某种错误，称之为race condition竞争条件。所以要保证某线程开始执行某操作时，其他线程不能执行此操作，此时此操作(一段代码)就是一个临界区。以下两个方法保证一个线程独享临界区：

1. Locks / Mutual     Exclusion互斥锁
2. Semaphore信号量

### 互斥锁

- 初始化互斥锁mutex：

  ```c
  int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
  ```

- 使用完后取消互斥锁

  ```c
  int pthread_mutex_destroy(pthread_mutex_t *mutex);
  ```

- 获得临界区的访问权,线程需要调用

  ```c
  int pthread_mutex_lock(pthread_mutex_t *mutex);
  ```

- 线程退出临界区后，应该调用

  ```C
  int pthread_mutex_unlock(pthread_mutex_t *mutex);
  ```

- 例子

  ```C
  int account = 100;
  pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  void deposit(int money)
  {
      pthread_mutex_lock(&mutex);
      #account是这里各个线程的共享数据，即临界区
      account = account + money;
      pthread_mutex_unlock(&mutex); return 0;
  }
  ```

- 当加速比T串/T并=thread_count线程数时，达到理想的性能。

### 条件变量

- 路障barrier: 当所有线程到达程序中同一位置，即同步点(路障),线程才能继续运行下去。

  - 可以用来调试程序
  - 实现方法：
    - 忙等待
    - 互斥锁
    - 条件变量

- 条件变量Condition Variables

  - 条件变量是一个数据对象，允许线程再某个特定条件或事件发生前都处于挂起状态。

  - 解锁一个阻塞的线程

    ```c
    int pthread_cond_signal(pthread_cond_t *cond_var);

  - 解锁所有被阻塞的线程

    ```C
    int pthread_cond_broadcast(pthread_cond_t *cond_var);
    ```

  - 通过互斥锁mutex来阻塞线程

    ```C
    int pthread_cond_wait(pthread_cond_t *cond_var, pthread_mutex_t *mutex)
    ```

  - 初始化

    ```c
    int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *cond_attr_p)
    ```

  - 销毁

    ```c
    int pthread_cond_destroy(pthread_cond_t *cond)
    ```

  - 例子

    ```C
    //Global information
    int count = 0;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    //Waiting thread
    Pthread_mutex_lock(&mutex);
    While (count<=100) //Guard against sporadic wake-ups!防止零散的唤醒
    {
        pthread_cond_wait(&cond,&mutex);
    }
    pthread_mutex_unlock(&mutex);
    //Signaling thread
    for (i=0; i<1000; i++)
    {
        pthread_mutex_lock(&mutex);
        count++;
        if (count==100)
        {
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }   
    ```

    

