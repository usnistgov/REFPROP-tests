/*

Some of these tests are based on the tests implemented in the CoolProp program (www.coolprop.org)

*/

#include <cstdlib>

#include <unordered_map>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
using Catch::Matchers::WithinAbsMatcher;
using Catch::Matchers::WithinRelMatcher;

#include "REFPROPtests/baseclasses.h"

static double fudge = 1000;

// A structure to hold the values for one validation call
struct vel
{
public:
    std::string in1, in2, out, fluid;
    double v1, v2, tol, expected;
    vel(std::string fluid, std::string in1, double v1, std::string in2, double v2, std::string out, double expected, double tol)
    {
        this->fluid = fluid;
        this->in1 = in1; this->in2 = in2;  this->v1 = v1; this->v2 = v2; 
        this->out = out; this->expected = expected;
        this->tol = tol;
    };
};

std::vector<vel> transport_validation_data = {
    // From Vogel, JPCRD, 2016
    {"Propane", "T", 90, "Dmass", 730, "V", 8010.968e-6, 1e-3},
    {"Propane", "T", 300, "Dmass", 1, "V", 8.174374e-6, 5e-3},
    {"Propane", "T", 300, "Dmass", 20, "V", 8.230795e-6, 5e-3 },
    {"Propane", "T", 300, "Dmass", 490, "V", 95.631e-6, 5e-3},
    {"Propane", "T", 300, "Dmass", 600, "V", 223.6002e-6,1e-3},
    { "Propane", "T", 369.89, "Dmass", 220.478, "V", 25.70313e-6,1e-3 },
    { "Propane", "T", 375, "Dmass", 1, "V", 10.15009e-6,1e-3 },
    { "Propane", "T", 375, "Dmass", 100, "V", 13.07220e-6,1e-3 },
    { "Propane", "T", 375, "Dmass", 550, "V", 146.8987e-6,1e-3 },
    { "Propane", "T", 500, "Dmass", 1, "V", 13.26285e-6,1e-3 },
    { "Propane", "T", 500, "Dmass", 100, "V", 16.85501e-6,1e-3 },
    { "Propane", "T", 500, "Dmass", 450, "V", 77.67365e-6,1e-3 },
    { "Propane", "T", 650, "Dmass", 1, "V", 16.63508e-6,1e-3 },
    { "Propane", "T", 650, "Dmass", 100, "V", 20.72894e-6,1e-3 },
    { "Propane", "T", 650, "Dmass", 400, "V", 62.40780e-6,1e-3 },


    // Huber, FPE, 2004
    {"n-Octane", "T", 300, "Dmolar", 6177.2, "V", 553.60e-6, 1e-3},
    {"n-Nonane", "T", 300, "Dmolar", 5619.1, "V", 709.53e-6, 1e-3},
    {"n-Decane", "T", 300, "Dmolar", 5150.4, "V", 926.44e-6, 1e-3},

    // Huber, Energy & Fuels, 2004
    {"n-Dodecane", "T", 300, "Dmolar", 4411.5, "V", 1484.8e-6, 1e-3},
    {"n-Dodecane", "T", 500, "Dmolar", 3444.7, "V", 183.76e-6, 1e-3},

    // Huber, I&ECR, 2006
    {"R125", "T", 300, "Dmolar", 10596.9998, "V", 177.37e-6, 1e-3},
    {"R125", "T", 400, "Dmolar", 30.631, "V", 17.070e-6, 1e-3},

    // From REFPROP 9.1 since Huber I&ECR 2003 does not provide validation data
    {"R134a", "T", 185, "Q", 0, "V", 0.0012698376398294414, 1e-3},
    {"R134a", "T", 185, "Q", 1, "V", 7.4290821400170869e-006, 1e-3},
    {"R134a", "T", 360, "Q", 0, "V", 7.8146319978982133e-005, 1e-3},
    {"R134a", "T", 360, "Q", 1, "V", 1.7140264998576107e-005, 1e-3},

    // From REFPROP 9.1 since Kiselev, IECR, 2005 does not provide validation data
    {"Ethanol", "T", 300, "Q", 0, "V", 0.0010439017679191723, 1e-3},
    {"Ethanol", "T", 300, "Q", 1, "V", 8.8293820936046416e-006, 1e-3},
    {"Ethanol", "T", 500, "Q", 0, "V", 6.0979347125450671e-005, 1e-3},
    {"Ethanol", "T", 500, "Q", 1, "V", 1.7229157141572511e-005, 1e-3},

    // From CoolProp v5 implementation of correlation - more or less agrees with REFPROP
    // Errata in BibTeX File
    {"Hydrogen", "T", 35, "Dmass", 100, "V", 5.47889e-005, 1e-3},

    // From Meng 2012 experimental data (note erratum in BibTeX file)
    {"DimethylEther", "T", 253.146, "Dmass", 734.28, "V", 0.20444e-3, 3e-3},
    {"DimethylEther", "T", 373.132, "Dmass", 613.78, "V", 0.09991e-3, 3e-3},

    // From Monogenidou, 2018
    {"Ammonia", "T", 200, "Q", 1, "V", 6.95e-6, 1e-3},
    {"Ammonia", "T", 200, "Q", 0, "V", 516.02e-6, 1e-3},
    {"Ammonia", "T", 380, "Q", 1, "V", 14.03e-6, 1e-3},
    {"Ammonia", "T", 380, "Q", 0, "V", 53.95e-6, 1e-3},
    
    // From Sotiriadou, IJT, 2023
    {"ETHANOL", "T", 300, "Dmass", 0, "V", 8.9893e-6, 1e-3},
    {"ETHANOL", "T", 300, "Dmass", 850, "V", 1682.72e-6, 1e-3},

    // From Huber, IJT, 2024
    {"Nitrogen", "T", 90, "Dmass", 756, "V", 108.42550781e-6, 1e-4},
    {"Nitrogen", "T", 300, "Dmass", 560, "V", 50.59605975e-6, 1e-4},
    
    // From Lemmon and Jacobsen, JPCRD, 2004
//    {"Nitrogen", "T", 100, "Dmolar", 1e-14, "V", 6.90349e-6, 1e-3},
//    {"Nitrogen", "T", 300, "Dmolar", 1e-14, "V", 17.8771e-6, 1e-3},
//    {"Nitrogen", "T", 100, "Dmolar", 25000, "V", 79.7418e-6, 1e-3},
//    {"Nitrogen", "T", 200, "Dmolar", 10000, "V", 21.0810e-6, 1e-3},
//    {"Nitrogen", "T", 300, "Dmolar", 5000, "V", 20.7430e-6, 1e-3},
//    {"Nitrogen", "T", 126.195, "Dmolar", 11180, "V", 18.2978e-6, 1e-3},
    {"Argon", "T", 100, "Dmolar", 1e-14, "V", 8.18940e-6, 1e-3},
    {"Argon", "T", 300, "Dmolar", 1e-14, "V", 22.7241e-6, 1e-3},
    {"Argon", "T", 100, "Dmolar", 33000, "V", 184.232e-6, 1e-3},
    {"Argon", "T", 200, "Dmolar", 10000, "V", 25.5662e-6, 1e-3},
    {"Argon", "T", 300, "Dmolar", 5000, "V", 26.3706e-6, 1e-3},
    {"Argon", "T", 150.69, "Dmolar", 13400, "V", 27.6101e-6, 1e-3},
    {"Oxygen", "T", 100, "Dmolar", 1e-14, "V", 7.70243e-6, 1e-3},
    {"Oxygen", "T", 300, "Dmolar", 1e-14, "V", 20.6307e-6, 1e-3},
    {"Oxygen", "T", 100, "Dmolar", 35000, "V", 172.136e-6, 1e-3},
    {"Oxygen", "T", 200, "Dmolar", 10000, "V", 22.4445e-6, 1e-3},
    {"Oxygen", "T", 300, "Dmolar", 5000, "V", 23.7577e-6, 1e-3},
    {"Oxygen", "T", 154.6, "Dmolar", 13600, "V", 24.7898e-6, 1e-3},
    {"AIR.PPF", "T", 100, "Dmolar", 1e-14, "V", 7.09559e-6, 1e-3},
    {"AIR.PPF", "T", 300, "Dmolar", 1e-14, "V", 18.5230e-6, 1e-3},
    {"AIR.PPF", "T", 100, "Dmolar", 28000, "V", 107.923e-6, 1e-3},
    {"AIR.PPF", "T", 200, "Dmolar", 10000, "V", 21.1392e-6, 1e-3},
    {"AIR.PPF", "T", 300, "Dmolar", 5000, "V", 21.3241e-6, 1e-3},
    {"AIR.PPF", "T", 132.64, "Dmolar", 10400, "V", 17.7623e-6, 1e-3},

    // From Michailidou, JPCRD, 2013
    {"Hexane", "T", 250, "Dmass", 1e-14, "V", 5.2584e-6, 1e-3},
    {"Hexane", "T", 400, "Dmass", 1e-14, "V", 8.4149e-6, 1e-3},
    {"Hexane", "T", 550, "Dmass", 1e-14, "V", 11.442e-6, 1e-3},
    {"Hexane", "T", 250, "Dmass", 700, "V", 528.2e-6, 1e-3},
    {"Hexane", "T", 400, "Dmass", 600, "V", 177.62e-6, 1e-3},
    {"Hexane", "T", 550, "Dmass", 500, "V", 95.002e-6, 1e-3},

    // From Assael, JPCRD, 2014
    {"Heptane", "T", 250, "Dmass", 1e-14, "V", 4.9717e-6, 1e-3},
    {"Heptane", "T", 400, "Dmass", 1e-14, "V", 7.8361e-6, 1e-3},
    {"Heptane", "T", 550, "Dmass", 1e-14, "V", 10.7394e-6, 1e-3},
    {"Heptane", "T", 250, "Dmass", 720, "V", 725.69e-6, 1e-3},
    {"Heptane", "T", 400, "Dmass", 600, "V", 175.94e-6, 1e-3},
    {"Heptane", "T", 550, "Dmass", 500, "V", 95.105e-6, 1e-3},

    // From Laesecke, JPCRD, 2017
    {"CO2", "T", 110, "Dmass", 0.0, "V", 0.0053757e-3, 1e-3},
    {"CO2", "T", 2000, "Dmass", 0.0, "V", 0.066079e-3, 1e-3},
    {"CO2", "T", 10000, "Dmass", 0.0, "V", 0.17620e-3, 1e-3},
    {"CO2", "T", 220, "Dmass", 3.0, "V", 0.011104e-3, 1e-3}, // no critical enhancement
    {"CO2", "T", 225, "Dmass", 1150, "V", 0.22218e-3, 1e-3},
    {"CO2", "T", 300, "Dmass", 65, "V", 0.015563e-3, 1e-3},
    {"CO2", "T", 300, "Dmass", 1400, "V", 0.50594e-3, 1e-3},
    {"CO2", "T", 700, "Dmass", 100, "V", 0.033112e-3, 1e-3},
    {"CO2", "T", 700, "Dmass", 1200, "V", 0.22980e-3, 1e-3},


    // Tanaka, IJT, 1996
    {"R123", "T", 265, "Dmass", 1545.8, "V", 627.1e-6, 1e-3},
    {"R123", "T", 265, "Dmass", 1.614, "V", 9.534e-6, 1e-3},
    {"R123", "T", 415, "Dmass", 1079.4, "V", 121.3e-6, 1e-3},
    {"R123", "T", 415, "Dmass", 118.9, "V", 15.82e-6, 1e-3},

    // Krauss, IJT, 1996
    {"R152A", "T", 242, "Dmass", 1025.5, "V", 347.3e-6, 1e-3},
    {"R152A", "T", 242, "Dmass", 2.4868, "V", 8.174e-6, 1e-3},
    {"R152A", "T", 384, "Dmass", 504.51, "V", 43.29e-6, 5e-3},
    {"R152A", "T", 384, "Dmass", 239.35, "V", 21.01e-6, 10e-3},

    // Huber, JPCRD, 2008 and IAPWS
    {"Water", "T", 298.15, "Dmass", 998, "V", 889.735100e-6, 1e-7},
    {"Water", "T", 298.15, "Dmass", 1200, "V", 1437.649467e-6, 1e-7},
    {"Water", "T", 373.15, "Dmass", 1000, "V", 307.883622e-6, 1e-7},
    {"Water", "T", 433.15, "Dmass", 1, "V", 14.538324e-6, 1e-7},
    {"Water", "T", 433.15, "Dmass", 1000, "V", 217.685358e-6, 1e-7},
    {"Water", "T", 873.15, "Dmass", 1, "V", 32.619287e-6, 1e-7},
    {"Water", "T", 873.15, "Dmass", 100, "V", 35.802262e-6, 1e-7},
    {"Water", "T", 873.15, "Dmass", 600, "V", 77.430195e-6, 1e-7},
    {"Water", "T", 1173.15, "Dmass", 1, "V", 44.217245e-6, 1e-7},
    {"Water", "T", 1173.15, "Dmass", 100, "V", 47.640433e-6, 1e-7},
    {"Water", "T", 1173.15, "Dmass", 400, "V", 64.154608e-6, 1e-7},
    {"Water", "T", 647.35, "Dmass", 122, "V", 25.520677e-6, 1e-7},
    {"Water", "T", 647.35, "Dmass", 222, "V", 31.337589e-6, 1e-7},
    {"Water", "T", 647.35, "Dmass", 272, "V", 36.228143e-6, 1e-7},
    {"Water", "T", 647.35, "Dmass", 322, "V", 42.961579e-6, 1e-7},
    {"Water", "T", 647.35, "Dmass", 372, "V", 45.688204e-6, 1e-7},
    {"Water", "T", 647.35, "Dmass", 422, "V", 49.436256e-6, 1e-7},

    // Quinones-Cisneros, JPCRD, 2012
    {"SF6", "T", 300, "Dmass", 1e-14, "V", 15.2887e-6, 1e-4},
    {"SF6", "T", 300, "Dmass", 5.92, "V", 15.3043e-6, 1e-4},
    {"SF6", "T", 300, "Dmass", 1345.1, "V", 117.417e-6, 1e-4},
    {"SF6", "T", 400, "Dmass", 1e-14, "V", 19.6796e-6, 1e-4},
    {"SF6", "T", 400, "Dmass", 278.47, "V", 24.4272e-6, 1e-4},
    {"SF6", "T", 400, "Dmass", 1123.8, "V", 84.7835e-6, 1e-4},

    //// Quinones-Cisneros, JCED, 2012, data from validation
    //{"H2S", "T", 200, "P", 1000e5, "V", 0.000460287, 1e-3},
    //{"H2S", "T", 200, "P", 0.251702e5, "V", 8.02322E-06, 1e-3},
    //{"H2S", "T", 596.961, "P", 1000e5, "V", 6.94741E-05, 1e-3},
    //{"H2S", "T", 596.961, "P", 1e5, "V", 2.38654E-05, 1e-3},

    // Geller, Purdue Conference, 2000
    //{"R410A", "T", 243.15, "Q", 0, "V", 238.61e-6, 5e-2},
    //{"R410A", "T", 243.15, "Q", 1, "V", 10.37e-6, 5e-2},
    //{"R410A", "T", 333.15, "Q", 0, "V", 70.71e-6, 5e-2},
    //{"R410A", "T", 333.15, "Q", 1, "V", 19.19e-6, 5e-2},
    //{"R407C", "T", 243.15, "Q", 0, "V", 304.18e-6, 1e-2},
    //{"R407C", "T", 243.15, "Q", 1, "V", 9.83e-6, 1e-2},
    //{"R407C", "T", 333.15, "Q", 0, "V", 95.96e-6, 1e-2},
    //{"R407C", "T", 333.15, "Q", 1, "V", 16.38e-6, 1e-2},
    //{"R404A", "T", 243.15, "Q", 0, "V", 264.67e-6, 1e-2},
    //{"R404A", "T", 243.15, "Q", 1, "V", 10.13e-6, 1e-2},
    //{"R404A", "T", 333.15, "Q", 0, "V", 73.92e-6, 1e-2},
    //{"R404A", "T", 333.15, "Q", 1, "V", 18.56e-6, 1e-2},
    //{"R507A", "T", 243.15, "Q", 0, "V", 284.59e-6, 3e-2},
    //{"R507A", "T", 243.15, "Q", 1, "V", 9.83e-6, 1e-2},
    //{"R507A", "T", 333.15, "Q", 0, "V", 74.37e-6, 1e-2},
    //{"R507A", "T", 333.15, "Q", 1, "V", 19.35e-6, 1e-2},

    // From Arp, NIST, 1998
    {"Helium", "T", 3.6, "P", 0.180e6, "V", 3.745e-6, 1e-2},
    {"Helium", "T", 50, "P", 0.180e6, "V", 6.376e-6, 1e-2},
    {"Helium", "T", 400, "P", 0.180e6, "V", 24.29e-6, 1e-2},

    // From Herrmann, JPCRD, 2018
    {"ETHANE", "T", 100, "Dmass", 650, "V", 1050.774e-6, 1e-6},
    {"ETHANE", "T", 300, "Dmass", 0, "V", 9.285785e-6, 1e-6},
    {"ETHANE", "T", 300, "Dmass", 1, "V", 9.293015e-6, 1e-6},
    {"ETHANE", "T", 300, "Dmass", 100, "V", 12.55151e-6, 1e-6},
    {"ETHANE", "T", 300, "Dmass", 500, "V", 114.3985e-6, 1e-6},
    {"ETHANE", "T", 305.322, "Dmass", 206.18,"V", 22.63285e-6, 1e-6},
    {"ETHANE", "T", 310, "Dmass", 1, "V", 9.592824e-6, 1e-6},
    {"ETHANE", "T", 310, "Dmass", 100, "V", 12.88843e-6, 1e-6},
    {"ETHANE", "T", 310, "Dmass", 500, "V", 114.8683e-6, 1e-6},
    {"ETHANE", "T", 500, "Dmass", 1, "V", 14.85686e-6, 1e-6},
    {"ETHANE", "T", 500, "Dmass", 100, "V", 18.82032e-6, 1e-6},
    {"ETHANE", "T", 500, "Dmass", 400, "V", 66.15890e-6, 1e-6},
    {"ETHANE", "T", 675, "Dmass", 0, "V", 18.97242e-6, 1e-6},
    {"ETHANE", "T", 675, "Dmass", 1, "V", 18.99427e-6, 1e-6},
    {"ETHANE", "T", 675, "Dmass", 100, "V", 23.37711e-6, 1e-6},
    {"ETHANE", "T", 675, "Dmass", 300, "V", 45.90524e-6, 1e-6},

    // From Xiang, JPCRD, 2006
    {"Methanol", "T", 300, "Dmass", 0.12955, "V", 0.009696e-3, 1e-3},
    {"Methanol", "T", 300, "Dmass", 788.41, "V", 0.5422e-3, 1e-3},
    {"Methanol", "T", 630, "Dmass", 0.061183, "V", 0.02081e-3, 1e-3},
    {"Methanol", "T", 630, "Dmass", 888.50, "V", 0.2405e-3, 1e-1}, // They use a different EOS in the high pressure region

    // From REFPROP 9.1 since no data provided
    {"n-Butane", "T", 150, "Q", 0, "V", 0.0013697657668, 1e-4},
    {"n-Butane", "T", 400, "Q", 1, "V", 1.2027464524762453e-005, 1e-4},
    {"IsoButane", "T", 120, "Q", 0, "V", 0.0060558450757844271, 1e-4},
    {"IsoButane", "T", 400, "Q", 1, "V", 1.4761041187617117e-005, 2e-4},
    {"R134a", "T", 175, "Q", 0, "V", 0.0017558494524138289, 1e-4},
    {"R134a", "T", 360, "Q", 1, "V", 1.7140264998576107e-005, 1e-4},

    // From Tariq, JPCRD, 2014
    {"Cyclohexane", "T", 300, "Dmolar", 1e-10, "V", 7.058e-6, 1e-4},
    {"Cyclohexane", "T", 300, "Dmolar", 0.0430e3, "V", 6.977e-6, 1e-4},
    {"Cyclohexane", "T", 300, "Dmolar", 9.1756e3, "V", 863.66e-6, 1e-4},
    {"Cyclohexane", "T", 300, "Dmolar", 9.9508e3, "V", 2850.18e-6, 1e-4},
    {"Cyclohexane", "T", 500, "Dmolar", 1e-10, "V", 11.189e-6, 1e-4},
    {"Cyclohexane", "T", 500, "Dmolar", 6.0213e3, "V", 94.842e-6, 1e-4},
    {"Cyclohexane", "T", 500, "Dmolar", 8.5915e3, "V", 380.04e-6, 1e-4},
    {"Cyclohexane", "T", 700, "Dmolar", 1e-10, "V", 15.093e-6, 1e-4},
    {"Cyclohexane", "T", 700, "Dmolar", 7.4765e3, "V", 176.749e-6, 1e-4},

    // From Avgeri, JPCRD, 2014
    {"Benzene", "T", 300, "Dmass", 1e-10, "V", 7.625e-6, 1e-4},
    {"Benzene", "T", 400, "Dmass", 1e-10, "V", 10.102e-6, 1e-4},
    {"Benzene", "T", 550, "Dmass", 1e-10, "V", 13.790e-6, 1e-4},
    {"Benzene", "T", 300, "Dmass", 875, "V", 608.52e-6, 1e-4},
    {"Benzene", "T", 400, "Dmass", 760, "V", 211.74e-6, 1e-4},
    {"Benzene", "T", 550, "Dmass", 500, "V", 60.511e-6, 1e-4},

    // From Cao, JPCRD, 2016
    {"m-Xylene", "T", 300, "Dmolar", 1e-10, "V", 6.637e-6, 1e-4},
    {"m-Xylene", "T", 300, "Dmolar", 0.04*1e3, "V", 6.564e-6, 1e-4},
    {"m-Xylene", "T", 300, "Dmolar", 8.0849*1e3, "V", 569.680e-6, 1e-4},
    //{"m-Xylene", "T", 300, "Dmolar", 8.9421*1e3, "V", 1898.841e-6, 1e-4}, // Above max p of EOS
    {"m-Xylene", "T", 400, "Dmolar", 1e-10, "V", 8.616e-6, 1e-4},
    {"m-Xylene", "T", 400, "Dmolar", 0.04*1e3, "V", 8.585e-6, 1e-4},
    {"m-Xylene", "T", 400, "Dmolar", 7.2282*1e3, "V", 238.785e-6, 1e-4},
    {"m-Xylene", "T", 400, "Dmolar", 8.4734*1e3, "V", 718.950e-6, 1e-4},
    {"m-Xylene", "T", 600, "Dmolar", 1e-10, "V", 12.841e-6, 1e-4},
    {"m-Xylene", "T", 600, "Dmolar", 0.04*1e3, "V", 12.936e-6, 1e-4},
    {"m-Xylene", "T", 600, "Dmolar", 7.6591*1e3, "V", 299.164e-6, 1e-4},

    // From Cao, JPCRD, 2016
    {"o-Xylene", "T", 300, "Dmolar", 1e-10, "V", 6.670e-6, 1e-4},
    {"o-Xylene", "T", 300, "Dmolar", 0.04*1e3, "V", 6.598e-6, 1e-4},
    {"o-Xylene", "T", 300, "Dmolar", 8.2369*1e3, "V", 738.286e-6, 1e-4},
    //{"o-Xylene", "T", 300, "Dmolar", 8.7845*1e3, "V", 1645.436e-6, 1e-4},  // Above max p of EOS
    {"o-Xylene", "T", 400, "Dmolar", 1e-10, "V", 8.658e-6, 1e-4},
    {"o-Xylene", "T", 400, "Dmolar", 0.04*1e3, "V", 8.634e-6, 1e-4},
    {"o-Xylene", "T", 400, "Dmolar", 7.4060*1e3, "V", 279.954e-6, 1e-4},
    {"o-Xylene", "T", 400, "Dmolar", 8.2291*1e3, "V", 595.652e-6, 1e-4},
    {"o-Xylene", "T", 600, "Dmolar", 1e-10, "V", 12.904e-6, 1e-4},
    {"o-Xylene", "T", 600, "Dmolar", 0.04*1e3, "V", 13.018e-6, 1e-4},
    {"o-Xylene", "T", 600, "Dmolar", 7.2408*1e3, "V", 253.530e-6, 1e-4},

    // From Balogun, JPCRD, 2016
    {"p-Xylene", "T", 300, "Dmolar", 1e-10, "V", 6.604e-6, 1e-4},
    {"p-Xylene", "T", 300, "Dmolar", 0.049*1e3, "V", 6.405e-6, 1e-4},
    {"p-Xylene", "T", 300, "Dmolar", 8.0548*1e3, "V", 593.272e-6, 1e-4},
    //{"p-Xylene", "T", 300, "Dmolar", 8.6309*1e3, "V", 1266.337e-6, 1e-4}, // Above max p of EOS
    {"p-Xylene", "T", 400, "Dmolar", 1e-10, "V", 8.573e-6, 1e-4},
    {"p-Xylene", "T", 400, "Dmolar", 7.1995*1e3, "V", 239.202e-6, 1e-4},
    {"p-Xylene", "T", 400, "Dmolar", 8.0735*1e3, "V", 484.512e-6, 1e-4},
    {"p-Xylene", "T", 600, "Dmolar", 1e-10, "V", 12.777e-6, 1e-4},
    {"p-Xylene", "T", 600, "Dmolar", 7.0985*1e3, "V", 209.151e-6, 1e-4},

    // From Mylona, JPCRD, 2014
    {"EthylBenzene", "T", 617, "Dmass", 316, "V", 33.22e-6, 1e-2},

    // Heavy Water, Assael et al., JPCRD, 2021
    {"HeavyWater", "T", 298.15, "Dmass", 0   , "V", 10.035938e-6, 1e-8},
    {"HeavyWater", "T", 298.15, "Dmass", 1105, "V", 1092.6424e-6, 1e-8},
    {"HeavyWater", "T", 298.15, "Dmass", 1130, "V", 1088.3626e-6, 1e-8},
    {"HeavyWater", "T", 373.15, "Dmass", 1064, "V", 326.63791e-6, 1e-8},
    {"HeavyWater", "T", 775.00, "Dmass", 1   , "V", 29.639474e-6, 1e-8},
    {"HeavyWater", "T", 775.00, "Dmass", 100 , "V", 31.930085e-6, 1e-8},
    {"HeavyWater", "T", 775.00, "Dmass", 400 , "V", 53.324172e-6, 1e-4}, // Assumption of mu_1 = 1 not good for this point
    {"HeavyWater", "T", 644.101, "Dmass", 145, "V", 26.640959e-6, 1e-8},
    {"HeavyWater", "T", 644.101, "Dmass", 245, "V", 32.119967e-6, 1e-8},
    {"HeavyWater", "T", 644.101, "Dmass", 295, "V", 36.828275e-6, 1e-8},
    {"HeavyWater", "T", 644.101, "Dmass", 345, "V", 43.225017e-6, 1e-8},
    {"HeavyWater", "T", 644.101, "Dmass", 395, "V", 47.193530e-6, 1e-8},
    {"HeavyWater", "T", 644.101, "Dmass", 445, "V", 50.241640e-6, 1e-8},

    // Toluene, Avgeri, JPCRD, 2015
    {"Toluene", "T", 300, "Dmass", 1e-10, "V", 7.023e-6, 1e-4},
    {"Toluene", "T", 400, "Dmass", 1e-10, "V", 9.243e-6, 1e-4},
    {"Toluene", "T", 550, "Dmass", 1e-10, "V", 12.607e-6, 1e-4},
    {"Toluene", "T", 300, "Dmass", 865, "V", 566.78e-6, 1e-4},
    {"Toluene", "T", 400, "Dmass", 770, "V", 232.75e-6, 1e-4},
    {"Toluene", "T", 550, "Dmass", 550, "V", 80.267e-6, 1e-4},

    // Octane, from MH
    { "Octane","T",300,"Dmass",0,"V",5.7673e-6,1e-5 },
    { "Octane","T",300,"Dmass",700,"V",517.60e-6,1e-5 },
    { "Octane","T",512,"Dmass",485,"V",86.886e-6,1e-5 },

    
//    { "MDEA","T",298.0,"Dmass",0.00,"V",6.6142770e-6,1e-6 },
//    { "MDEA","T",298.0,"Dmass",1035.0000,"L",76782.882e-6,1e-6 },
//    { "MDEA","T",310.0,"Dmass",1025.8168,"L",40450.400e-6,1e-6 },

    // From NISTIR, 2018
//    {"R1243ZF", "T", 339.2, "Dmolar", 0.0, "V", 13.15103e-6, 1e-8},
//    {"R1243ZF", "T", 339.2, "Dmolar", 8901, "V", 103.5661e-6, 1e-8},

    {"R1336MZZE", "T", 298, "Dmass", 0.0, "V", 8.8880024e-6, 1e-8},
    {"R1336MZZE", "T", 298, "Dmass", 1340.0, "V", 303.23838e-6, 1e-8},
    
    // Sotiriadou et al., IJT, 2024
    {"THF","T",200,"Dmass",991.26,"V",2290.9e-6,1e-6},
    {"THF","T",250,"Dmass",940.08,"V",920.6e-6,1e-6},
    {"THF","T",300,"Dmass",888.43,"V",504.8e-6,1e-6},
    {"THF","T",350,"Dmass",834.98,"V",322.3e-6,1e-6},
    {"THF","T",400,"Dmass",778.12,"V",224.1e-6,1e-6},
    {"THF","T",450,"Dmass",715.04,"V",163.1e-6,1e-6},
    {"THF","T",500,"Dmass",639.54,"V",119.8e-6,1e-6},
    {"THF","T",200,"Dmass",997.97,"V",2768.9e-6,1e-6},
    {"THF","T",250,"Dmass",948.79,"V",1080.1e-6,1e-6},
    {"THF","T",300,"Dmass",899.99,"V",589.3e-6,1e-6},
    {"THF","T",350,"Dmass",850.70,"V",379.5e-6,1e-6},
    {"THF","T",400,"Dmass",800.14,"V",269.1e-6,1e-6},
    {"THF","T",450,"Dmass",747.42,"V",203.0e-6,1e-6},
    {"THF","T",500,"Dmass",691.46,"V",159.4e-6,1e-6},
    
    
    // New models added since REFPROP 10.0
    {"1-HEXENE", "T", 300, "Dmass", 0, "V", 6.72367979793243E-06, 1e-7},
    {"1-HEXENE", "T", 300, "Dmass", 700, "V", 0.0003643652910748, 1e-7},
    {"1-HEXENE", "T", 500, "Dmass", 1, "V", 1.10690591152088E-05, 1e-7},
    {"MDEA", "T", 350, "Dmass", 0, "V", 7.84933982928373E-06, 1e-7},
    {"MDEA", "T", 350, "Dmass", 1034, "V", 0.00914393315329331, 1e-7},
    {"MDEA", "T", 480, "Dmass", 0.5, "V", 1.09846411626608E-05, 1e-7},
    {"MILPRF23699", "T", 750, "Dmass", 0, "V", 7.0649179978274E-06, 1e-7},
    {"MILPRF23699", "T", 750, "Dmass", 1, "V", 7.0649179986931E-06, 1e-7},
    {"MILPRF23699", "T", 750, "Dmass", 637, "V", 0.000167201020667514, 1e-7},
    {"POE5", "T", 750, "Dmass", 0, "V", 8.02066505534832E-06, 1e-7},
    {"POE5", "T", 750, "Dmass", 10, "V", 8.02066651495334E-06, 1e-7},
    {"POE5", "T", 750, "Dmass", 632, "V", 9.65128871139775E-05, 1e-7},
    {"POE7", "T", 750, "Dmass", 0, "V", 6.66403167662706E-06, 1e-7},
    {"POE7", "T", 750, "Dmass", 1, "V", 6.66403167662706E-06, 1e-7},
    {"POE7", "T", 750, "Dmass", 632, "V", 0.00013633141836284, 1e-7},
    {"POE9", "T", 750, "Dmass", 0, "V", 6.14267840458504E-06, 1e-7},
    {"POE9", "T", 750, "Dmass", 1, "V", 6.14267840667601E-06, 1e-7},
    {"POE9", "T", 750, "Dmass", 628, "V", 0.000262527287701281, 1e-7},
    {"PROPYLENE GLYCOL", "T", 500, "Dmass", 0, "V", 1.30671111399009E-05, 1e-7},
    {"PROPYLENE GLYCOL", "T", 500, "Dmass", 5, "V", 1.31013695421416E-05, 1e-7},
    {"PROPYLENE GLYCOL", "T", 500, "Dmass", 860, "V", 0.000430026713907861, 1e-7},
    {"R1336MZZ(E)", "T", 300, "Dmass", 0, "V", 8.94905995084936E-06, 1e-7},
    {"R1336MZZ(E)", "T", 300, "Dmass", 5, "V", 8.89987084719874E-06, 1e-7},
    {"R1336MZZ(E)", "T", 300, "Dmass", 1320, "V", 0.000277796170086617, 1e-7},
//    {"THF", "T", 350, "Dmass", 0, "V", 9.80460082772613E-06, 1e-7},
//    {"THF", "T", 350, "Dmass", 1, "V", 9.79538084456908E-06, 1e-7},
//    {"THF", "T", 350, "Dmass", 830, "V", 0.000298372529925208, 1e-7},
    {"CHLORINE", "T", 230, "Dmass", 0, "V", 0.00001043385004573, 1e-7},
    {"CHLORINE", "T", 230, "Dmass", 1, "V", 1.04045169557019E-05, 1e-7},
    {"CHLORINE", "T", 230, "Dmass", 1591, "V", 0.000526817055834834, 1e-7},
    {"CYCLOPENTANE", "T", 240, "Dmass", 0, "V", 6.43278046254749E-06, 1e-7},
    {"CYCLOPENTANE", "T", 240, "Dmass", 801.5, "V", 0.000972435099833185, 1e-7},
    {"CYCLOPENTANE", "T", 400, "Dmass", 650, "V", 0.000194942607428024, 1e-7},
    {"HEAVY WATER", "T", 298.15, "Dmass", 0, "V", 1.00359376781603E-05, 1e-7},
    {"HEAVY WATER", "T", 298.15, "Dmass", 1105, "V", 0.00109264240791847, 1e-7},
    {"HEAVY WATER", "T", 298.15, "Dmass", 1130, "V", 0.00108836261745782, 1e-7},
    {"D5", "T", 298.15, "Dmass", 0, "V", 5.68750996810537E-06, 1e-7},
    {"D5", "T", 298.15, "Dmass", 954, "V", 0.00415591685108145, 1e-7},
    {"D5", "T", 400, "Dmass", 844, "V", 0.000755204141396476, 1e-7},
    {"ETHYLENE GLYCOL", "T", 350, "Dmass", 0, "V", 9.52201053984092E-06, 1e-7},
    {"ETHYLENE GLYCOL", "T", 350, "Dmass", 0.01, "V", 9.52480390690984E-06, 1e-7},
    {"ETHYLENE GLYCOL", "T", 350, "Dmass", 1100, "V", 0.00424678429140958, 1e-7},
    {"MD2M", "T", 300, "Dmass", 0, "V", 5.56506544277807E-06, 1e-7},
    {"MD2M", "T", 300, "Dmass", 848, "V", 0.0013134555394017, 1e-7},
    {"MD2M", "T", 400, "Dmass", 746, "V", 0.000406093197263763, 1e-7},
    {"MDM", "T", 333, "Dmass", 0, "V", 6.67276228935578E-06, 1e-7},
    {"MDM", "T", 333, "Dmass", 780, "V", 0.000554792009453641, 1e-7},
    {"MDM", "T", 400, "Dmass", 716, "V", 0.00031297726203291, 1e-7},
    {"MM", "T", 300, "Dmass", 0, "V", 6.53188454206884E-06, 1e-7},
    {"MM", "T", 300, "Dmass", 758, "V", 0.000476860254143413, 1e-7},
    {"MM", "T", 400, "Dmass", 645, "V", 0.000174021407561764, 1e-7},
    {"N2O", "T", 444, "Dmass", 0, "V", 2.13055095965517E-05, 1e-7},
    {"N2O", "T", 444, "Dmass", 516, "V", 4.15882876104059E-05, 1e-7},
    {"N2O", "T", 500, "Dmass", 0, "V", 2.35542844700301E-05, 1e-7},
    {"R113", "T", 313, "Dmass", 0, "V", 1.08736857403336E-05, 1e-7},
    {"R113", "T", 313, "Dmass", 3, "V", 1.08041621921657E-05, 1e-7},
    {"R113", "T", 313, "Dmass", 1528, "V", 0.000548865455122245, 1e-7},
    {"R12", "T", 240, "Dmass", 0, "V", 9.486987925E-06, 1e-7},
    {"R12", "T", 240, "Dmass", 1, "V", 9.470144971E-06, 1e-7},
    {"R12", "T", 240, "Dmass", 1500, "V", 360.3775924e-6, 1e-7},
    {"R1224YDZ", "T", 300, "Dmass", 0, "V", 1.05980696194996E-05, 1e-7},
    {"R1224YDZ", "T", 300, "Dmass", 1, "V", 1.05778914530355E-05, 1e-7},
    {"R1224YDZ", "T", 300, "Dmass", 1400, "V", 0.000346525856127829, 1e-7},
    {"R1243ZF", "T", 293, "Dmass", 0, "V", 11.3486371E-6, 1e-7},
    {"R1243ZF", "T", 293, "Dmass", 994, "V", 161.7891196E-6, 1e-7},
    {"R1243ZF", "T", 373, "Dmass", 598, "V", 44.4965965E-06, 1e-7},
    {"R1336MZZ(Z)", "T", 300, "Dmass", 0, "V", 1.03013705031937E-05, 1e-7},
    {"R1336MZZ(Z)", "T", 300, "Dmass", 1363, "V", 0.000360317097412434, 1e-7},
    {"R1336MZZ(Z)", "T", 300, "Dmass", 1, "V", 1.02876352829735E-05, 1e-7},
    {"R13B1", "T", 293, "Dmass", 0, "V", 1.51781317364921E-05, 1e-7},
    {"R13B1", "T", 293, "Dmass", 5, "V", 1.51671283684348E-05, 1e-7},
    {"R13B1", "T", 293, "Dmass", 1930, "V", 0.00031087189711179, 1e-7},
    {"R23", "T", 130, "Dmass", 0, "V", 6.4509802507718E-06, 1e-7},
    {"R23", "T", 130, "Dmass", 1663, "V", 0.00116857951831184, 1e-7},
    {"R23", "T", 200, "Dmass", 5, "V", 9.94315932695455E-06, 1e-7},
    {"R32", "T", 300, "Dmass", 0, "V", 1.26169803503671E-05, 1e-7},
    {"R32", "T", 300, "Dmass", 10, "V", 1.26333351323668E-05, 1e-7},
    {"R32", "T", 300, "Dmass", 1100, "V", 0.000173431755378586, 1e-7},
    {"RE347MCC", "T", 273, "Dmass", 0, "V", 1.00681970635436E-05, 1e-7},
    {"RE347MCC", "T", 273, "Dmass", 2, "V", 9.93994270195466E-06, 1e-7},
    {"RE347MCC", "T", 273, "Dmass", 1475, "V", 0.000612195168691688, 1e-7},
    {"XENON", "T", 300, "Dmass", 0, "V", 2.31561023091198E-05, 1e-7},
    {"XENON", "T", 300, "Dmass", 6, "V", 2.33186009197807E-05, 1e-7},
    {"XENON", "T", 300, "Dmass", 2500, "V", 0.000206449445098134, 1e-7},


    // ***********************************************************
    //                   CONDUCTIVITY
    // ***********************************************************

    // From Assael, JPCRD, 2013
    { "Hexane", "T", 250, "Dmass", 700, "L", 137.62e-3, 1e-4 },
    { "Hexane", "T", 400, "Dmass", 2, "L", 23.558e-3, 1e-4 },
    { "Hexane", "T", 400, "Dmass", 650, "L", 129.28e-3, 2e-4 },
    { "Hexane", "T", 510, "Dmass", 2, "L", 36.772e-3, 1e-4 },

    // From Assael, JPCRD, 2013
    { "Heptane", "T", 250, "Dmass", 720, "L", 137.09e-3, 1e-4 },
    { "Heptane", "T", 400, "Dmass", 2, "L", 21.794e-3, 1e-4 },
    { "Heptane", "T", 400, "Dmass", 650, "L", 120.75e-3, 1e-4 },
    { "Heptane", "T", 535, "Dmass", 100, "L", 51.655e-3, 3e-3 }, // Relaxed tolerance because conductivity was fit using older viscosity correlation

    // From Assael, JPCRD, 2013
    { "Ethanol", "T", 300, "Dmass", 850, "L", 209.68e-3, 1e-4 },
    { "Ethanol", "T", 400, "Dmass", 2, "L", 26.108e-3, 1e-4 },
    { "Ethanol", "T", 400, "Dmass", 690, "L", 149.21e-3, 1e-4 },
    { "Ethanol", "T", 500, "Dmass", 10, "L", 39.594e-3, 1e-4 },

    // From Assael, JPCRD, 2012
    {"Toluene", "T", 298.15, "Dmass", 1e-15, "L", 10.749e-3, 1e-4},
    {"Toluene", "T", 298.15, "Dmass", 862.948, "L", 130.66e-3, 1e-4},
    {"Toluene", "T", 298.15, "Dmass", 876.804, "L", 136.70e-3, 1e-4},
    {"Toluene", "T", 595, "Dmass", 1e-15, "L", 40.538e-3, 1e-4},
    {"Toluene", "T", 595, "Dmass", 46.512, "L", 41.549e-3, 1e-4},
    {"Toluene", "T", 185, "Dmass", 1e-15, "L", 4.3758e-3, 1e-4},
    {"Toluene", "T", 185, "Dmass", 968.821, "L", 158.24e-3, 1e-4},

    // From Assael, JPCRD, 2012
    { "SF6", "T", 298.15, "Dmass", 1e-13, "L", 12.952e-3, 1e-4 },
    { "SF6", "T", 298.15, "Dmass", 100, "L", 14.126e-3, 1e-4 },
    { "SF6", "T", 298.15, "Dmass", 1600, "L", 69.729e-3, 1e-4 },
    { "SF6", "T", 310, "Dmass", 1e-13, "L", 13.834e-3, 1e-4 },
    { "SF6", "T", 310, "Dmass", 1200, "L", 48.705e-3, 1e-4 },
    { "SF6", "T", 480, "Dmass", 100, "L", 28.847e-3, 1e-4 },

    //// From Assael, JPCRD, 2012
    {"Benzene", "T", 290, "Dmass", 890, "L", 147.66e-3, 1e-4},
    {"Benzene", "T", 500, "Dmass", 2, "L", 30.174e-3, 1e-4},
    {"Benzene", "T", 500, "Dmass", 32, "L", 32.175e-3, 1e-4},
    {"Benzene", "T", 500, "Dmass", 800, "L", 141.24e-3, 1e-4},
    {"Benzene", "T", 575, "Dmass", 1.7, "L", 37.763e-3, 1e-4},

        // From Assael, JPCRD, 2011
    { "Hydrogen", "T", 298.15, "Dmass", 1e-13, "L", 185.67e-3, 1e-4 },
    { "Hydrogen", "T", 298.15, "Dmass", 0.80844, "L", 186.97e-3, 1e-4 },
    { "Hydrogen", "T", 298.15, "Dmass", 14.4813, "L", 201.35e-3, 1e-4 },
    { "Hydrogen", "T", 35, "Dmass", 1e-13, "L", 26.988e-3, 1e-4 },
    { "Hydrogen", "T", 35, "Dmass", 30, "L", 0.0770177, 1e-4 }, // Updated since Assael uses a different viscosity correlation
    { "Hydrogen", "T", 18, "Dmass", 1e-13, "L", 13.875e-3, 1e-4 },
    { "Hydrogen", "T", 18, "Dmass", 75, "L", 104.48e-3, 1e-4 },
    /*{"ParaHydrogen", "T", 298.15, "Dmass", 1e-13, "L", 192.38e-3, 1e-4},
    {"ParaHydrogen", "T", 298.15, "Dmass", 0.80844, "L", 192.81e-3, 1e-4},
    {"ParaHydrogen", "T", 298.15, "Dmass", 14.4813, "L", 207.85e-3, 1e-4},
    {"ParaHydrogen", "T", 35, "Dmass", 1e-13, "L", 27.222e-3, 1e-4},
    {"ParaHydrogen", "T", 35, "Dmass", 30, "L", 70.335e-3, 1e-4},
    {"ParaHydrogen", "T", 18, "Dmass", 1e-13, "L", 13.643e-3, 1e-4},
    {"ParaHydrogen", "T", 18, "Dmass", 75, "L", 100.52e-3, 1e-4},*/

    // Some of these don't work
    { "R125", "T", 341, "Dmass", 600, "L", 0.0565642978494, 2e-4 },
    { "R125", "T", 200, "Dmass", 1e-13, "L", 0.007036843623086, 2e-4 },
    { "IsoButane", "T", 390, "Dmass", 387.09520158645068, "L", 0.063039, 2e-4 },
    { "IsoButane", "T", 390, "Dmass", 85.76703973869482, "L", 0.036603, 2e-4 },
    { "n-Butane", "T", 415, "Dmass", 360.01895129934866, "L", 0.067045, 2e-4 },
    { "n-Butane", "T", 415, "Dmass", 110.3113177144, "L", 0.044449, 1e-4 },

    // From Huber, FPE, 2005
    { "n-Octane", "T", 300, "Dmolar", 6177.2, "L", 0.12836, 1e-4 },
    { "n-Nonane", "T", 300, "Dmolar", 5619.4, "L", 0.13031, 1e-4 },
    //{"n-Decane", "T", 300, "Dmass", 5150.4, "L", 0.13280, 1e-4}, // no viscosity

    // From Huber, EF, 2004
    { "n-Dodecane", "T", 300, "Dmolar", 4411.5, "L", 0.13829, 1e-4 },
    { "n-Dodecane", "T", 500, "Dmolar", 3444.7, "L", 0.09384, 1e-4 },
    { "n-Dodecane", "T", 660, "Dmolar", 1500.98, "L", 0.090346, 1e-4 },

    // From REFPROP 9.1 since no data provided in Marsh, 2002
    { "n-Propane", "T", 368, "Q", 0, "L", 0.07282154952457,1e-3 },
    { "n-Propane", "T", 368, "Dmolar", 1e-10, "L", 0.0266135388745317,1e-4 },

    // From Perkins, JCED, 2011
    //{"R1234yf", "T", 250, "Dmass", 2.80006, "L", 0.0098481, 1e-4},
    //{"R1234yf", "T", 300, "Dmass", 4.671556, "L", 0.013996, 1e-4},
    //{"R1234yf", "T", 250, "Dmass", 1299.50, "L", 0.088574, 1e-4},
    //{"R1234yf", "T", 300, "Dmass", 1182.05, "L", 0.075245, 1e-4},
    //{"R1234ze(E)", "T", 250, "Dmass", 2.80451, "L", 0.0098503, 1e-4},
    //{"R1234ze(E)", "T", 300, "Dmass", 4.67948, "L", 0.013933, 1e-4},
    //{"R1234ze(E)", "T", 250, "Dmass", 1349.37, "L", 0.10066, 1e-4},
    //{"R1234ze(E)", "T", 300, "Dmass", 1233.82, "L", 0.085389, 1e-4},

    // From Laesecke, IJR 1995
    { "R123", "T", 180, "Dmass", 1739, "L", 110.9e-3, 2e-4 },
    { "R123", "T", 180, "Dmass", 0.2873e-2, "L", 2.473e-3, 1e-3 },
    { "R123", "T", 430, "Dmass", 996.35, "L", 45.62e-3, 1e-3 },
    { "R123", "T", 430, "Dmass", 166.9,  "L", 21.03e-3, 1e-3 },

    // From Huber, JPCRD, 2016
    { "CO2", "T", 250, "Dmass", 0, "L", 12.99e-3, 1e-4 },
    { "CO2", "T", 250, "Dmass", 2, "L", 13.05e-3, 1e-4 },
    { "CO2", "T", 250, "Dmass", 1058, "L", 140.0e-3, 1e-4 },
    { "CO2", "T", 310, "Dmass", 400, "L", 73.04e-3, 1e-4 },

    // From Friend, JPCRD, 1991
    { "Ethane", "T", 100, "Dmass", 1e-13, "L", 3.46e-3, 1e-2 },
    { "Ethane", "T", 230, "Dmolar", 16020, "L", 126.2e-3, 1e-2 },
    { "Ethane", "T", 440, "Dmolar", 1520, "L", 45.9e-3, 1e-2 },
    { "Ethane", "T", 310, "Dmolar", 4130, "L", 45.4e-3, 1e-2 },

    // From Lemmon and Jacobsen, JPCRD, 2004
    { "Nitrogen", "T", 100, "Dmolar", 1e-14, "L", 9.27749e-3, 1e-4 },
    { "Nitrogen", "T", 300, "Dmolar", 1e-14, "L", 25.9361e-3, 1e-4 },
    { "Nitrogen", "T", 100, "Dmolar", 25000, "L", 103.834e-3, 1e-4 },
    { "Nitrogen", "T", 200, "Dmolar", 10000, "L", 36.0099e-3, 1e-4 },
    { "Nitrogen", "T", 300, "Dmolar", 5000, "L", 32.7694e-3, 1e-4 },
    { "Nitrogen", "T", 126.195, "Dmolar", 11180, "L", 675.800e-3, 1e-4 },
    { "Argon", "T", 100, "Dmolar", 1e-14, "L", 6.36587e-3, 1e-4 },
    { "Argon", "T", 300, "Dmolar", 1e-14, "L", 17.8042e-3, 1e-4 },
    { "Argon", "T", 100, "Dmolar", 33000, "L", 111.266e-3, 1e-4 },
    { "Argon", "T", 200, "Dmolar", 10000, "L", 26.1377e-3, 1e-4 },
    { "Argon", "T", 300, "Dmolar", 5000, "L", 23.2302e-3, 1e-4 },
    { "Argon", "T", 150.69, "Dmolar", 13400, "L", 856.793e-3, 1e-4 },
    { "Oxygen", "T", 100, "Dmolar", 1e-14, "L", 8.94334e-3, 1e-4 },
    { "Oxygen", "T", 300, "Dmolar", 1e-14, "L", 26.4403e-3, 1e-4 },
    { "Oxygen", "T", 100, "Dmolar", 35000, "L", 146.044e-3, 1e-4 },
    { "Oxygen", "T", 200, "Dmolar", 10000, "L", 34.6124e-3, 1e-4 },
    { "Oxygen", "T", 300, "Dmolar", 5000, "L", 32.5491e-3, 1e-4 },
    { "Oxygen", "T", 154.6, "Dmolar", 13600, "L", 377.476e-3, 1e-4 },
    { "AIR.PPF", "T", 100, "Dmolar", 1e-14, "L", 9.35902e-3, 1e-4 },
    { "AIR.PPF", "T", 300, "Dmolar", 1e-14, "L", 26.3529e-3, 1e-4 },
    { "AIR.PPF", "T", 100, "Dmolar", 28000, "L", 119.221e-3, 1e-4 },
    { "AIR.PPF", "T", 200, "Dmolar", 10000, "L", 35.3185e-3, 1e-4 },
    { "AIR.PPF", "T", 300, "Dmolar", 5000, "L", 32.6062e-3, 1e-4 },
    { "AIR.PPF", "T", 132.64, "Dmolar", 10400, "L", 75.6231e-3, 1e-4 },

    // Huber, JPCRD, 2012
    { "Water", "T", 298.15, "Dmass", 1e-14, "L", 18.4341883e-3, 1e-6 },
    { "Water", "T", 298.15, "Dmass", 998, "L", 607.712868e-3, 1e-6 },
    { "Water", "T", 298.15, "Dmass", 1200, "L", 799.038144e-3, 1e-6 },
    { "Water", "T", 873.15, "Dmass", 1e-14, "L", 79.1034659e-3, 1e-6 },
    { "Water", "T", 647.35, "Dmass", 1, "L", 51.9298924e-3, 1e-6 },
    { "Water", "T", 647.35, "Dmass", 122, "L", 130.922885e-3, 2e-4 },
    { "Water", "T", 647.35, "Dmass", 222, "L", 367.787459e-3, 2e-4 },
    { "Water", "T", 647.35, "Dmass", 272, "L", 757.959776e-3, 2e-4 },
    { "Water", "T", 647.35, "Dmass", 322, "L", 1443.75556e-3, 2e-4 },
    { "Water", "T", 647.35, "Dmass", 372, "L", 650.319402e-3, 2e-4 },
    { "Water", "T", 647.35, "Dmass", 422, "L", 448.883487e-3, 2e-4 },
    { "Water", "T", 647.35, "Dmass", 750, "L", 600.961346e-3, 2e-4 },

    // From Monogenidou, 2018
    { "Ammonia", "T", 390, "Dmass", 415, "L", 264.13e-3, 1e-4 },

    // From Hands, Cryogenics, 1981
    { "Helium", "T", 800, "P", 1e5, "L", 0.3085, 1e-2 },
    { "Helium", "T", 300, "P", 1e5, "L", 0.1560, 1e-2 },
    { "Helium", "T", 20, "P", 1e5, "L", 0.0262, 1e-2 },
    { "Helium", "T", 8, "P", 1e5, "L", 0.0145, 1e-2 },
    { "Helium", "T", 4, "P", 20e5, "L", 0.0255, 1e-2 },
    { "Helium", "T", 8, "P", 20e5, "L", 0.0308, 1e-2 },
    { "Helium", "T", 20, "P", 20e5, "L", 0.0328, 1e-2 },
    { "Helium", "T", 4, "P", 100e5, "L", 0.0385, 3e-2 },
    { "Helium", "T", 8, "P", 100e5, "L", 0.0566, 3e-2 },
    { "Helium", "T", 20, "P", 100e5, "L", 0.0594, 1e-2 },
    { "Helium", "T", 4, "P", 1e5, "L", 0.0186, 1e-2 },
    { "Helium", "T", 4, "P", 2e5, "L", 0.0194, 1e-2 },
    { "Helium", "T", 5.180, "P", 2.3e5, "L", 0.0195, 1e-1 },
    { "Helium", "T", 5.2, "P", 2.3e5, "L", 0.0202, 1e-1 },
    { "Helium", "T", 5.230, "P", 2.3e5, "L", 0.0181, 1e-1 },
    { "Helium", "T", 5.260, "P", 2.3e5, "L", 0.0159, 1e-1 },
    { "Helium", "T", 5.3, "P", 2.3e5, "L", 0.0149, 1e-1 },

    // Geller, IJT, 2001 - based on experimental data, no validation data provided
    //{"R404A", "T", 253.03, "P", 0.101e6, "L", 0.00991, 0.03},
    //{"R404A", "T", 334.38, "P", 2.176e6, "L", 19.93e-3, 0.03},
    //{"R407C", "T", 253.45, "P", 0.101e6, "L", 0.00970, 0.03},
    //{"R407C", "T", 314.39, "P", 0.458e6, "L", 14.87e-3, 0.03},
    //{"R410A", "T", 260.32, "P", 0.101e6, "L", 0.01043, 0.03},
    //{"R410A", "T", 332.09, "P", 3.690e6, "L", 22.76e-3, 0.03},
    //{"R507A", "T", 254.85, "P", 0.101e6, "L", 0.01007, 0.03},
    //{"R507A", "T", 333.18, "P", 2.644e6, "L", 21.31e-3, 0.03},

    // From REFPROP 9.1 since no data provided
    { "R134a", "T", 240, "Dmass", 1e-10, "L", 0.008698768 , 1e-4 },
    { "R134a", "T", 330, "Dmass", 1e-10, "L", 0.015907606 , 1e-4 },
    { "R134a", "T", 330, "Q", 0, "L", 0.06746432253 , 1e-4 },
    { "R134a", "T", 240, "Q", 1, "L", 0.00873242359, 1e-4 },

    // Mylona, JPCRD, 2014
    { "o-Xylene", "T", 635, "Dmass", 270, "L", 96.4e-3 , 1e-2 },
    { "m-Xylene", "T", 616, "Dmass", 220, "L", 79.5232e-3 , 1e-2 }, // CoolProp is correct, paper is incorrect (it seems)
    { "p-Xylene", "T", 620, "Dmass", 287, "L", 107.7e-3 , 1e-2 },
    { "EthylBenzene", "T", 617, "Dmass", 316, "L", 140.2e-3, 1e-2 },
    // dilute values
    { "o-Xylene", "T", 300, "Dmass", 1e-12, "L", 13.68e-3 , 1e-3 },
    { "o-Xylene", "T", 600, "Dmass", 1e-12, "L", 41.6e-3 , 1e-3 },
    { "m-Xylene", "T", 300, "Dmass", 1e-12, "L", 9.45e-3 , 1e-3 },
    { "m-Xylene", "T", 600, "Dmass", 1e-12, "L", 40.6e-3 , 1e-3 },
    { "p-Xylene", "T", 300, "Dmass", 1e-12, "L", 10.57e-3 , 1e-3 },
    { "p-Xylene", "T", 600, "Dmass", 1e-12, "L", 41.73e-3 , 1e-3 },
    { "EthylBenzene", "T", 300, "Dmass", 1e-12, "L", 9.71e-3, 1e-3 },
    { "EthylBenzene", "T", 600, "Dmass", 1e-12, "L", 41.14e-3, 1e-3 },

    // Friend, JPCRD, 1989
    { "Methane", "T", 100, "Dmass", 1e-12, "L", 9.83e-3, 1e-3 },
    { "Methane", "T", 400, "Dmass", 1e-12, "L", 49.96e-3, 1e-3 },
    { "Methane", "T", 182, "Q", 0, "L", 82.5e-3, 5e-3 },
    { "Methane", "T", 100, "Dmolar", 28.8e3, "L", 234e-3, 1e-2 },

    // Sykioti, JPCRD, 2013
    { "Methanol", "T", 300, "Dmass", 850, "L", 241.48e-3, 1e-2 },
    { "Methanol", "T", 400, "Dmass", 2, "L", 25.803e-3, 1e-2 },
    { "Methanol", "T", 400, "Dmass", 690, "L", 183.59e-3, 1e-2 },
    { "Methanol", "T", 500, "Dmass", 10, "L", 40.495e-3, 1e-2 },

    // Heavy Water, IAPWS '22 formulation
    { "HeavyWater","T",298.15,"Dmass",0,"L",17.7498e-3,1e-5 },
    { "HeavyWater","T",298.15,"Dmass",1104.5,"L",599.557e-3,1e-5 },
    { "HeavyWater","T",298.15,"Dmass",1200,"L",690.421e-3,1e-5 },
    { "HeavyWater","T",825.00,"Dmass",0,"L",76.4492e-3,1e-5 },

    // Vassiliou, JPCRD, 2015
    { "Cyclopentane", "T", 512, "Dmass", 1e-12, "L", 37.042e-3, 1e-5 },
    { "Cyclopentane", "T", 512, "Dmass", 400, "L", 69.698e-3, 1e-1 },
    { "Isopentane", "T", 460, "Dmass", 1e-12, "L", 35.883e-3, 1e-4 },
    { "Isopentane", "T", 460, "Dmass", 329.914, "L", 59.649e-3, 1e-1 },
    { "n-Pentane", "T", 460, "Dmass", 1e-12, "L", 34.048e-3, 1e-5 },
    { "n-Pentane", "T", 460, "Dmass", 377.687, "L", 71.300e-3, 1e-1 },
    
    // Perkins, R.A., Huber, M.L. & Assael, M.J. Int J Thermophys 43, 12 (2022). https://doi.org/10.1007/s10765-021-02941-7
    { "RE347MCC","T",273.0,"Dmass",0.00,"L",0.010088,1e-6 },
    { "RE347MCC","T",273.0,"Dmass",1475.0,"L",0.070957,1e-6 },
    { "RE347MCC","T",435.0,"Dmass",0.00,"L",0.023416,1e-6 },
    { "RE347MCC","T",435.0,"Dmass",300.0,"L",0.033789,1e-6 },

    {"R1336MZZE", "T", 298, "Dmass", 0.0, "L", 12.404656e-3, 1e-8},
    {"R1336MZZE", "T", 298, "Dmass", 1340.0, "L", 71.773531e-3, 1e-8},

    { "R113", "T", 300, "Dmass", 0.0, "L", 7.2162851e-3, 1e-8 },
    { "R113", "T", 300, "Dmass", 1559.0, "L", 71.819605e-3, 1e-8 },
    
    // Sotiriadou, IJT, 2024
    {"THF","T",200,"Dmass",991.26,"L",196.4e-3, 1e-6},
    {"THF","T",250,"Dmass",940.08,"L",175.4e-3, 1e-6},
    {"THF","T",300,"Dmass",888.43,"L",153.9e-3, 1e-6},
    {"THF","T",350,"Dmass",834.98,"L",134.1e-3, 1e-6},
    {"THF","T",400,"Dmass",778.12,"L",117.0e-3, 1e-6},
    {"THF","T",450,"Dmass",715.04,"L",103.2e-3, 1e-6},
    {"THF","T",500,"Dmass",639.54,"L",92.6e-3, 1e-6},
    {"THF","T",200,"Dmass",997.97,"L",200.2e-3, 1e-6},
    {"THF","T",250,"Dmass",948.79,"L",180.2e-3, 1e-6},
    {"THF","T",300,"Dmass",899.99,"L",159.9e-3, 1e-6},
    {"THF","T",350,"Dmass",850.70,"L",141.0e-3, 1e-6},
    {"THF","T",400,"Dmass",800.14,"L",124.9e-3, 1e-6},
    {"THF","T",450,"Dmass",747.42,"L",112.0e-3, 1e-6},
    {"THF","T",500,"Dmass",691.46,"L",102.5e-3, 1e-6},
    
    // New models added since REFPROP 10.0
    {"1-HEXENE", "T", 300, "Dmass", 0, "L", 0.0125894432681896, 1e-7},
    {"1-HEXENE", "T", 300, "Dmass", 700, "L", 0.132139160523451, 1e-7},
    {"1-HEXENE", "T", 500, "Dmass", 1, "L", 0.0349695871935857, 1e-7},
    {"MDEA", "T", 350, "Dmass", 0, "L", 0.0192224244226866, 1e-7},
    {"MDEA", "T", 350, "Dmass", 1034, "L", 0.1716879101914, 1e-7},
    {"MDEA", "T", 480, "Dmass", 0.5, "L", 0.0346693287001671, 1e-7},
    {"MILPRF23699", "T", 750, "Dmass", 0, "L", 0.0246684380580137, 1e-7},
    {"MILPRF23699", "T", 750, "Dmass", 1, "L", 0.0247849357582438, 1e-7},
    {"MILPRF23699", "T", 750, "Dmass", 637, "L", 0.0760067699665674, 1e-7},
    {"POE5", "T", 750, "Dmass", 0, "L", 0.0275837932040537, 1e-7},
    {"POE5", "T", 750, "Dmass", 10, "L", 0.0296672458365272, 1e-7},
    {"POE5", "T", 750, "Dmass", 632, "L", 0.0712575725562785, 1e-7},
    {"POE7", "T", 750, "Dmass", 0, "L", 0.0275731895486971, 1e-7},
    {"POE7", "T", 750, "Dmass", 1, "L", 0.0277446116037626, 1e-7},
    {"POE7", "T", 750, "Dmass", 632, "L", 0.0704244816592156, 1e-7},
    {"POE9", "T", 750, "Dmass", 0, "L", 0.0280969394581862, 1e-7},
    {"POE9", "T", 750, "Dmass", 1, "L", 0.0282794680240426, 1e-7},
    {"POE9", "T", 750, "Dmass", 628, "L", 0.082011054539003, 1e-7},
    {"PROPYLENE GLYCOL", "T", 500, "Dmass", 0, "L", 0.0451850071063311, 1e-7},
    {"PROPYLENE GLYCOL", "T", 500, "Dmass", 5, "L", 0.0450405185559355, 1e-7},
    {"PROPYLENE GLYCOL", "T", 500, "Dmass", 860, "L", 0.176533396042923, 1e-7},
    {"R1336MZZ(E)", "T", 300, "Dmass", 0, "L", 0.0124499527059053, 1e-7},
    {"R1336MZZ(E)", "T", 300, "Dmass", 5, "L", 0.0124885120348959, 1e-7},
    {"R1336MZZ(E)", "T", 300, "Dmass", 1320, "L", 0.070441851809977, 1e-7},
//    {"THF", "T", 350, "Dmass", 0, "L", 0.017043724429919, 1e-7},
//    {"THF", "T", 350, "Dmass", 1, "L", 0.0170329262613654, 1e-7},
//    {"THF", "T", 350, "Dmass", 830, "L", 0.137417698388207, 1e-7},
    {"CHLORINE", "T", 230, "Dmass", 0, "L", 0.00675816048599364, 1e-7},
    {"CHLORINE", "T", 230, "Dmass", 1, "L", 0.00673540153022207, 1e-7},
    {"CHLORINE", "T", 230, "Dmass", 1591, "L", 0.168322846614074, 1e-7},
    {"CYCLOPENTANE", "T", 240, "Dmass", 0, "L", 0.00668522687935635, 1e-7},
    {"CYCLOPENTANE", "T", 240, "Dmass", 801.5, "L", 0.153075921666864, 1e-7},
    {"CYCLOPENTANE", "T", 400, "Dmass", 650, "L", 0.0994982494864392, 1e-7},
    {"HEAVY WATER", "T", 298.15, "Dmass", 0, "L", 0.0177498232443124, 1e-7},
    {"HEAVY WATER", "T", 298.15, "Dmass", 1105, "L", 0.599968418443271, 1e-7},
    {"HEAVY WATER", "T", 298.15, "Dmass", 1130, "L", 0.621611761812763, 1e-7},
    {"D5", "T", 298.15, "Dmass", 0, "L", 0.00992527476245956, 1e-7},
    {"D5", "T", 298.15, "Dmass", 954, "L", 0.115035732012566, 1e-7},
    {"D5", "T", 400, "Dmass", 844, "L", 0.0913496627998883, 1e-7},
    {"ETHYLENE GLYCOL", "T", 350, "Dmass", 0, "L", 0.0205428033974662, 1e-7},
    {"ETHYLENE GLYCOL", "T", 350, "Dmass", 0.01, "L", 0.0205420817841618, 1e-7},
    {"ETHYLENE GLYCOL", "T", 350, "Dmass", 1100, "L", 0.269603034545444, 1e-7},
    {"MD2M", "T", 300, "Dmass", 0, "L", 0.00996829137561801, 1e-7},
    {"MD2M", "T", 300, "Dmass", 848, "L", 0.108726387685464, 1e-7},
    {"MD2M", "T", 400, "Dmass", 746, "L", 0.0870695257115092, 1e-7},
    {"MDM", "T", 333, "Dmass", 0, "L", 0.0132136555584385, 1e-7},
    {"MDM", "T", 333, "Dmass", 780, "L", 0.0950170845567858, 1e-7},
    {"MDM", "T", 400, "Dmass", 716, "L", 0.0836286684022599, 1e-7},
    {"MM", "T", 300, "Dmass", 0, "L", 0.0128538953436524, 1e-7},
    {"MM", "T", 300, "Dmass", 758, "L", 0.102823188921216, 1e-7},
    {"MM", "T", 400, "Dmass", 645, "L", 0.0778385251873491, 1e-7},
    {"N2O", "T", 444, "Dmass", 0, "L", 0.0292259851287374, 1e-7},
    {"N2O", "T", 444, "Dmass", 516, "L", 0.0578398522226677, 1e-7},
    {"N2O", "T", 500, "Dmass", 0, "L", 0.0334624509103059, 1e-7},
    {"R113", "T", 313, "Dmass", 0, "L", 0.00774572835622552, 1e-7},
    {"R113", "T", 313, "Dmass", 3, "L", 0.00775832212321464, 1e-7},
    {"R113", "T", 313, "Dmass", 1528, "L", 0.0691324090436349, 1e-7},
    {"R12", "T", 240, "Dmass", 0, "L", 7.130349421e-3, 1e-7},
    {"R12", "T", 240, "Dmass", 1, "L", 7.136016811e-3, 1e-7},
    {"R12", "T", 240, "Dmass", 1500, "L", 86.68572196e-3, 1e-7},
    {"R1224YDZ", "T", 300, "Dmass", 0, "L", 0.00967878602054618, 1e-7},
    {"R1224YDZ", "T", 300, "Dmass", 1, "L", 0.00969009650648601, 1e-7},
    {"R1224YDZ", "T", 300, "Dmass", 1400, "L", 0.0809438471524826, 1e-7},
    {"R1243ZF", "T", 293, "Dmass", 0, "L", 14.20116623e-3, 1e-7},
    {"R1243ZF", "T", 293, "Dmass", 994, "L", 69.68530602e-3, 1e-7},
    {"R1243ZF", "T", 373, "Dmass", 598, "L", 51.80643624e-3, 1e-7},
    {"R1336MZZ(Z)", "T", 300, "Dmass", 0, "L", 0.0110563356834747, 1e-7},
    {"R1336MZZ(Z)", "T", 300, "Dmass", 1363, "L", 0.0787012668867616, 1e-7},
    {"R1336MZZ(Z)", "T", 300, "Dmass", 1, "L", 0.0110745275113003, 1e-7},
    {"R13B1", "T", 293, "Dmass", 0, "L", 0.00900702948661681, 1e-7},
    {"R13B1", "T", 293, "Dmass", 5, "L", 0.00902854295025036, 1e-7},
    {"R13B1", "T", 293, "Dmass", 1930, "L", 0.0766020002424884, 1e-7},
    {"R23", "T", 130, "Dmass", 0, "L", 0.00397477416650708, 1e-7},
    {"R23", "T", 130, "Dmass", 1663, "L", 0.182831755176538, 1e-7},
    {"R23", "T", 200, "Dmass", 5, "L", 0.00732085405389854, 1e-7},
    {"R32", "T", 300, "Dmass", 0, "L", 0.0126204119422108, 1e-7},
    {"R32", "T", 300, "Dmass", 10, "L", 0.0127955466069637, 1e-7},
    {"R32", "T", 300, "Dmass", 1100, "L", 0.16274633295305, 1e-7},
    {"RE347MCC", "T", 273, "Dmass", 0, "L", 0.0100881777804659, 1e-7},
    {"RE347MCC", "T", 273, "Dmass", 2, "L", 0.0100817151624666, 1e-7},
    {"RE347MCC", "T", 273, "Dmass", 1475, "L", 0.0709571960996124, 1e-7},
    {"XENON", "T", 300, "Dmass", 0, "L", 0.00549933595114765, 1e-7},
    {"XENON", "T", 300, "Dmass", 6, "L", 0.00553692704951132, 1e-7},
    {"XENON", "T", 300, "Dmass", 2500, "L", 0.0514002935071961, 1e-7},


    // ***********************************************************
    //                    SURFACE TENSION
    // ***********************************************************
    // (technically not a transport property, but can use same architecture)

    { "R113", "T", 300, "Q", 0.0, "I", 17.010746e-3, 1e-8 },
};

