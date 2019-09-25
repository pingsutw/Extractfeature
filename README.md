# Extractfeature

TODO : introduction


## Pre-requested 
* C++ or python basic 
* OpenCV skills 
* gcc or MSVC

## How to use
Download project 
```shell
git clone https://github.com/pingsutw/Extractfeature.git
```
## Download sample input 
```shell
https://drive.google.com/drive/folders/1cLeVbbSnbldD_98JT_J4XSArp3bB1GvL?fbclid=IwAR2qzsas_gXLIzdrnleRVCJR2ZoW_FVulUou4G7TtKtLx-9J8C9l-aOdTpc
```
### Windows
Open with MSVC
```
Open ./opencv.sln 
```
### Ubuntu 
```
g++ ./opencv/source.cpp -o output `pkg-config --cflags --libs opencv`
./output 
```

## Goal 
1. Dectet point of laser on the metal 
2. real time analyze
3. reduce flaw of of manufacture product 


## To-do
1.溫度串接加速  
2.計算特徵 減少重複呼叫 
3.跑太久的特徵 考慮刪除
