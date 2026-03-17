#include <stdio.h>
#include <math.h>

const double dt = 0.001;
const double R = 0.4;
const double C = 0.05;

double VC(double t)
{
    return 1.0 - exp(-50.0 * t);  // Correct analytic solution
}

double DE(double x, double u)
{
    return (u - x) / (R * C);  // Correct differential equation
}

double Euler(double (*f)(double, double), double x, double u)
{
    return x + dt * f(x, u);  // Euler's method implementation
}

int main(void)
{
    double t = 0, u = 1.0, x = 0.0;
    FILE *fp;

    fp = fopen("out.dat", "w");
    if (fp == NULL) {
        printf("File error\n");
        return -1;
    }
    
    for (t = 0; t < 5; t += dt) {
        fprintf(fp, "%e\t%e\t%e\n", t, x, VC(t));
        x = Euler(DE, x, u);
    }
    
    fclose(fp);
    return 0;
}