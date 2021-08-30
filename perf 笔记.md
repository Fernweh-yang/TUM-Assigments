https://ivanzz1001.github.io/records/post/linuxops/2017/11/16/linux-perf-usge#5-perf-stat%E7%9A%84%E4%BD%BF%E7%94%A8

# perf 简介

- perf是一款Linux性能分析工具。通过perf，应用程序可以利用PMU、tracepoint和内核中的计数器来进行性能统计。它不但可以分析指定应用程序的性能问题，也可以用来分析内核的性能问题，

## 背景知识

- Tracepoints:
  - tracepoints是散落在内核源代码中的一些hook，它们可以在特定的代码被执行到时触发，这一特性可以被各种trace/debug工具所使用。
  - perf将tracepoint产生的时间记录下来，生成报告，通过分析这些报告，调优人员便可以了解程序运行期间内核的各种细节，对性能症状做出准确的诊断。
  - 这些tracepoint对应的sysfs节点在/sys/kernel/debug/tracing/events目录下。
- Cache缓存
  - 内存读写是很快的，但是还是无法和处理器指令执行速度相比。为了从内存中读取指令和数据，处理器需要等待，用处理器时间来衡量，这种等待非常漫长。
  - cache是一种SRAM，读写速度非常快，能和处理器相匹配。因此，将常用的数据保存在cache中，处理器便无需等待，从而提高性能。
  - cache的尺寸一般都很小，充分利用cache是软件调优非常重要的部分。

# Perf使用

## 全局性概况

### Perf list

- 查看当前系统支持的所有性能事件。包括硬件性能事件、软件性能事件以及检查点

- perf list不能完全显示所有支持的事件类型 ，需要执行`sudo perf list`

- 还可以特定模块显式：

  - `hw/hardware`显式支持的相关硬件事件。

    ```
    sudo perf list hardware
    ```

  - ` sw/software`显示支持的软件事件列表

    ```
    sudo perf list sw
    ```

  - `cache/hwcache`显式硬件cache相关的事件列表

    ```
    sudo perf list cache
    ```

  - `pmu`显式支持的pmu时间列表

    ```
    sudo perf list pmu
    ```

  - `tracepoint`显式支持的所有tracepoint列表

    ```
    sudo perf list tracepoint
    ```

  - 指定性能事件

    ```
    -e <event>:u          //userspace
    
    -e <event>:k          //kernel
    
    -e <event>:h          //hypervisor
    
    -e <event>:G          //guest counting(in KVM guests)
    
    -e <event>:H          //host counting(not in KVM guests)
    ```

### perf bench

- perf中内置的benchmark基准，目前包括两套针对调度器和内存管理子系统的benchmark

### perf test

- 对系统进行健全性测试

### perf stat

- 执行某个命令，收集特定进程的性能概况，包括CPI、Cache丢失率等。可以对某一个程序进行全局性的性能统计

## 全局细节

### perf top

- 类似于linux的top命令，对系统性能进行实时分析。可以实时查看当前系统进程函数占用率情况

  perf top主要用于实时分析各个函数在某个性能事件上的热度，能够快速的定位热点函数，包括应用程序函数、模块函数与内核函数，甚至能够定位到热点指令。默认的性能事件为cpu cycles。

- 产生的信息：

  - 第一列： 符号引发的性能事件的比例，指占用CPU周期比例
  - 第二列： 符号所在的DSO(Dynamic Shared Object)，可以是应用程序、内核、动态链接库、模块
  - 第三列： DSO的类型。
    - `[.]`表示此符号属于用户态的ELF文件，包括可执行文件和动态链接库； 
    - `[k]`表示此符号属于内核或模块
  - 第四列： 符号名。有些函数不能解析为函数名，只能用地址表示

- 常用的交互命令

  ```
  h：显示帮助，即可显示详细的帮助信息。
  UP/DOWN/PGUP/PGDN/SPACE：上下和翻页。
  a：annotate current symbol，注解当前符号。能够给出汇编语言的注解，给出各条指令的采样率。
  d：过滤掉所有不属于此DSO的符号。非常方便查看同一类别的符号。
  P：将当前信息保存到perf.hist.N中。
  ```

