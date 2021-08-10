#include <uWS/uWS.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "Eigen-3.3/Eigen/Core"
#include "Eigen-3.3/Eigen/QR"
#include "helpers.h"
#include "json.hpp"
#include "spline.h"

// for convenience
using nlohmann::json;
using std::string;
using std::vector;


int main() {
  uWS::Hub h;

  // Load up map values for waypoint's x,y,s and d normalized normal vectors
  vector<double> map_waypoints_x;
  vector<double> map_waypoints_y;
  vector<double> map_waypoints_s;
  vector<double> map_waypoints_dx;
  vector<double> map_waypoints_dy;

  // Waypoint map to read from
  string map_file_ = "../data/highway_map.csv";
  // The max s value before wrapping around the track back to 0
  double max_s = 6945.554;

  std::ifstream in_map_(map_file_.c_str(), std::ifstream::in);

  string line;
  while (getline(in_map_, line)) {
    std::istringstream iss(line);
    double x;
    double y;
    float s;
    float d_x;
    float d_y;
    iss >> x;
    iss >> y;
    iss >> s;
    iss >> d_x;
    iss >> d_y;
    map_waypoints_x.push_back(x);
    map_waypoints_y.push_back(y);
    map_waypoints_s.push_back(s);
    map_waypoints_dx.push_back(d_x);
    map_waypoints_dy.push_back(d_y);
  } 



    int lane = 1;
    double max_speed = 49.5/2.237;      // 49.5 MPH limit converted to [m/s] 
    double safe_distance = 30;          // meters. Minimum front and back distance for car to to take decisions.
    double ref_vel = 0.0;               // [m/s]. End point speed in trajectory vector. Zero since it starts from rest.


  h.onMessage([&map_waypoints_x,&map_waypoints_y,&map_waypoints_s,
               &map_waypoints_dx,&map_waypoints_dy,
                &max_speed,&lane,&safe_distance,&ref_vel,&max_s]
              (uWS::WebSocket<uWS::SERVER> ws, char *data, size_t length,
               uWS::OpCode opCode) {
    // "42" at the staSSrt of the message means there's a websocket message event.
    // The 4 signifies a websocket message
    // The 2 signifies a websocket event

    if (length && length > 2 && data[0] == '4' && data[1] == '2') {

      auto s = hasData(data);

      if (s != "") {
        auto j = json::parse(s);
        
        string event = j[0].get<string>();
        
        if (event == "telemetry") {
          // j[1] is the data JSON object
          
          // Main car's localization Data
          double car_x = j[1]["x"];
          double car_y = j[1]["y"];
          double car_s = j[1]["s"];
          double car_d = j[1]["d"];
          double car_yaw = j[1]["yaw"];
          double car_speed = j[1]["speed"];

          // Previous path data given to the Planner
          auto previous_path_x = j[1]["previous_path_x"];
          auto previous_path_y = j[1]["previous_path_y"];
          // Previous path's end s and d values 
          double end_path_s = j[1]["end_path_s"];
          double end_path_d = j[1]["end_path_d"];

          // Sensor Fusion Data, a list of all other cars on the same side 
          //   of the road.
          auto sensor_fusion = j[1]["sensor_fusion"];

          // Path planner decision from sensor fusion data (raise flags)
          bool too_close = false; // to vehicle ahead of main car (ego)
          double front_car_speed;
          double front_car_distance =  safe_distance +1; // greater than safe_distance
          bool left_lane_free  = true;
          bool right_lane_free = true; 
          double left_lane_closest = 999999.0;
          double right_lane_closest = 999999.0;

          
          for(int i=0; i<sensor_fusion.size(); i++){ 
            // Read other car's data from sensor fusion update
            float d = sensor_fusion[i][6]; // d value in Frenet coordinates
            double vx = sensor_fusion[i][3]; // Vx velocity vector
            double vy = sensor_fusion[i][4]; // Vy velocity vector
            double check_speed = sqrt(vx*vx+vy*vy);  // magnitude of velocity
            double check_car_s = sensor_fusion[i][5]; // s value in Frenet coordinates
            double check_distance = check_car_s-car_s; // distance between ego and other car
            
            
            // Check for vehicles in front of car lane
            if(d >(2+4*lane-2) && d<(2+4*lane+2)) { // note lanes are 4m wide, range takes into account vehicles that are off centre of their lane
              if ( check_car_s>car_s && check_distance<safe_distance ){ //if car is ahead and too close
                too_close = true; 
                front_car_speed = check_speed;
                front_car_distance = check_distance; 
              }
            }
            // Check for cars in right lane (front and back)
            if (lane == 2) right_lane_free = false; 
            
            else{
              if( d >(2+4*(lane+1)-2) && d<(2+4*(lane+1)+2) ){ // check if vehicle is in right lane
                if (check_distance > 0 && check_distance < right_lane_closest){
                  right_lane_closest = check_distance; // keeping track of closest vehicle in the right lane
                  std::cout << "Right: " << right_lane_closest << std::endl;
                }
                if ( check_distance < safe_distance && check_distance > -5){
                  right_lane_free = false;
                }
              }
            }

            // Check for cars in left lane (front and back)
            if (lane == 0) left_lane_free = false;
            
            else{
              if ( d>(2+4*(lane-1)-2) && d<(2+4*(lane-1)+2) ){ // check if vehicle is in right left
                if (check_distance > 0 && check_distance < left_lane_closest){
                  left_lane_closest = check_distance; // keeping track of closest vehicle in the left lane
                  std::cout << "Left: " << left_lane_closest << std::endl;
                }
                if ( check_distance < safe_distance && check_distance > -5){
                  left_lane_free = false;
                }
              }
            }
          }

          double target_speed; 
          if (lane == 0 && right_lane_free && right_lane_closest > 40){ // stay out of left lane as much as possible
            lane = 1;
            std::cout << "Going Center!" << std::endl;
          }

          else if (lane == 2 && left_lane_free && left_lane_closest > 40){ // stay in middle lane to have more opportunities of overtaking
            lane = 1;
            std::cout << "Going Center!" << std::endl;
          }

          if (too_close){
            target_speed = front_car_speed;
            if (right_lane_free && left_lane_free){ //If both side lanes are free, go in the one with the nearest car further away
              std::cout << "RIGHT: " << right_lane_closest << " LEFT: " << left_lane_closest << std::endl;
              if(right_lane_closest > left_lane_closest){
                std::cout << "Going Right!" << std::endl;
                lane = 2;
              }
              else{
                std::cout << "Going Left!" << std::endl;
                lane = 0;
              }
              target_speed = max_speed;
            }
            else if (left_lane_free){
              lane-=1;
              target_speed = max_speed;
            }
            else if (right_lane_free){
              lane+=1;
              target_speed = max_speed;
            }
            
          }
          else{
            target_speed = max_speed;
          }

          vector<double> ptsx;  //vector of x points for Step Procedure
          vector<double> ptsy;  //vector of y points for Step Procedure
            
          double ref_x;
          double ref_y;
          double ref_x_prev;
          double ref_y_prev;
          double ref_yaw; 

          // Collect previous and current 'pts(x,y)' waypoints
          int prev_size = previous_path_x.size(); 
          if(prev_size < 2) {
            // reference x,y, yaw states
            ref_x = car_x;
            ref_y = car_y;
            ref_yaw = deg2rad(car_yaw);

            ref_x_prev = ref_x - cos(car_yaw);
            ref_y_prev = ref_y - sin(car_yaw); 

            ptsx.push_back(ref_x_prev);
            ptsx.push_back(ref_x); 
            
            ptsy.push_back(ref_y_prev);
            ptsy.push_back(ref_y); 

            end_path_s = car_s; //assign to current position since there is no last position yet
          } 
          else{
            ref_x = previous_path_x[prev_size-1];
            ref_y = previous_path_y[prev_size-1]; 

            ref_x_prev = previous_path_x[prev_size-2];
            ref_y_prev = previous_path_y[prev_size-2];
            ref_yaw = atan2(ref_y-ref_y_prev, ref_x-ref_x_prev); 

            ptsx.push_back(ref_x_prev);
            ptsx.push_back(ref_x); 
            
            ptsy.push_back(ref_y_prev);
            ptsy.push_back(ref_y); 
          } 

          // Collect {30m,60m,90m} 'pts(x,y)' waypoints
          
          vector<double> next_wp0 = getXY(end_path_s+30, (2+4*lane), map_waypoints_s, map_waypoints_x, map_waypoints_y);
          vector<double> next_wp1 = getXY(end_path_s+60, (2+4*lane), map_waypoints_s, map_waypoints_x, map_waypoints_y);
          vector<double> next_wp2 = getXY(end_path_s+90, (2+4*lane), map_waypoints_s, map_waypoints_x, map_waypoints_y); 

          // push x points
          ptsx.push_back(next_wp0[0]);
          ptsx.push_back(next_wp1[0]);
          ptsx.push_back(next_wp2[0]); 
		      // push y points
          ptsy.push_back(next_wp0[1]);
          ptsy.push_back(next_wp1[1]);
          ptsy.push_back(next_wp2[1]);

          //Transform coordinates to car coordinate system, the last point of the previous path is set to 0,0 the orgin and its angles are 0 degrees
          for (int i = 0; i < ptsx.size(); i++ ) { 
            double shift_x = ptsx[i]-ref_x;
            double shift_y = ptsy[i]-ref_y; 

            ptsx[i] = (shift_x *cos(0-ref_yaw) - shift_y*sin(0-ref_yaw));
            ptsy[i] = (shift_x *sin(0-ref_yaw) + shift_y*cos(0-ref_yaw));
          }

          tk::spline s;
          // set (x,y) points to the spline, which include 5 (x,y) points as determined above, refer to as "anchor points"
          s.set_points(ptsx,ptsy);

          // path vector that will be sent back to simulator
          vector<double> next_x_vals;
          vector<double> next_y_vals;

          // Here we fill up our path planner with the remaining points to ensure we always have 50 points
            
          for (int i=1; i<previous_path_x.size(); i++){
            next_x_vals.push_back(previous_path_x[i]);
            next_y_vals.push_back(previous_path_y[i]);
          } 

          // Create Target/Goal Point (from car's end point reference)
          double target_x = 30.0; // horizon [m] for spline distance line
          double target_y = s(target_x); // y coordinate determined from spline function s(x)
          double target_dist = sqrt((target_x)*(target_x)+(target_y)*(target_y)); // euclidean distance
          
          // Refill empty 'next_val' slots with new points that the car will visit every .02 seconds
          double x_local = 0.0; // (x,y) end point of trajectory in local car's end point reference
          double y_local;
          for (int i=1; i<=(50-previous_path_x.size()); i++){ 
            if ( ref_vel > target_speed || front_car_distance < safe_distance/2 )
            {
              ref_vel -= 0.1;  //[m/s] equivalent to acceleration of -5 [m/s^2]
            }
            else if ( ref_vel < target_speed )
            {
              ref_vel += 0.1;  //[m/s] equivalent to acceleration of 5 [m/s^2]
            }
            ref_vel = std::max(0.00001,ref_vel);   // prevents from going backwards
            // N is the number of points along our spline
            double N = (target_dist/(0.02*ref_vel)); // spacing between points on line so that car travels at ref_vel
            x_local = x_local + (target_x)/N; 
            y_local = s(x_local); // y coordinate determined from spline function s(x)
            
            // Recall we are in car (local) coordinates, need to convert back to global (map) coordinates. 
            double x_map = x_local*cos(ref_yaw) - y_local*sin(ref_yaw) + ref_x; //rotate back to map
            double y_map = x_local*sin(ref_yaw) + y_local*cos(ref_yaw) + ref_y; 

            next_x_vals.push_back(x_map);
            next_y_vals.push_back(y_map); 
          }

          // Send data back to Simulator
          json msgJson;
          msgJson["next_x"] = next_x_vals;
          msgJson["next_y"] = next_y_vals;
          auto msg = "42[\"control\","+ msgJson.dump()+"]";
          ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);

        } 
      } 

      else
      {
        // Manual driving
        std::string msg = "42[\"manual\",{}]";
        ws.send(msg.data(), msg.length(), uWS::OpCode::TEXT);
      }
    }  // end websocket if
  }); // end h.onMessage

  h.onConnection([&h](uWS::WebSocket<uWS::SERVER> ws, uWS::HttpRequest req) {
    std::cout << "Connected!!!" << std::endl;
  });

  h.onDisconnection([&h](uWS::WebSocket<uWS::SERVER> ws, int code,
                         char *message, size_t length) {
    ws.close();
    std::cout << "Disconnected" << std::endl;
  });

  int port = 4567;
  if (h.listen(port)) {
    std::cout << "Listening to port " << port << std::endl;
  } else {
    std::cerr << "Failed to listen to port" << std::endl;
    return -1;
  }
  
  h.run();
}