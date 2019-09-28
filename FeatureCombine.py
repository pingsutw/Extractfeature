# -*- coding: utf-8 -*-
"""
Created on Fri Jan 18 09:49:06 2019

@author: AUTO Lab, IMIS, iMRC, NCKU
"""

import pandas as pd
import numpy as np
import os
import time
from datetime import datetime
from datetime import timedelta

import matplotlib.pyplot as plt    
    
#%%
'''判斷點是否在多邊形內 reference: https://blog.csdn.net/qq_40771567/article/details/81509078'''
def isInsidePolygon(pt, poly):
    c = False
    i = -1
    l = len(poly)
    j = l - 1
    while i < l-1:
        i += 1
        if ((poly[i,0] <= pt[0] and pt[0] < poly[j,0]) or (poly[j,0] <= pt[0] and pt[0] < poly[i,0])):
            if (pt[1] < (poly[j,1] - poly[i,1]) * (pt[0] - poly[i,0]) / (poly[j,0] - poly[i,0]) + poly[i,1]):
                c = not c
        j = i
    return c

'''計算特徵'''
def calculate_indicator (layer_image_feature,layer_temper_data_time,layer_temper_data):
    AVM_features=pd.DataFrame(columns=['Length_min','Length_max','Length_mean','Length_var','Length_std','Length_skew',
                                       'Length_kurt','Length_1quantile','Length_2quantile','Length_3quantile','Length_range',
                                       'Length_quantile',
                                       'Width_min','Width_max','Width_mean','Width_var','Width_std','Width_skew',
                                       'Width_kurt','Width_1quantile','Width_2quantile','Width_3quantile','Width_range',
                                       'Width_quantile',
                                       'Temper_min','Temper_max','Temper_mean','Temper_var','Temper_std','Temper_skew',
                                       'Temper_kurt','Temper_1quantile','Temper_2quantile','Temper_3quantile','Temper_range',
                                       'Temper_quantile',
                                       ])
    #segment each sample time
    sample_no_array =  np.array(layer_image_feature.Sample_no,dtype=int)
    
    for i in range (max(layer_image_feature['Sample_no'])):
        sample_filter=(layer_image_feature['Sample_no']== i)
        
        temp_sample=layer_image_feature[sample_filter].copy()
        
        temp_sample[['Length','Width']] = temp_sample[['Length','Width']].apply(pd.to_numeric, downcast='float')
        
        temp_Length_min=temp_sample.Length.min()
        temp_Length_max=temp_sample.Length.max()
        temp_Length_mean=temp_sample.Length.mean()
        temp_Length_var=temp_sample.Length.var()
        temp_Length_std=temp_sample.Length.std()
        temp_Length_skew=temp_sample.Length.skew()
        temp_Length_kurt=temp_sample.Length.kurtosis()
        temp_Length_1quantile=temp_sample.Length.quantile(q=0.25)
        temp_Length_2quantile=temp_sample.Length.quantile(q=0.5)
        temp_Length_3quantile=temp_sample.Length.quantile(q=0.75)
        temp_Length_range=temp_Length_max-temp_Length_min
        temp_Length_quantile=temp_Length_3quantile-temp_Length_1quantile
        
        temp_Width_min=temp_sample.Width.min()
        temp_Width_max=temp_sample.Width.max()
        temp_Width_mean=temp_sample.Width.mean()
        temp_Width_var=temp_sample.Width.var()
        temp_Width_std=temp_sample.Width.std()
        temp_Width_skew=temp_sample.Width.skew()
        temp_Width_kurt=temp_sample.Width.kurtosis()
        temp_Width_1quantile=temp_sample.Width.quantile(q=0.25)
        temp_Width_2quantile=temp_sample.Width.quantile(q=0.5)
        temp_Width_3quantile=temp_sample.Width.quantile(q=0.75)
        temp_Width_range=temp_Width_max-temp_Width_min
        temp_Width_quantile=temp_Width_3quantile-temp_Width_1quantile
        
        #calculate temper features
        idx = np.where (sample_no_array==(i+1))[0]
            
        seg_idx_1 = (np.abs(idx-(np.quantile(idx,0.25)-1.5*(np.quantile(idx,0.75)-np.quantile(idx,0.25))))).argmin()
        #layer_image_feature.Timetag.iloc[idx[seg_idx_1+1]]
        #print ('sample ' +str(i+1)+ 'start:') 
        #print(layer_image_feature.Timetag.iloc[idx[seg_idx_1+1]])
        with open('pyro_layer_time.txt','a') as pyro_time_writer:
            pyro_time_writer.write('sample ' +str(i+1)+ 'start:'+(layer_image_feature.Timetag.iloc[idx[seg_idx_1+1]]).strftime('%Y/%m/%d-%H:%M:%S.%f')+'\n')
 
        
        temper_start_seg_idx = (layer_image_feature.Timetag.iloc[idx[seg_idx_1+1]]-layer_temper_data_time).total_seconds()*100000
        
        seg_idx_2 = (np.abs(idx-(np.quantile(idx,0.75)+1.5*(np.quantile(idx,0.75)-np.quantile(idx,0.25))))).argmin()
        #layer_image_feature.Timetag.iloc[idx[seg_idx_2-1]]
        #print ('sample ' +str(i+1)+ 'end:')
        #print(layer_image_feature.Timetag.iloc[idx[seg_idx_2-1]])
        with open('pyro_layer_time.txt','a') as pyro_time_writer:
            pyro_time_writer.write('sample ' +str(i+1)+ 'end:'+(layer_image_feature.Timetag.iloc[idx[seg_idx_2-1]]).strftime('%Y/%m/%d-%H:%M:%S.%f')+'\n')

        
        temper_end_seg_idx = (layer_image_feature.Timetag.iloc[idx[seg_idx_2-1]]-layer_temper_data_time).total_seconds()*100000
        
        temp_temper_array = np.array(layer_temper_data[int(temper_start_seg_idx):int(temper_end_seg_idx)],dtype=float)
        print(temper_start_seg_idx,temper_end_seg_idx)
        temp_Temper_min=np.min(temp_temper_array)
        temp_Temper_max=np.max(temp_temper_array)
        temp_Temper_mean=np.mean(temp_temper_array)
        temp_Temper_var=np.var(temp_temper_array)
        temp_Temper_std=np.std(temp_temper_array)
        temp_Temper_skew=pd.DataFrame(temp_temper_array).skew()[0]
        temp_Temper_kurt=pd.DataFrame(temp_temper_array).kurtosis()[0]
        temp_Temper_1quantile=np.quantile(temp_temper_array,0.25)
        temp_Temper_2quantile=np.quantile(temp_temper_array,0.5)
        temp_Temper_3quantile=np.quantile(temp_temper_array,0.75)
        temp_Temper_range=temp_Temper_max-temp_Temper_min
        temp_Temper_quantile=temp_Temper_3quantile-temp_Temper_1quantile        
        
        
        AVM_features.loc[i]=[temp_Length_min, temp_Length_max, temp_Length_mean, temp_Length_var, temp_Length_std,
                    temp_Length_skew, temp_Length_kurt, temp_Length_1quantile, temp_Length_2quantile,temp_Length_3quantile, 
                    temp_Length_range, temp_Length_quantile,
                    temp_Width_min, temp_Width_max, temp_Width_mean, temp_Width_var, temp_Width_std, temp_Width_skew,
                    temp_Width_kurt, temp_Width_1quantile, temp_Width_2quantile, temp_Width_3quantile, temp_Width_range,
                    temp_Width_quantile,
                    temp_Temper_min, temp_Temper_max, temp_Temper_mean, temp_Temper_var, temp_Temper_std, temp_Temper_skew,
                    temp_Temper_kurt, temp_Temper_1quantile, temp_Temper_2quantile, temp_Temper_3quantile, temp_Temper_range,
                    temp_Temper_quantile,
                    ]
    return AVM_features


