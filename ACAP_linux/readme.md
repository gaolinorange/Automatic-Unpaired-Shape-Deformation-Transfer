
# As-Consistent-As-Possible Deformation Representation (ACAP)
The implementation of ACAP.
Part of our sparse deformation [algorithm](https://arxiv.org/abs/1709.01250).

```
ACAP: Sparse Data Driven Mesh Deformation
Lin Gao, Yu-Kun Lai, Jie Yang, Ling-Xiao Zhang, Shihong Xia, Leif Kobbelt
IEEE Transactions on Visualization and Computer Graphics, 2019
```


## Dependencies
- [Boost 1.65.1](https://www.boost.org/)
- [Blas & Lapack](https://www.netlib.org/lapack/lug/node11.html)
- [GMM](http://getfem.org/download.html)
- [Eigen3](http://eigen.tuxfamily.org/index.php?title=Main_Page)
- [GUROBI Optimizer 8.0](http://www.gurobi.com/), you must apply a license to active the gurobi to compute the larger optimizations.
- [OpenMesh 7.0](https://www.openmesh.org/download/)
- [CoMISo](https://graphics.rwth-aachen.de:9000/CoMISo/CoMISo)

Note: we test the code on the OS platform with ```ubuntu 18.04.3, g++ 7.5, gcc 7.5```. All above the dependencies are provided, which are located in folder ```3rd```. Of course, you also could download them from theirs offical websites and complie them by yourself.

## ACAP Library

### 1. Boost and Blas & Lapack

Install the two libraries.
```sh
sudo apt-get install libboost-all-dev
sudo apt-get install libopenblas-dev libblas-dev liblapack-dev
```

### 2. Gurobi

We have provide the ```gurobi8.0.1``` libiary in the folder ```3rd```, you can download it by yourself on the [GUROBI website](https://www.gurobi.com/downloads/gurobi-optimizer-eula/). You could only unzip the installation files with any opration. Then, you must regist a academic license to active the software by the command:

```
./bin/grbgetkey license-serial-number
```

Note: The above instructions also can be found in the GUROBI website.


### 3. Install

```sh
git clone --recursive https://github.com/gaolinorange/Automatic-Unpaired-Shape-Deformation-Transfer.git
cd Automatic-Unpaired-Shape-Deformation-Transfer/ACAP_linux
```

Note: You need to modify the variable ${GUROBI_ROOT} in CMakeLists.txt

Then, run the scripts step by step. (You must make sure that each library is successfully installed)

```sh
 install_CoMISo.sh
 install_gmm.sh
 install_OpenMesh.sh
 install_ACAP.sh
```

### 4. Example

Usage: ```./main path_to_folder [-opt] 0/1```

Output: Two files(```LOGRNEW.txt``` and ```S.txt```), which stored a matrix with size```N * (V*9)``` respectively. ```N``` is the number of shapes in the ```path_to_folder```. ```V``` is the number of vertices of shape. Each row of file ```LOGRNEW.txt``` and ```S.txt``` represent the ACAP feature of a shape(For the detils of algorithm, please refer to sparse deformation [algorithm](https://arxiv.org/abs/1709.01250)). For the row 1, row 2,..., row 3 of feature files, every row coresponds to the [Natural Sorting](https://en.wikipedia.org/wiki/Natural_sort_order) of names of shape. 


Where:

```sh
  path_to_folder    The path of shape folder.
  -opt 0/1          Use integer optimization or not.
```
In order to check whether the code runs successfully, we provide a simple test example. In the folder ```test```, we also provide the GT ACAP feature files(```LOGRNEW_GT.txt``` and ```S_GT.txt```) for the three obj files ```1.obj, 2.obj, 3.obj```. You can check the code by comparing the output files with GT ACAP feature files. The output files can be obtained by the following command, the files will be stored in your input path.

```sh
cd ./ACAP-build/ACAP-build
./main ../../test -opt 1
```

Suggestions: It is best not to contain too many shapes in the input folder. 

## Citation

If you find this useful or use our code in your research, please cite our work by the following.

```
@article{gao2019sparse,
  title={Sparse data driven mesh deformation},
  author={Gao, Lin and Lai, Yu-Kun and Yang, Jie and Ling-Xiao, Zhang and Xia, Shihong and Kobbelt, Leif},
  journal={IEEE transactions on visualization and computer graphics},
  year={2019},
  publisher={IEEE}
}
```