class TransportValidationFixture : public REFPROPDLLFixture
{
public:

    void payload() {
        for (auto el : transport_validation_data) {

            // Get the inputs
            CAPTURE(el.fluid);
            CAPTURE(el.in1);
            CAPTURE(el.v1);
            CAPTURE(el.in2);
            CAPTURE(el.v2);
            CAPTURE(el.out);

            int ierr = 1, nc = 1;
            char herr[255], hfld[10000] = "", hhmx[] = "HMX.BNC", href[] = "DEF";
            strcpy(hfld, el.fluid.c_str());
            SETUPdll(nc, hfld, hhmx, href, ierr, herr, 10000, 255, 3, 255);
            CAPTURE(herr);
            CHECK(ierr == 0);

            double T_K, D_molm3;
            if (el.in1 == "T" && el.in2 == "Dmolar") {
                T_K = el.v1; D_molm3 = el.v2;
            }
            else if (el.in1 == "T" && el.in2 == "Dmass") {
                double z[] = { 1.0 };
                double wm; WMOLdll(z, wm); wm /= 1000.0; // [kg/mol]
                T_K = el.v1; D_molm3 = el.v2 / wm;
            }
            else if (el.in1 == "T" && el.in2 == "P") {
                double z[] = { 1.0 }, q = 0;
                char hfld2[10000], hin[255] = "TP", hout[255] = "D", hUnits[255];
                strcpy(hfld2, el.fluid.c_str());
                int iMass = 0, iFlag = 0, iUnit;
                double Output[200], x[20], y[200], x3[20];
                REFPROPdll(hfld2, hin, hout, MOLAR_BASE_SI, iMass, iFlag, el.v1, el.v2, z, Output, hUnits, iUnit, x, y, x3, q, ierr, herr, 10000, 255, 255, 255, 255);
                CAPTURE(herr);
                CAPTURE(ierr);
                CAPTURE(Output[0]);
                CHECK(ierr < 100);
                CAPTURE("ERROR: This error code needs to be > 100!");
                T_K = el.v1; D_molm3 = Output[0];
                CHECK(D_molm3 >= 0);
            }
            else if (el.in1 == "T" && el.in2 == "Q") {
                double q = 0;
                std::vector<double> z(20, 0.0); z[0] = 1;
                char hfld2[10000], hin[255] = "TQ", hout[255] = "D", hUnits[255];
                strcpy(hfld2, el.fluid.c_str());
                int iMass = 0, iFlag = 0, iUnit;
                double Output[200], x[20], y[20], x3[20];
                REFPROPdll(hfld2, hin, hout, MOLAR_BASE_SI, iMass, iFlag, el.v1, el.v2, &(z[0]), Output, hUnits, iUnit, x, y, x3, q, ierr, herr, 10000, 255, 255, 255, 255);
                CAPTURE(herr);
                CHECK(ierr == 0);
                T_K = el.v1; D_molm3 = Output[0];
            }
            else {
                std::string err = "CANNOT use these input variables";
                CAPTURE(err);
                CHECK(false);
            }
            if (ierr > 100) {
                continue;
            }
            CAPTURE(T_K);
            CAPTURE(D_molm3);
            if (D_molm3 < 0 && ierr == 0) {
                std::string errmsg = "Can't have negative density from flash and also ierr of zero";
                CAPTURE(errmsg);
                CAPTURE(ierr);
                CAPTURE(herr);
                CHECK(false);
                continue;
            }

            {
                // Check with REFPROP function
                std::vector<double> z(20, 0.0); z[0] = 1;
                double actual = -10000000;
                REFPROPResult r;
                int iMass = 0, iFlag = 0;
                if (el.out == "L") {
                    r = REFPROP(el.fluid, "TD&", "TCX", MOLAR_BASE_SI, iMass, iFlag, T_K, D_molm3, z);
                    actual = r.Output[0]; // [W/m/K]
                }
                else if (el.out == "V") {
                    r = REFPROP(el.fluid, "TD&", "VIS", MOLAR_BASE_SI, iMass, iFlag, T_K, D_molm3, z);
                    actual = r.Output[0]; // [Pa-s]
                }
                else if (el.out == "I") {
                    r = REFPROP(el.fluid, "TD&", "STN", MOLAR_BASE_SI, iMass, iFlag, T_K, D_molm3, z);
                    actual = r.Output[0]; // N/m
                }
                else {
                    std::string err = "Output variable is invalid: " + el.out;
                    CAPTURE(err);
                    CHECK(false);
                }
                
                CAPTURE(r.ierr);
                CAPTURE(r.herr);
                CAPTURE(r.hUnits);
                CHECK(ierr < 100);
                //double eta = r.Output[0];
                //if (std::abs(eta + 9999990) < 1e-6){
                //    double rr =0;
                //}

                CAPTURE(el.expected);
                CAPTURE(actual);
                CHECK(std::abs(actual / el.expected - 1) < el.tol*fudge);
            }

            {
                // Check with TRNPRP function

                ierr = 0;
                double eta, tcx, z[] = { 1.0 };
                double D_molL = D_molm3 / 1000.0;
                TRNPRPdll(T_K, D_molL, z, eta, tcx, ierr, herr, 255);
                CAPTURE(herr);
                CHECK(ierr < 100);

                double actual = -10000000;
                if (el.out == "L") {
                    actual = tcx; // [W/m/K]
                }
                else if (el.out == "V") {
                    actual = eta / 1e6; // uPa-s to Pa-s
                }
                else if (el.out == "I") {
                    actual = el.expected; // This is to skip the TRNPRP test
                }
                else {
                    std::string err = "Output variable is invalid: " + el.out;
                    CAPTURE(err);
                    CHECK(false);
                }

                CAPTURE(el.expected);
                CAPTURE(actual);
                CHECK(std::abs(actual / el.expected - 1) < el.tol*fudge);
            }

        }
    }
};
TEST_CASE_METHOD(TransportValidationFixture, "Check transport properties against validation data", "[transport]") { payload(); };

