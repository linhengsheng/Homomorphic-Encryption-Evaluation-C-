# 本次实验测试参考了SEAL官方的代码示例，对图像加密以及同态计算进行了性能测试
[SEAL](https://github.com/microsoft/SEAL)
### 本次实验使用的环境
     VS2022， 需要opencv(4.8.0)库以及SEAL(4.1)库
### 本次实验的目录结构
    root/   
        |-resources/    
        |     |-ciphers/  
        |     |-images/   
        |     |-key/  
        |-main.cpp  
        |-utils.cpp 
        |-examples.h    


### 本次实验实现的一些功能函数
#### Class CKKS
##### 对SEAL库中CKKS的加解密以及同态计算进行了相应的封装
## 相应的数据结构
#### vector<vector<double>>
     为图像的明文向量表示，每个行向量由图像的其中一个通道的所有像素值组成 
     形状即为 (c, h*w)，本次示例使用的多项式深度为4096，即单个行向量的维度为4096
#### vector<Ciphertext> 
     图像的密文向量表示，size为c，即每个行向量对应一个密文
## 以下是一些功能函数的封装
#### void search(CKKS& cryptor, vector<Ciphertext>& ciphers, const string& image_dir, string& str, vector<Ciphertext>& result, double& count_time);
     主要输入为一张图像的密文向量，以及所有密文图像存储的文件目录，然后输出找到的匹配图像的密文向量所在路径以及相应的密文
     函数中匹配的方式不依靠图像名称的索引，而是采用密态下计算余弦相似度的方式，相似度阈值为0.9999，与python实现的测试一致
#### void evaluate(CKKS& cryptor, Ciphertext& cipher, double& add_time, double& mul_time, double& dot_time);
     用于测试CKKS进行同态加密的逻辑计算性能
### 以下四个函数是对明文向量进行操作的函数，用于验证同态计算的正确性
#### void add(const vector<double>& vector_1, const vector<double>& vector_2, vector<double>& result);
     向量标量相加
#### double dot(const vector<double>& vector_1, const vector<double>& vector_2);
     向量内积
#### void mul(const vector<double>& vector_1, const vector<double>& vector_2, vector<double>& result);
     向量的标量乘积
#### double add_self(const vector<double>& vector);
     对向量内所有元素求和 
    
    

#### bool isImage(string path);
     判断某个文件是否为图像
#### void getImagePath(const string& dir, vector<string>& imagePaths);
     获取某个目录下的图像文件
#### void getImageVector(const string& str, vector<vector<double>>& result);
     将图像读取为上述明文向量的形式
#### void getFilePath(const string& dir, vector<string>& Paths);
     获取某个目录下的所有文件
#### void getSubDir(const string& dir, vector<string>& Paths);
     获取某个目录下的所有子目录
    
### 以下三种方法判断图像是否相等，为重载函数
#### bool equalImage(const string& str1, const string& str2);
#### bool equalImage(const vector<vector<double>>& image1, const vector<vector<double>>& image2);
#### bool equalImage(const string& str, const vector<vector<double>>& input);
### 获取高精度的时间点，单位为纳秒(ns)
#### long long getClockTime();
