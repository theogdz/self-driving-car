/**
 * particle_filter.cpp
 *
 * Created on: Dec 12, 2016
 * Author: Tiffany Huang
 */

#include "particle_filter.h"

#include <math.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "helper_functions.h"

using std::string;
using std::vector;
using std::normal_distribution;
static std::default_random_engine gen;


void ParticleFilter::init(double x, double y, double theta, double std[]) {
  num_particles = 100;  // TODO: Set the number of particles

   normal_distribution<double> init_x(x, std[0]);
   normal_distribution<double> init_y(y, std[1]);
   normal_distribution<double> init_theta(theta, std[2]);
   
   for (int i = 0; i < num_particles; i++){

     Particle p;
     p.id = int(i);
     p.weight = 1.0;
     p.x = init_x(gen);
     p.y = init_y(gen);
     p.theta = init_theta(gen);
     particles.push_back(p);
     weights.push_back(p.weight);
   }

   is_initialized = true;

}

void ParticleFilter::prediction(double delta_t, double std_pos[], 
                                double velocity, double yaw_rate) {
    for (int i = 0; i < num_particles; i++){
      double pred_theta = particles[i].theta + (yaw_rate * delta_t); 

     if (fabs(yaw_rate) < 0.00001) {  //If vehicle is going straight
        particles[i].x += velocity * delta_t * cos(particles[i].theta);
        particles[i].y += velocity * delta_t * sin(particles[i].theta);
      }

      else{ //If the vehicle turns
        particles[i].x += (velocity / yaw_rate) * (sin(pred_theta) - sin(particles[i].theta));
        particles[i].y += (velocity / yaw_rate) * (cos(particles[i].theta) - cos(pred_theta));
        particles[i].theta = pred_theta;
      }

      normal_distribution<double> norm_x(particles[i].x, std_pos[0]);
      normal_distribution<double> norm_y(particles[i].y, std_pos[1]);
      normal_distribution<double> norm_theta(particles[i].theta, std_pos[2]);

      particles[i].x = norm_x(gen);
      particles[i].y = norm_y(gen);
      particles[i].theta = norm_theta(gen);
    }

}

// Update the weights of each particle using a multi-variate Gaussian distribution
double multiv_prob_gaussian(double sig_x, double sig_y, double x_obs, double y_obs,
                   double mu_x, double mu_y) {
  
  // calculate normalization term
  double gaussian_norm;
  gaussian_norm = 1 / (2 * M_PI * sig_x * sig_y);

  // calculate exponent
  double exponent;
  exponent = (pow(x_obs - mu_x, 2) / (2 * pow(sig_x, 2)))
               + (pow(y_obs - mu_y, 2) / (2 * pow(sig_y, 2)));

  // calculate weight using normalization terms and exponent
  double weight;
  weight = gaussian_norm * exp(-exponent);
    
  return weight;
}


void ParticleFilter::updateWeights(double sensor_range, double std_landmark[], 
                                   const vector<LandmarkObs> &observations, 
                                   const Map &map_landmarks) {
for (unsigned int i = 0; i < particles.size(); ++i)
    {
    Particle particle = particles[i];
    double prob = 1.0;

        for (unsigned int j = 0; j < observations.size(); j++)
        {
            // Homogenous Transformation 
            double x_map = particle.x + (cos(particle.theta) * observations[j].x) - (sin(particle.theta) * observations[j].y);
            double y_map = particle.y + (sin(particle.theta) * observations[j].x) + (cos(particle.theta) * observations[j].y);

            std::vector<Map::single_landmark_s> landmark_list = map_landmarks.landmark_list;
            double land_x; // x location of landmark
            double land_y; // y location of landmark
            double max_val = 2 * sensor_range;
            for (unsigned int k = 0; k < landmark_list.size(); k++)
            {
                // Calculate distance between particle and landmarks
                double local_land_x = landmark_list[k].x_f;
                double local_land_y = landmark_list[k].y_f;
              	// dist function
                double distance = dist(x_map, y_map, local_land_x, local_land_y);
                if ((distance <= sensor_range) && (distance <= max_val))
                {
                  // Calculate multivariate Gaussian normal distribution
                  land_x = local_land_x;
                  land_y = local_land_y;
                  max_val = distance;
                  prob = multiv_prob_gaussian(std_landmark[0], std_landmark[1], x_map, y_map, land_x, land_y);
                  particles[i].weight = prob;
                  weights[i] = prob;
                }
            }
        }
    }
}


void ParticleFilter::resample() {
    std::discrete_distribution<> d(weights.begin(), weights.end());
    std::vector<Particle> resampled_particles;

    for (int n = 0; n < num_particles; ++n) 
    {
        Particle particle = particles[d(gen)];
        resampled_particles.push_back(particle);
    }
    particles = resampled_particles;
}


void ParticleFilter::SetAssociations(Particle& particle, 
                                     const vector<int>& associations, 
                                     const vector<double>& sense_x, 
                                     const vector<double>& sense_y) {
  // particle: the particle to which assign each listed association, 
  //   and association's (x,y) world coordinates mapping
  // associations: The landmark id that goes along with each listed association
  // sense_x: the associations x mapping already converted to world coordinates
  // sense_y: the associations y mapping already converted to world coordinates
  particle.associations= associations;
  particle.sense_x = sense_x;
  particle.sense_y = sense_y;
}

string ParticleFilter::getAssociations(Particle best) {
  vector<int> v = best.associations;
  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<int>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}

string ParticleFilter::getSenseCoord(Particle best, string coord) {
  vector<double> v;

  if (coord == "X") {
    v = best.sense_x;
  } else {
    v = best.sense_y;
  }

  std::stringstream ss;
  copy(v.begin(), v.end(), std::ostream_iterator<float>(ss, " "));
  string s = ss.str();
  s = s.substr(0, s.length()-1);  // get rid of the trailing space
  return s;
}