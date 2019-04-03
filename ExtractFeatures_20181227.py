# -*- coding: utf-8 -*-
"""
Created on Thu Dec 27 15:57:20 2018

@author: Autolab
"""

import cv2
import os
import numpy as np
import pandas as pd
import time
import threading
from multiprocessing import Process

def FeatureExtraction(imageDirPath):
    start = time.time()
    #Get image file 
    imageDir = imageDirPath #image path
    image_files= os.listdir(imageDir)
    image_files.sort()
    image_files.sort(key= lambda x:int(x[:-4]))
    
    #files_np = np.array(image_files)
    all_route=[]
    
    for i in range(len(image_files)):
        routetemp = imageDir+image_files[i]
        all_route.append(routetemp)
    
    #Features empty list
    W_list=[]
    L_list=[]
    X_center_list=[]
    Y_center_list=[]
    X_Ellipse_list=[]
    Y_Ellipse_list=[]
    
    #Image feature extraction loop    
    for t in range(len(all_route)):
        img = cv2.imread(all_route[t],0)
	#print(t)
	#print("\n")
        img[0:41,0:61]=18  #remove the frame number
        
        gray = cv2.GaussianBlur(img, (5, 5), 1)
        edged = cv2.Canny(gray, 150, 240)
        
        #Width & length
        minX=9999
        maxX=0
        minY=9999
        maxY=0
      
        for i in range (len(edged[:,0])-1):
           for j in range (len(edged[0,:])-1):
            if edged[i,j]==255:
                if (j<minX) : minX=j
                elif (j>maxX) : maxX=j
                if (i<minY) : minY=i
                elif (i>maxY) : maxY=i
                
                width = maxX-minX
                length= maxY-minY
        
        #X.Y location
        X_center = (maxX+minX)/2
        Y_center = (maxY+minY)/2
    
        if minX==9999:   #black image set w & L =0
            width=0
            length=0
            X_center=0
            Y_center=0
                
        W_list.append(width)
        L_list.append(length)
        X_center_list.append(X_center)
        Y_center_list.append(Y_center)
        
        #Ellipse part
        # convert the grayscale image to binary image
        ret,thresh = cv2.threshold(gray,127,255,0)
    
        #Find contours in the binary image
        im2, contours, hierarchy = cv2.findContours(edged,cv2.RETR_TREE,cv2.CHAIN_APPROX_NONE)
        
        #Get X.Y of Ellipsecenter 
        if contours==[]:
            point=(0,0)
            axis=(0,0)
            angle=0
            X_Ellipse=point[0]
            Y_Ellipse=point[1]
        else:
            c=np.zeros(shape=(len(contours),1))
            for d in range (len(contours)):
                c[d,0]=len(contours[d])
            cnt = contours[np.argmax(c)]  
            point,axis,angle = cv2.fitEllipse(cnt)
            X_Ellipse=point[0]
            Y_Ellipse=point[1]
        
        X_Ellipse_list.append(X_Ellipse)
        Y_Ellipse_list.append(Y_Ellipse)
    
    #Convert list to array
    L_np=np.array(L_list)
    W_np=np.array(W_list)
    X_center_np=np.array(X_center_list)
    Y_center_np=np.array(Y_center_list)
    X_Ellipse_np=np.array(X_Ellipse_list)
    Y_Ellipse_np=np.array(Y_Ellipse_list)

    #Integrate features as dataframe
    features=pd.DataFrame({'Length':L_np, 
                          'Width':W_np, 
                          'X_center':X_center_np, 
                          'Y_center':Y_center_np,
                          'X_Ellipse_np':X_Ellipse_np,
                          'Y_Ellipse_np':Y_Ellipse_np
                          }
            )
    end = time.time();
    print(end - start)
    return features

features_13=FeatureExtraction("./1-3/")

features_13.to_csv('test1.csv')