#%%
'''模擬線上收值使用，上線不需要此部分'''
image_csv_simulation = './0724csv_1_simulation' 
pyro_txt_simulation =  './0724pyro_1_simulation' 
#read pyro file data name
pyro_simulation= os.listdir(pyro_txt_simulation) 
pyro_simulation.sort()

pyro_simu_time =np.zeros(shape=(len(pyro_simulation),))
t=0
for i in pyro_simulation:
    time_idx = i.split('.')[0].split('_')
    pyro_simu_time[t] = int(time_idx[0])*3600 + int(time_idx[1])*60 + int(time_idx[2]) + int(time_idx[3])/100000
    t+=1

#read image file data name
image_simulation= os.listdir(image_csv_simulation) 
image_simulation.sort()

image_simu_time =np.zeros(shape=(len(image_simulation),2))
t=0
for i in image_simulation:
    time_idx = i.split('.')[0].split('-')[1].split('_')
    image_simu_time[t,0] = int(time_idx[0])*3600 + int(time_idx[1])*60 + int(time_idx[2]) + int(time_idx[3])/100000
    t+=1
    
for i in range (int(len(pyro_simulation)/30)):
    c = image_simu_time[:,0]<(pyro_simu_time[int(len(pyro_simulation)/30-i)*30]-1)
    for j in range (len(image_simu_time)):
        if c[j]:
            image_simu_time[j,1]=int(len(pyro_simulation)/30)-i-1    


