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

  

