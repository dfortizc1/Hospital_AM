/**
 * @file dynamics.h
 * @author Daniel Felipe
 * @date 2020
 * @brief Header containing the functions that return time step and number of reaction. The auxiliar functions for the algorithm are here.
 */


#ifndef DYNAMICS_H
#define DYNAMICS_H

#include <map>
#include <Random64.h>
#include <bases.h>
#include <workers.h>


/* Esta función me dice cuánto tiempo se demora en hacerse una reacción y qué reacción es. */
std::vector<double> contagio(std::map<std::string, group> &Val, std::map<std::string, group> &Vba, Crandom &ran, double t, double* tj);


/* Esta función implementa el método de bisección para hallar el tiempo */
double biseccion(double* A, double t, double Ba1, double Ba2, double Bb, double ranr, double* tj, int n, std::vector<lognormal_d> &dist);


/* Es la función integrada de la prevalencia externa */
double function(double A, double prom, double sigma, double t, double tau);


/* Es la función phi */
double phi(double* A, double* tj, unsigned int n, double Ba1, double Ba2, double Bb, double deltat, double t, std::vector<lognormal_d> &dist);


/* Con esta función identifico a la persona que hico la infección */
int who_infected(group &Pa, group &Pb, group &PTa, group &PTb, group &PTAa, group &PTAb, group &La, group &Lb, group &LTa, group &LTb, group &LTAa, group &LTAb, group &IAa, group &IAb, double cons1, double cons2, Crandom &ran, int alti, double t, double TBa, double TBb);


/* Esta es la función madre para la selección del que infectó */
int selection_infectious(group &Ga, group &Gb, group &Gc, group &Gd, Crandom &ran);


/* Esta es la función gaussiana */
double function_gauss(double x, double A, double mu, double sigma);


void aux_phi_function(double t0, double diff, double &gnum, double &bnum, const double* lim_beta, const double* m_beta, const double* b_beta, const size_t N_beta);


void main_aux_phi_function(double t0, double diff, double &gnum, double &bnum, double t, const double* lim_beta, const double* m_beta, const double* b_beta, const size_t N_beta);


double int_beta_gauss(double t0, double t1, double prom, double sigma, double A, double m, double b);


double int_beta(double t0, double t1, double m, double b);


double function_beta(double x, const double* lim_beta, const double* m_beta, const double* b_beta, size_t N_beta);


size_t index_beta(double x, const double* lim_beta, const size_t N_beta);


#endif /* DYNAMICS_H */
