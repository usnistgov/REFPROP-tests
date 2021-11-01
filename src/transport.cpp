/*

Some of these tests are based on the tests implemented in the CoolProp program (www.coolprop.org)

*/

#include <cstdlib>

#include "catch.hpp"

#include "REFPROPtests/baseclasses.h"
#include <boost/filesystem.hpp>

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

    // From Lemmon and Jacobsen, JPCRD, 2004
    {"Nitrogen", "T", 100, "Dmolar", 1e-14, "V", 6.90349e-6, 1e-3},
    {"Nitrogen", "T", 300, "Dmolar", 1e-14, "V", 17.8771e-6, 1e-3},
    {"Nitrogen", "T", 100, "Dmolar", 25000, "V", 79.7418e-6, 1e-3},
    {"Nitrogen", "T", 200, "Dmolar", 10000, "V", 21.0810e-6, 1e-3},
    {"Nitrogen", "T", 300, "Dmolar", 5000, "V", 20.7430e-6, 1e-3},
    {"Nitrogen", "T", 126.195, "Dmolar", 11180, "V", 18.2978e-6, 1e-3},
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

    // Quinones-Cisneros, JCED, 2012, data from validation
    {"H2S", "T", 200, "P", 1000e5, "V", 0.000460287, 1e-3},
    {"H2S", "T", 200, "P", 0.251702e5, "V", 8.02322E-06, 1e-3},
    {"H2S", "T", 596.961, "P", 1000e5, "V", 6.94741E-05, 1e-3},
    {"H2S", "T", 596.961, "P", 1e5, "V", 2.38654E-05, 1e-3},

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

    // From Shan, ASHRAE, 2000
    {"R23", "T", 180, "Dmolar", 21097, "V", 353.88e-6, 1e-4},
    {"R23", "T", 420, "Dmolar", 7564, "V", 39.459e-6, 1e-4},
    {"R23", "T", 370, "Dmolar", 32.62, "V", 18.213e-6, 1e-4},

    // From Friend, JPCRD, 1991
    {"Ethane", "T", 100, "Dmolar", 21330, "V", 878.6e-6, 1e-2},
    {"Ethane", "T", 430, "Dmolar", 12780, "V", 58.70e-6, 1e-2},
    {"Ethane", "T", 500, "Dmolar", 11210, "V", 48.34e-6, 1e-2},

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

    // From Shan, ASHRAE, 2000
    { "R23", "T", 180, "Dmolar", 21097, "L", 143.19e-3, 1e-4 },
    { "R23", "T", 420, "Dmolar", 7564, "L", 50.19e-3, 2e-4 },
    { "R23", "T", 370, "Dmolar", 32.62, "L", 17.455e-3, 1e-4 },

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

    // Heavy Water, IAPWS formulation
    { "HeavyWater", "T", 0.5000*643.847, "Dmass", 3.07 * 358, "L", 835.786416818*0.742128e-3, 1e-5 },
    { "HeavyWater", "T", 0.9000*643.847, "Dmass", 2.16 * 358, "L", 627.777590127*0.742128e-3, 1e-5 },
    { "HeavyWater", "T", 1.2000*643.847, "Dmass", 0.8 * 358, "L", 259.605241187*0.742128e-3, 1e-5 },

    // Vassiliou, JPCRD, 2015
    { "Cyclopentane", "T", 512, "Dmass", 1e-12, "L", 37.042e-3, 1e-5 },
    { "Cyclopentane", "T", 512, "Dmass", 400, "L", 69.698e-3, 1e-1 },
    { "Isopentane", "T", 460, "Dmass", 1e-12, "L", 35.883e-3, 1e-4 },
    { "Isopentane", "T", 460, "Dmass", 329.914, "L", 59.649e-3, 1e-1 },
    { "n-Pentane", "T", 460, "Dmass", 1e-12, "L", 34.048e-3, 1e-5 },
    { "n-Pentane", "T", 460, "Dmass", 377.687, "L", 71.300e-3, 1e-1 },

};

class TransportValidationFixture : public REFPROPDLLFixture
{
public:

    void payload() {
        int MOLAR_BASE_SI = get_enum("MOLAR BASE SI   ");
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
                    r = REFPROP(el.fluid, "TD&", "ETA", MOLAR_BASE_SI, iMass, iFlag, T_K, D_molm3, z);
                    actual = r.Output[0]; // [Pa-s]
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
        auto r1 = REFPROP("", "TD&", "ETA;TCX", 0, 0, 0, Tc*1.1, Dc*1.1, z);
        CAPTURE(r1.herr);
        CHECK(r1.ierr < 100);
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check mixtures give warning for transport", "[transport],[mixtures]") {
    
    int ierr; std::string herr;
    SETFLUIDS("CO2 * SO2", ierr, herr);
    std::vector<double> z(20,0); z[0] = 0.4; z[1] = 0.6;
    auto r1 = REFPROP("", "TD&", "ETA;TCX", 0, 0, 0, 300, 300, z);
    CHECK(r1.ierr < 0);
};