- Perf top 常用选项

  - `-e <event>`: 指明要分析的性能事件

  - `-p <pid>`: 仅分析目标进程及其创建的线程。pid之间以逗号分割

  - `-k <path>`: 带符号表的内核映像所在的路径

  - `-K`: 不显示属于内核或模块的符号

  - `-U`: 不显示属于用户态程序的符号

  - `-d <n>`: 界面的刷新周期，默认为2s，因为perf top默认每2s从mmap的内存区域读取一次性能数据

  - `-G`: 得到函数的调用关系图

  - 例子：

    ```
    # perf top // 默认配置
    # perf top -G // 得到调用关系图
    # perf top -e cycles // 指定性能事件
    # perf top -p 23015,32476 // 查看这两个进程的cpu cycles使用情况
    # perf top -s comm,pid,symbol // 显示调用symbol的进程名和进程号
    # perf top --comms nginx,top // 仅显示属于指定进程的符号
    # perf top --symbols kfree // 仅显示指定的符号
    ```

- 具体程序示例

  代码：perf_test1.c

  ```c
  int main(int argc,char *argv[])
  {
  	int i;
  	while(1)
  		i++;
  	return 0x0;
  }
  ```

  编译运行：

  ```
  # gcc -g -o perf_test1 perf_test1.c
  # ./perf_test1
  ```

  运行后查看：

  ```
  # perf top
  Samples: 54K of event 'cycles:ppp', Event count (approx.): 37421490869        
  Overhead  Shared Object               Symbol                           
    88.05%  perf_test1                  [.] main            
     0.37%  libpython3.4m.so.1.0        [.] PyEval_EvalFrameEx         
     0.32%  bash                        [.] main                         
     0.23%  beam.smp                    [.] erts_alloc_init       
     0.20%  beam.smp                    [.] erts_foreach_sys_msg_in_q   
     0.19%  beam.smp                    [.] trace_sched_ports           
     0.13%  beam.smp                    [.] trace_virtual_sched 
  ```

  

### perf probe

- 用于自定义动态检查点

## **特定功能分析**

### perf kmem

- 针对slab子系统性能分析；

### perf kvm

- 针对kvm虚拟化分析

### perf lock 

- 分析锁性能

### perf mem 

-  分析内存slab性能

### perf sched 

- 分析内核调度器性能

### perf trace

- 记录系统调用轨迹

## 最常用功能

### perf record

-  收集采样信息，并将其记录在数据文件中。随后可通过其它工具对数据文件进行分析

```
sudo perf record ./student_submission
```

### perf report

- 读取perf record创建的数据文件，并给出热点分析结果。

```
sudo perf report -s pid | cat
```

例子：

```
1.
$ sudo perf record -a -g sleep 10  # record system for 10s
$ sudo perf report --sort comm,dso # display report

2.
$ sudo perf record ./serialbh -n 8192 -t 500
$ sudo perf report
```

### perf diff

- 对比两个数据文件的差异。能够给出每个符号（函数）在热点分析上的具体差异.

### perf evlist

- 列出数据文件perf.data中所有性能事件

### perf annotate

-  解析perf record生成的perf.data文件，显示被注释的代码。

### perf archive

-  根据数据文件记录的build-id，将所有被采样到的elf文件打包。利用此压缩包，可以再在任何机器上分析数据文件中记录的采样数据。

### perf script

- 执行perl或python写的功能扩展脚本、生成脚本框架、读取数据文件中的数据信息等

## 可视化工具

### perf timechart

- 针对测试期间系统行为进行可视化的工具,生成output.svg文档。 

## 负荷overhead

### counting:

-  内核提供计数总结，多是Hardware Event、Software Events、PMU计数等。相关命令perf stat
- 引入的额外负荷最小

### sampling

- perf将事件数据缓存到一块buffer中，然后异步写入到perf.data文件中。使用perf report等工具进行离线分析

- 在某些情况下会引入非常大的负荷

- 针对`sampling`，可以通过挂载建立在RAM上的文件系统来有效降低读写IO引入的负荷：

  ```
  # mkdir /tmpfs
  
  # mount -t tmpfs tmpfs /tmpfs
  ```

### bpf

- Kernel 4.4+新增功能，可以提供更多有效filter和输出总结
- 可以有效缩减负荷