'''設定加工日期、image csv資料夾、溫度直資料夾'''
#setting
date = '2019/07/24'
image_csv_file = './0724csv_1' 
pyro_txt_file =  './0724pyro_1' 
sample_position = './sample_position.csv'

'''讀取樣本位置'''
sample_position = pd.read_csv(sample_position, index_col=0)
sample_position = np.array(sample_position.iloc[:,:-2]).reshape(len(sample_position),-1,2)

'''溫度資料處理初始化參數'''
#initial for pyro_data 
layer = 0
next_layer = 1
count = 0
pyro_calculate_features, image_calculate_features= False , False

check_window_layer = np.zeros(shape = (20000,))+5000  #0.2 seconds moving window
state_waitting_layer = True # record if priting layer
wait_time_layer=0 

remain_temper=[]

while True:   #continous loop
    print(datetime.now())
    print('count:'+str(count))
    
    '''搬感測器資料，模擬線上'''    
    for i in range (30):
        os.rename(pyro_txt_simulation+'/'+pyro_simulation[count*30+i],pyro_txt_file+'/'+pyro_simulation[count*30+i])

    for i in range (len(image_simulation)):
        if  image_simu_time[i,1] == count:
            os.rename(image_csv_simulation+'/'+image_simulation[i],image_csv_file+'/'+image_simulation[i])
    
    
    '''間隔時間讀取pyro溫度及image csv檔資料夾'''
    time.sleep(1)    #time gap: 1 second
        
    #read pyro file data name
    pyro_files= os.listdir(pyro_txt_file) #get all name of this file
    pyro_files.sort()
    
    #read image file data name
    image_files= os.listdir(image_csv_file) #get all name of this file
    image_files.sort()

    '''獲得溫度開始時間，並存放到pyro_layer_time.txt'''
    #initial pyro time
    if count == 0:
        #pyro_start_time = pyro_files[0].split('.')[0]
        pyro_start_time = datetime.strptime(date+'-'+pyro_files[0].split('.')[0]+'0', '%Y/%m/%d-%H_%M_%S_%f')
        current_temepr_time = pyro_start_time
        
        layer_temper_data_time = pyro_start_time
        layer_end_time = pyro_start_time + timedelta(days=1) #first set long time
        test=pyro_start_time #remove
        
        with open('pyro_layer_time.txt','a') as pyro_time_writer:
            pyro_time_writer.write('Pyro start at '+pyro_start_time.strftime('%Y/%m/%d-%H:%M:%S.%f')+'\n')
        
        layer_temper_data = []
        
    current_temper_data = []

    '''讀取溫度資料，轉換成溫度值，並串聯起來'''
    #read pyro data, calculate temperature and concate
    for i in range (len(pyro_files)):  #-1 remember remove, only for check
        
        temp = open(pyro_txt_file+'/'+pyro_files[i], "r")
        lines = temp.read().split(',')
        temp.close()
        
        if lines[-1]=='':
            del lines[-1]
            
        if len(lines)%2 !=0 :    #prevent odd condition
            del lines[-1]
        
        temp_array = np.array (lines,dtype=np.int).reshape(-1,2)
        
        temp_temper = ((temp_array[:,0]*pow(2,8) + temp_array [:,1])*0.061257618916352723 + 492.77160096787043)*10
        
        current_temper_data.extend(list(temp_temper[temp_temper>4928]))  #remove reducdant data
        
        os.remove(pyro_txt_file+'/'+pyro_files[i]) #remove load data
    
    layer_temper_data.extend(current_temper_data)
    
    test=test+len(current_temper_data)*timedelta(microseconds=10) #remove please
    print(test)
    
    '''
    利用moving windows尋找每層開始以及結束時間點
    如果沒雷射的時候，逐點確認
    有雷射的時候，每0.1秒(10000)跳著確認
    '''
    remain_temper.extend(current_temper_data)  #把剩的串接起來
    
    leave=False
    no=0 #current calculate number
    remain_len=len(remain_temper) 
    #while (len(remain_temper)>10000):#(no<remain_len):
    while (remain_len>10000):# or count==0):
        if  leave:
            break
        while state_waitting_layer:
            temp_temper = remain_temper[no]
            if  temp_temper>5020:
                layer += 1
                state_waitting_layer = False
                wait_time_layer=0
                
                #write pyro time
                layer_start_time = current_temepr_time + (no+1)*timedelta(microseconds=10)
                with open('pyro_layer_time.txt','a') as pyro_time_writer:
                    pyro_time_writer.write('Layer '+str(layer)+' start at '+layer_start_time.strftime('%Y/%m/%d-%H:%M:%S.%f')+'\n')
            no+=1
            remain_len-=1
            if no==len(remain_temper):
                #remain_temper=[]
                break
            
        if no==len(remain_temper):
            remain_temper=[]
            break
        else:
            remain_temper=remain_temper[no:]
        #if no<len(remain_temper):
        #    remain_temper=remain_temper[no:]
        
        if (not state_waitting_layer):
            #wait_time_layer +=1    
            for i in range (int(len(remain_temper)/10000)):
                #check_window_layer=remain_temper[i*10000:(i+1)*10000]
                check_window_layer=remain_temper[0:10000]
                #if (not state_waitting_layer):
                #    wait_time_layer+=10000 
                wait_time_layer+=10000            
                no+=10000    
                #print('wait_time_layer:',wait_time_layer)
                #print('no:',no)
                remain_temper=remain_temper[10000:]
                remain_len-=10000
                if ((wait_time_layer>1000000) & (np.mean(check_window_layer)<5010)):   #前十秒忽略
                    state_waitting_layer = True
                    pyro_calculate_features = True
                    
                    wait_time_layer=0
                    
                    if layer == next_layer:
                        #write pyro time
                        layer_end_time = current_temepr_time + (no+1)*timedelta(microseconds=10)
                        with open('pyro_layer_time.txt','a') as pyro_time_writer:
                            pyro_time_writer.write('Layer '+str(layer)+' end at '+layer_end_time.strftime('%Y/%m/%d-%H:%M:%S.%f')+'\n')
                    
                        next_layer +=1
                        print('layer_end_time:',layer_end_time)
                    leave=True
                    break 
                
        #if (not state_waitting_layer) and (no>(remain_len-10000)):
        #    break
        
        #remain_temper=remain_temper[-(len(remain_temper)%10000):]
    current_temepr_time = current_temepr_time + (no+1)*timedelta(microseconds=10)
    #print('PYRO')
    print('current_temepr_time:',current_temepr_time)
    #print('state_waitting_layer:',state_waitting_layer)
    #print('wait_time_layer:',wait_time_layer, ' mean:',np.mean(check_window_layer))
    
    #read image data and concate
    if count == 0:
        layer_image_feature=pd.DataFrame(columns=['Length','Width','X_center','Y_center','Timetag'])
    
    current_image_feature=pd.DataFrame(columns=['Length','Width','X_center','Y_center','Timetag'])
        
        
    for i in range (len(image_files)):
        temp_feature = pd.read_csv(image_csv_file+'/'+image_files[i],index_col=0)
        
        #Get the time of this csv file
        filename_split=image_files[i].split('.')[0].split('_')
        filename_combine = date+' '+filename_split[0]+':'+filename_split[1]+':'+filename_split[2]+'.'+filename_split[3]+'0' 
        filetime = datetime.strptime(filename_combine, '%Y/%m/%d video-%H:%M:%S.%f')
        
        temp_feature['Timetag']=filetime
        
        for t in range (len(temp_feature)):
            temp_feature.loc[t,'Timetag']=filetime + t*timedelta(microseconds=400)
        
        #remove too big meltpool
        index = (temp_feature['Length']>=30) | (temp_feature['Width']>=30)
        
        feature_remove = temp_feature.copy()
        feature_remove.loc[index,0:]=0
        
        #remove 0 value rows
        feature_remove = feature_remove[feature_remove.Length!=0]
        feature_final = feature_remove[feature_remove.Width!=0]
        
        #combine features 
        current_image_feature = pd.concat([current_image_feature,feature_final],axis=0,sort=False)
        #print('IMAGE')
        #print('state_waitting_layer:',state_waitting_layer)
        #print('filetime:',filetime,'layer_end_time:',layer_end_time)
        print('filetime:',filetime)
        print('state_waitting_layer:',state_waitting_layer)
        if state_waitting_layer & (filetime > layer_end_time):
            image_calculate_features=True      
            
        os.remove(image_csv_file+'/'+image_files[i]) #remove load data
        
    layer_image_feature = pd.concat([layer_image_feature,current_image_feature],axis=0,sort=False)
     
        
    count+=1
    '''計算各個樣本特徵'''
    #sample match and calculate features
    if pyro_calculate_features & image_calculate_features:
        print(datetime.now())
        print('calculate layer '+str(layer)+' feature')
        
        #get sample position
        position=np.array(layer_image_feature.loc[:,['X_center','Y_center']])
        
        #mover image position to actual position
        position[:,0] = 160 - position[:,0] #0726+
        position[:,1] = 160 - position[:,1] #0726+    
        
        #position[:,0] = (position[:,0]-98) * 140/75
        #position[:,1] = (position[:,1]-75) * 140/93
        
        position[:,0] = (position[:,0]-90) * 140/75
        position[:,1] = (position[:,1]-57) * 140/93
        
        #initial feature sample_no        
        layer_image_feature['Sample_no'] = 0
        
        #allocate sample number
        for i in range (len(layer_image_feature)):
            for j in range (len(sample_position)):
                c = isInsidePolygon(position[i],sample_position[j])
                if c==True:
                    layer_image_feature.iloc[i,-1]=j+1
                    break
        
        print(layer_temper_data_time)
        print(layer_image_feature['Timetag'].iloc[0])
        print(layer_image_feature['Timetag'].iloc[-1])
        print(layer_temper_data_time + len(layer_temper_data)*timedelta(microseconds=10))
        AVM_features = calculate_indicator(layer_image_feature,layer_temper_data_time,layer_temper_data)
        
        '''輸出每層詳細資料 測試用 上線請移除'''
        with open('./all_data_check/'+layer_temper_data_time.strftime('%H_%M_%S_%f')+'.txt', 'w') as f:
            for item in layer_temper_data:
                f.write("%s\n" % item)
        
        layer_image_feature.to_csv('./all_data_check/layer_'+str(layer)+'_allfeature.csv')
        
        #calculate image features
        #AVM_image_features = calculate_image_indicator(layer_image_feature)
            
        #AVM_image_features.to_csv('layer_'+str(layer)+'_feature.csv')
        
        AVM_features.to_csv('layer_'+str(layer)+'_feature.csv')
        
        #reset data
        layer_temper_data_time =  layer_temper_data_time + len(layer_temper_data)*timedelta(microseconds=10)   
        layer_temper_data = []
        pyro_calculate_features, image_calculate_features = False, False
        
        layer_image_feature=pd.DataFrame(columns=['Length','Width','X_center','Y_center','Timetag'])