TEST_CASE_METHOD(REFPROPDLLFixture, "Check very old fluid files with old transport models", "[transport],[oldfiles],[resources]") {
    char * RESOURCES = std::getenv("RESOURCES");
    REQUIRE(RESOURCES != nullptr);
    auto resources = normalize_path(std::string(RESOURCES) + "/TRANSPORT");
    char nopath[255] = ""; memset(nopath, 255, ' ');
    strcpy(nopath, resources.c_str());
    SETPATHdll(nopath, 255);

    auto files = get_files_in_folder(resources, ".FLD");
    REQUIRE(!files.empty());
    for (auto &&file : files){
        int ierr; std::string herr;
        SETFLUIDS(file, ierr, herr);
        CAPTURE(file);
        CAPTURE(herr);
        REQUIRE(ierr == 0);
        std::vector<double> z(20,0); z[0] = 1;
        auto r = REFPROP("","","TC;DC",0,0,0,0,0,z);
        auto Tc = r.Output[0], Dc = r.Output[1];
        auto r1 = REFPROP("", "TD&", "VIS;TCX", 0, 0, 0, Tc*1.1, Dc*1.1, z);
        CAPTURE(r1.herr);
        CHECK(r1.ierr < 100);
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check ethane test points", "[ethane]") {
    std::vector<std::tuple<double, double>> vals  =  {{0,9.285784551214537},{1,9.293014645143069},{100,12.551507244363366},{500,114.39845400414495}};
    for (auto [rho_kgm3, expected_uPas] : vals){
        CAPTURE(expected_uPas);
        std::vector<double> z(20,0); z[0] = 1;
        auto r1 = REFPROP("ETHANE", "TD", "VIS", MASS_BASE_SI, 0, 0, 300, rho_kgm3, z);
        CHECK(r1.ierr == 0);
        CHECK_THAT(r1.Output[0]*1e6, WithinRelMatcher(expected_uPas, 1e-6));
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check argon setting of ECS via SETMOD", "[transport]") {
        
    std::vector<double> z(20, 1);
    
    // base calculation, normal model
    auto r0 = REFPROP("ARGON", "TQ", "VIS", MASS_BASE_SI, 0, 0, 100, 0, z);
    
    auto [ierr, herr] = SETMOD(1, "VIS", "HMX", "ECS");
    int ierr2 = -1; std::string herr2 = ""; SETFLUIDS("ARGON", ierr2, herr2);
    
    // with ECS model
    auto r1 = REFPROP("", "TQ", "VIS", MASS_BASE_SI, 0, 0, 100, 0, z);
    CAPTURE(r0.herr);
    CHECK(r0.ierr == 0);
    CAPTURE(r1.herr);
    CHECK(r1.ierr == 0);
    CHECK(r0.Output[0] != r1.Output[0]);
};


TEST_CASE_METHOD(REFPROPDLLFixture, "Check THF test points", "[THF]") {
    SECTION("Viscosity"){
        std::vector<std::tuple<double, double>> vals = {{0,8.3705}, {900, 589.3956}};
        for (auto [rho_kgm3, expected_uPas] : vals){
            CAPTURE(expected_uPas);
            std::vector<double> z(20,0); z[0] = 1;
            auto r1 = REFPROP("THF", "TD", "VIS", MASS_BASE_SI, 0, 0, 300, rho_kgm3, z);
            CHECK(r1.ierr == 0);
            CHECK_THAT(r1.Output[0]*1e6, WithinRelMatcher(expected_uPas, 1e-5));
        }
    }
    SECTION("Conductivity"){
        std::vector<std::tuple<double, double>> vals = {{0,12.2206}, {900, 159.8654}};
        for (auto [rho_kgm3, expected_uPas] : vals){
            CAPTURE(expected_uPas);
            std::vector<double> z(20,0); z[0] = 1;
            auto r1 = REFPROP("THF", "TD", "TCX", MASS_BASE_SI, 0, 0, 300, rho_kgm3, z);
            CHECK(r1.ierr == 0);
            CHECK_THAT(r1.Output[0]*1e3, WithinRelMatcher(expected_uPas, 1e-5));
        }
    }
};


TEST_CASE_METHOD(REFPROPDLLFixture, "Check mixtures give warning for transport", "[transport],[mixtures]") {
    
    int ierr; std::string herr;
    SETFLUIDS("CO2 * SO2", ierr, herr);
    std::vector<double> z(20,0); z[0] = 0.4; z[1] = 0.6;
    auto r1 = REFPROP("", "TD&", "VIS;TCX", 0, 0, 0, 300, 300, z);
    CHECK(r1.ierr < 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Ddil", "[Ddil]") {
    // At this state point, experimental value is etaexp = 10.8 uPa*s
    double T_K = 293.138, p_MPa = 0.101, rho_kgm3 = 0.40648, xmole_H2 = 0.4823;
    std::vector<double> z(20, 0.0); z[0] = xmole_H2; z[1] = 1 - xmole_H2;

    char* RESOURCES = std::getenv("RESOURCES");
    REQUIRE(RESOURCES != nullptr);

    // Map from HMX file to expected result
    std::unordered_map<std::string, double> value_map{
        {"HMX10.0_H2NH3.BNC", 10.603},
        {"HMX10.0_H2NH3_correctedDdil.BNC", 10.892},
        {"HMX10.0_H2NH3_incorrectDdil.BNC", 11.479},
        {"HMX10.0_H2NH3_Ddilordering.BNC", 10.892},
    };
    int kflag = -999;
    for (auto &pair : value_map) {
        int ierr = 0; std::string herr;
        std::string HMX_path = std::string(RESOURCES) + "/Ddil/" + pair.first;
        FLAGS("Reset HMX", 1, kflag, false /*check_kflag*/); 
        SETUP(2, "H2*NH3", HMX_path, "DEF", ierr, herr);
        REQUIRE(std::filesystem::exists(HMX_path));
        CAPTURE(HMX_path);
        CAPTURE(herr);
        CHECK(ierr == 0);
        double expected = pair.second;
        auto r = REFPROP("", "TP", "VIS;D;TCX", MASS_BASE_SI, 0, 0, T_K, p_MPa * 1e6, z);
        double eta_calc = r.Output[0]*1e6;
        //double rho_calc = r.Output[1];
        double lambda_calc = r.Output[1];
        CHECK_THAT(eta_calc, WithinAbsMatcher(expected, 0.001));
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "helium viscosity regression", "[transport]") {
    int MassSI = get_enum("Mass SI"); // unit in MASS SI
    int iMass = 0; // 0: molar fraction, 1 : mass fraction, in this code we only use pure fluid, 0 or 1 either OK.
    int iFlag = 0; // 0: don't call SATSPLN, 1: call SATSPLN
    std::vector<double> z (20, 1.0);

    auto r = REFPROP("helium", "TP", "VIS", MassSI, iMass, iFlag, 300, 1.0, z);
    CHECK_THAT(r.Output[0], WithinAbsMatcher(20.0, 1.0));
}
