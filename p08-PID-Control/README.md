# Project 8 - PID Controller

[![Udacity - Self-Driving Car NanoDegree](https://s3.amazonaws.com/udacity-sdc/github/shield-carnd.svg)](http://www.udacity.com/drive)


Overview
---
The goal of this project is to implement a PID controller. A car in simulation will use this controller to adjust the steering angle around the track. This project strengthens the knowlodge of each component of a PID controller. The criteria found in the project [rubic](https://review.udacity.com/#!/rubrics/1972/view) is key to success. 

<p align="center">
<img width="1000" src="media/title.png"
</p>  
   
PID controller overview
---  

A proportional–integral–derivative controller is one of the most common control loop feedback mechanisms that continuously calculates an error function and applies a correction based on proportional (P), integral (I), and derivative (D) terms.

* Proportional component  depends only on the difference between the set point and the process variable. This difference is referred to as the Error term. We use this error term to know how far we are from the middle of the track.

* Integral component sums the error term over time. This is useful to eliminate any bias or steady-state errors.

* Derivative component causes the output to decrease if the process variable is increasing rapidly. This avoids our vehicle from overshooting the target (i.e: the center of the track).

<p align="center">
<img width="1000" src="media/pid.png"
</p>  
   

Comparing controllers
---
The table below shows 3 different types of controllers.
P controller                |  PD controller		| PID controller
:-------------------------:|:--------------------------:|:--------------------------:
<img src="media/p.gig"/>   | <img src="media/pd.gif"/>  | <img src="media/pid.gif"/>

As we can see, using only the P component results in a very unstable navigation. A pd controller is more stable but fails to correct the bias that causes the car to only drive on the right side of the track. A pid controller provides a smooth navigation alonf the middle of the track.


Intializing coefficients
---
The initial coefficients of each components is essential to start with a smooth navigation. A common way to intialize is using the Twiddle algorithm. For this project, I intialized the coefficients by trial and error.

Results
---
The controller has been correctly implemented with great intial coefficients since the car navigates smoothly from start to finish. 

Improvements
---
Possibly implement the Twiddle algorithm to initialize the coefficients.

End note
---
This project has strengthen my PID controller knowledge and I look forward to reading more about them. 


