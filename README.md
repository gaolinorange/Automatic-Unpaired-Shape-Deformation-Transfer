# [Automatic-Unpaired-Shape-Deformation-Transfer](http://geometrylearning.com/ausdt)
Code for "Automatic Unpaired Shape Deformation Transfer", Siggraph Asia 2018
<p align='center'>  
  <img src='imgs/teaser.jpg' width='880'/>
</p>
We will include more details soon.

## Note for VC-Gan 
- Linux
- Python 2 or 3
- NVIDIA GPU (larger than 8G) + CUDA cuDNN
- Tensorflow 1.10
- We add a script 'matlabscript/GetICPandLFDDist.m' to compute the distance(similarity) between two models.


## Note for ACAP Lib

### News

- The ACAP Lib supports the Linux OS!

### Prerequisites
- Windows is supported.
- Linux is supported, the implementation is located in '''ACAP_linux'''.
### Dependencies
- Install [GUROBI Optimizer 8.0](http://www.gurobi.com/), you must apply a license to active the gurobi to compute the larger optimizations.
- Install [OpenMesh 7.0](https://www.openmesh.org/download/)
- Install [Matlab 2017b](https://www.mathworks.com/)
- Install [GMM 5.2](http://getfem.org/download.html)
- Install [CoMISo](https://graphics.rwth-aachen.de:9000/CoMISo/CoMISo) VS2017 Complied
- Install [Eigen 3.3.3](http://eigen.tuxfamily.org/index.php?title=Main_Page)


## Citation

If you find this useful for your research, please use the following.

```
@article{gao2018automatic,
  title={Automatic unpaired shape deformation transfer},
  author={Gao, Lin and Yang, Jie and Qiao, Yi-Ling and Lai, Yukun and Rosin, Paul and Xu, Weiwei and Xia, Shihong},
  journal={ACM Transactions on Graphics},
  volume={37},
  number={6},
  pages={1--15},
  year={2018},
  publisher={Association for Computing Machinery (ACM)}
}
```
