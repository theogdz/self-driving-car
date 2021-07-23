#include "FusionEKF.h"
#include <iostream>
#include "Eigen/Dense"
#include "tools.h"

using Eigen::MatrixXd;
using Eigen::VectorXd;
using std::cout;
using std::endl;
using std::vector;

/**
 * Constructor.
 */
FusionEKF::FusionEKF() {
  is_initialized_ = false;

  previous_timestamp_ = 0;

  // initializing matrices
  R_laser_  = MatrixXd(2, 2);
  R_radar_  = MatrixXd(3, 3);
  H_laser_  = MatrixXd(2, 4);
  ekf_.H_ = MatrixXd(4, 4);
  Hj_       = MatrixXd(3, 4);
  ekf_.P_   = MatrixXd(4,4);
  ekf_.F_   = MatrixXd(4,4);
  ekf_.Q_   = MatrixXd(4,4);
  ekf_.x_   = VectorXd(4);

  //measurement covariance matrix - laser
  R_laser_ << 0.0225, 0     ,
              0     , 0.0225;

  //measurement covariance matrix - radar
  R_radar_ << 0.09, 0     , 0   ,
              0   , 0.0009, 0   ,
              0   , 0     , 0.09;

  /**
   * TODO: Finish initializing the FusionEKF.
   * TODO: Set the process and measurement noises
   */

  //measurement covariance matrix - lidar
  H_laser_ << 1, 0, 0, 0,
              0, 1, 0, 0;
  
  // jacobian matrix - radar
  // Hj_ << 0, 0, 0, 0,
  //        0, 0, 0, 0,
  //        0, 0, 0, 0;

  // Initial state covariance matrix P
  ekf_.P_ << 1,    0,    0,    0,
             0,    1,    0,    0,
             0,    0, 1000,    0,
             0,    0,    0, 1000;
  
  // Initial transition matrix F
  ekf_.F_ <<  1, 0, 1, 0,
              0, 1, 0, 1,
              0, 0, 1, 0,
              0, 0, 0, 1;

  noise_ax = 9;
  noise_ay = 9;


}

/**
 * Destructor.
 */
FusionEKF::~FusionEKF() {}

void FusionEKF::ProcessMeasurement(const MeasurementPackage &measurement_pack) {
  /**
   * Initialization
   */
  if (!is_initialized_) {

    if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
      // TODO: Convert radar from polar to cartesian coordinates 
      //         and initialize state.
      float rho = measurement_pack.raw_measurements_(0);
      float phi = measurement_pack.raw_measurements_(1);
      float rho_dot = measurement_pack.raw_measurements_(2);

      float px = rho * cos(phi);
      float py = rho * sin(phi);
      float vx = rho_dot * cos(phi);
      float vy = rho_dot * sin(phi);

      ekf_.x_ << px, py, vx, vy;
    }
    else if (measurement_pack.sensor_type_ == MeasurementPackage::LASER) {
      // TODO: Initialize state.
      float px = measurement_pack.raw_measurements_(0);
      float py = measurement_pack.raw_measurements_(1);

      ekf_.x_ << px, py, 0, 0; 

    }

    // update time
    previous_timestamp_ = measurement_pack.timestamp_;

    // done initializing, no need to predict or update
    is_initialized_ = true;
    return;
  }
  /**
   * Prediction
   */

  float dt = (measurement_pack.timestamp_ - previous_timestamp_) / 1000000.0;
  

  previous_timestamp_ = measurement_pack.timestamp_;

  ekf_.F_(0,2) = dt;
  ekf_.F_(1,3) = dt;

  float dt_2  = dt * dt;
  float dt_3 = dt_2 * dt;
  float dt_4 = dt_3 * dt;

  ekf_.Q_ <<  dt_4 / 4 * noise_ax , 0                  , dt_3 / 2 * noise_ax, 0                   ,
              0                   , dt_4 / 4 * noise_ay, 0                  , dt_3 / 2 * noise_ay ,
              dt_3 / 2 * noise_ax , 0                  , dt_2 *noise_ax     , 0                   ,
              0                   , dt_3 / 2 * noise_ay, 0                  , dt_2 * noise_ay     ;
  ekf_.Predict();

  /**
   * Update
   */

  if (measurement_pack.sensor_type_ == MeasurementPackage::RADAR) {
    // TODO: Radar updates
    Hj_ = tools.CalculateJacobian(ekf_.x_);
    ekf_.Init(ekf_.x_, ekf_.P_, ekf_.F_, Hj_, R_radar_, ekf_.Q_);
    ekf_.UpdateEKF(measurement_pack.raw_measurements_);


  } else {
    ekf_.Init(ekf_.x_, ekf_.P_, ekf_.F_, H_laser_, R_laser_, ekf_.Q_);
    ekf_.Update(measurement_pack.raw_measurements_);

  }

  // print the output
  cout << "x_ = " << ekf_.x_ << endl;
  cout << "P_ = " << ekf_.P_ << endl;
}
