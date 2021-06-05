# **Advanced Finding Lane Lines on the Road** 
[![Udacity - Self-Driving Car NanoDegree](https://s3.amazonaws.com/udacity-sdc/github/shield-carnd.svg)](http://www.udacity.com/drive)


The Goal
---
The goal of this project is to automatically detect lane lines on a road, calculate the curvature and the position of the vehicle with respect to the lane

Input Image                |  Output Image
:-------------------------:|:-------------------------:
<img src="test_images/straight_lines1.jpg"/>  | <img src="output_images/straight_lines1.jpg"/>

The Pipeline
---
1. Compute the camera calibration matrix and distortion coefficients given a set of chessboard images. <img src="readMe_images/1.png"/>
2. Apply a distortion correction to raw images. <img src="readMe_images/2.png"/>
3. Use color transforms, gradients, etc., to create a thresholded binary image. <img src="readMe_images/3.png"/>
4. Apply a perspective transform to rectify binary image ("birds-eye view"). <img src="readMe_images/4.png"/>
5. Detect lane pixels and fit to find the lane boundary. 
6. Determine the curvature of the lane and vehicle position with respect to center. <img src="readMe_images/5&6.png"/>
7. Warp the detected lane boundaries back onto the original image. 
   
   <img src="readMe_images/7.png"/>
8. Output visual display of the lane boundaries and numerical estimation of lane curvature and vehicle position. <img src="output_images/test4.jpg"/>
9. Create a pipeline for videos. 
  
  <img src="output_images/gif-output.gif"/>

Shortcomings
---
The algorithm works well when there is no change in light exposure but could struggle if it goes under a bridge for example.

Possible Improvements
---
Only check neighbor pixels to the lanes detected in the previous frame.
