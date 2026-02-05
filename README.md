# Update 
2025.12.20  
Our paper has been accepted by the OLT, and I will update our code soon.  

2025.05.17   

We have submitted our manuscript to the Optics & Laser Technology Journal, wish us good luck. We will release our code immediately upon acceptance


This project implements a Flexible Order Modulator (FOM) based image denoising algorithm with low-rank regularization to improve denoising performance. The algorithm is suitable for complex noise scenarios such as Poisson noise.  It is worth noting that the modeling Gaussian noise not shown in the uploaded code is included in the Poisson noise. You can safely use the modified code.  

# Infrared_Denoise

We propose a denoising model for infrared images designed for hybird noise.


We will publicly open all code and resources after the paper is accepted. Thank you for your attention.  
 
 
If you'd like to read about non-uniform denoising in infrared images, you can check out this link.
https://github.com/ImageVisioner/InfraredNUC/   

##  / Project Structure

```
FOM_Denoising/
├── main.m                    # 主函数入口 / Main entry point
├── Functions/               # 函数文件夹 / Functions directory
│   ├── FOM.m                # FOM去噪核心算法 / FOM denoising core algorithm
│   ├── defDDt.m             # 差分算子定义 / Difference operator definition
│   ├── cconv2.m             # 循环卷积 / Circular convolution
│   └── PSNR.m               # PSNR计算函数 / PSNR calculation function
├── Results/                  # 结果输出文件夹 / Results output directory (auto-created)
├── processed_noise_0005.png # 测试图像 / Test image
├── irdenoise.pdf            # 相关论文 / Related paper
├── FOTV_denoising.pdf       # 原始算法论文 / Original algorithm paper
└── README.md                # 说明文档 / Documentation
```

---
**Dataset** 
We have used both real datasets and simulated datasets for experiments. The real dataset is used to verify the denoising performance of real situations. The purpose of the simulated dataset is to explore the potential performance of our method.


We use the image from the link below for denoising.

http://openai.raytrontek.com/apply/E_Image_noise_reduction.html/

It provides RAW format images, and I provide batch processing code to convert them into readable and writable formats, such as png, bmp, etc.

<center>
<img src="figs/1.jpg" width=80%>
</center


---


**Results** 

*Visualize the results*
Real dataset:

<center>
<img src="figs/res1.jpg" width=80%>
</center>


*Quantitative Evaluation*
<center>
<img src="figs/image.png" width=80%>
</center>



---








```
@inproceedings{llvip,
  title={LLVIP: A visible-infrared paired dataset for low-light vision},
  author={Jia, Xinyu and Zhu, Chuang and Li, Minzhen and Tang, Wenqi and Zhou, Wenli},
  booktitle={Proceedings of the IEEE/CVF international conference on computer vision},
  pages={3496--3504},
  year={2021}
}

```











**Contact**   
If you encounter any problems during use, please do not hesitate to email me.     
Email:imagevisioner@outlook.com    or lh_010625@163.com

---


**Acknowledgments**   

Our work has benefited from the National Natural Science Foundation of China. And the work on the following two datasets. Please cite when it is helpful to you.
``` 
@article{liu2026infrared,
  title={Infrared image denoising: A hybrid noise removal pipeline with anti-artifact regularization},
  author={Liu, Chenhua and Li, Hao and Li, Maoyong and Deng, Lei and Dong, Mingli and Zhu, Lianqing},
  journal={Optics \& Laser Technology},
  volume={193},
  pages={114174},
  year={2026},
  publisher={Elsevier}
}
``` 

``` 
@article{liu2025multi,
  title={Multi-stage non-uniformity correction pipeline for single-frame infrared images based on hybrid high-order directional and low-rank prior information},
  author={Liu, Chenhua and Li, Hao and Li, Maoyong and Deng, Lei and Dong, Mingli and Zhu, Lianqing},
  journal={IEEE Sensors Journal},
  year={2025},
  publisher={IEEE}
}
```

