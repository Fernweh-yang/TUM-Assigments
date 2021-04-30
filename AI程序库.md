# Numpy

import numpy as np

## 矩阵运算

- 点乘： 

  ```python
  np.dot(X,Y)
  ```

- 转置

  ```python
  np.transpose(X)
  ```

- 逆矩阵

  ```python
  # 求逆
  np.linalg.inv(X)	#Matlab中inv()函数
  
  #求伪逆
  np.linalg.pinv(X)	#Matlab中pinv()函数
  ```

  

# Pandas

import pandas as pd

## 存取文件

- 读取csv文件：

  ```python
  pd.read_csv("data/xx.csv")
  ```

  