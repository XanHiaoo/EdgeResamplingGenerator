# 边缘重采样生成器EdgeResamplingGenerator

## 项目简介

> ### 该项目工程旨在将检测器件的边缘（包括不规则角，圆弧，内控等）通过计算展平为规则的矩形图像，从而便于在边缘区域上使用常规的算子

## 工程简介

工程基于cmake构建。请阅读cmake进行生成

> EdgeResamplingGenerator
> ├─src
> 	│  ├─EdgeResamplingGeneratorImpl.h
> 	│  ├─EdgeResamplingGeneratorImpl.cpp
> 	│  ├─Test.cpp
>
> ├─out
>
> ├─third_party
> 	│  └─opencv470
>
> ├─CmakeList

## 示例

### 采样

对边缘的重采样基于检测原图与检测区域mask，以手机屏幕中摄像孔为例

> - 掩膜与原图
>
>   <center class="half">
>       <img src="https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221161809437.png" width="500"/>
>       <img src="https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221161804485.png" width="500"/>
>   </center>
>
> - 内孔洞
>
>   <center class="half">
>       <img src="https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221163149757.png" width="500"/>
>       <img src="https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221163146723.png" width="500"/>
>   </center>
>
> - 边框
>
>   <center class="half">
>       <img src="https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221163959034.png" width="500"/>
>       <img src="https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221163955724.png" width="500"/>
>   </center>

### 重采样

- 边框

  ![image-20240221163812674](https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221163812674.png)

- 内孔

  ![image-20240221163826772](https://imgurl-x.oss-cn-hangzhou.aliyuncs.com/xuxing-img/image-20240221163826772.png)