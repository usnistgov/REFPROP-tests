#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
using Catch::Matchers::WithinAbsMatcher;
using Catch::Matchers::WithinRelMatcher;

// Approx() is deprecated and should be removed, but
// no good drop-in solution is available
#include <catch2/catch_approx.hpp>
using Catch::Approx;

#include <utility>
#include "REFPROPtests/baseclasses.h"

// A structure to hold the values for one validation call
struct XLSin
{
    std::string OutputCode, FluidName, InpCode, Units;
    double Prop1, Prop2, Prop3;
    int Ninputs;
    XLSin() : Prop1(-19999999999), Prop2(-999999999), Prop3(-999999999), Ninputs(2), Units("DEFAULT"), OutputCode(""), FluidName(""), InpCode("") {};
    XLSin(const std::string &OutputCode, const std::string &FluidName) : OutputCode(OutputCode), FluidName(FluidName), Prop1(-9999999999), Prop2(-999999999), Prop3(-999999999), Ninputs(2), Units("DEFAULT") {};
    template<typename T1> XLSin(const std::string &OutputCode, const std::string &FluidName, T1 Prop1) : OutputCode(OutputCode), FluidName(FluidName), Prop1(Prop1), Prop2(-99999999), Prop3(-99999999), Ninputs(3) {};
    template<typename T1> XLSin(const std::string &OutputCode, const std::string &FluidName, const std::string &InpCode, const std::string &Units, T1 Prop1) : OutputCode(OutputCode), FluidName(FluidName), InpCode(InpCode), Units(Units), Prop1(Prop1), Prop2(-99999999), Prop3(-99999999), Ninputs(5) {};
    template<typename T1, typename T2> XLSin(const std::string &OutputCode, const std::string &FluidName, const std::string &InpCode, const std::string &Units, T1 Prop1, T2 Prop2) : OutputCode(OutputCode), FluidName(FluidName), InpCode(InpCode), Units(Units), Prop1(Prop1), Prop2(Prop2), Prop3(-99999999), Ninputs(6) {};
    template<typename T1, typename T2, typename T3> XLSin(const std::string &OutputCode, const std::string &FluidName, const std::string &InpCode, const std::string &Units, T1 Prop1, T2 Prop2, T3 Prop3) : OutputCode(OutputCode), FluidName(FluidName), InpCode(InpCode), Units(Units), Prop1(Prop1), Prop2(Prop2), Prop3(Prop3), Ninputs(7) {};
};

class XLSChecksumFixture : public REFPROPDLLFixture {

public:
    std::vector<std::pair<double, XLSin>> inputs;

    XLSChecksumFixture(){

        inputs = {
            { 28.96546   ,{ "M","Air" } },
            { 637.3775887,{ "T","argon","PD","SI",2,15 } },
            { 1.456916965,{ "P","r134a","TD","SI",400,50 } },
            { 684.9965172,{ "H","ethylene","TS","SI",370,2.5 } },
            { 153.8866807,{ "VIS","oxygen","TP","SI",100,1 } },
            { 100.1117462,{ "TCX","nitrogen","Tliq","SI",100 } },
            { 0.07493553 ,{ "D","air","TP","E",70,14.7 } },
            { 23643.99395,{ "H","R32;R125|0.3;0.7","PS","molar si",10,110 } },
            { 298.4314139,{ "T","ethane;0.5;butane;0.5 mass","DH","E",30,283 } },
            { 5532.424329,{ "W","ammonia;0.4;water;0.6","TP","E",300,10000 } },
            { 1.600404294,{ "D","r218;.1;r123;.9","PH","cgs",7,180 } },
            { 0.038640593,{ "QMASS","methane;40;ethane;60 mass","TD","mks",200,300 } },
            { 0.038640617,{ "QMASS","methane;40;ethane;60 mass","TP","mks",200,2814.5509 } },
            { 0.050092664,{ "QMOLE","methane;ethane|.4;.6 mass","TP","molar SI",200,2.8145509 } },
            { 280.2337991,{ "VIS","butane;hexane|.25;.75","TH>","SI",300,-21 } },
            { 17.89148313,{ "TCX","carbon dioxide;nitrogen|.5;.5 mass","TS","molar SI",250,230.7 } },
            { 1.037058033,{ "DE","ethane;.5;propane;.5","tvap","SI",300 } },
            { 0.697614699,{ "xmole","R410A.MIX",1 } },
            { 0.5        ,{ "XMASS","R410A.MIX",1 } },
            { 440.2592921,{ "DLIQ","methane;ethane;propane;butane|0.8;0.15;0.03;0.02","TD","SI",150,200 } },
            { 49.71894258,{ "F(2)","propane;R125|10;90","TH>","E",-20,72 } },
            { 0.89854362 ,{ "FC(2)","N2;CO2|0.6;0.4","TQ","MKS",200,1 } },
            { 0.231604716,{ "xmass","air.mix","TD","molar SI",80,12,3 } },
            { 210.9797558,{ "T","butane","Ssat3","SI",2.5 } },
            { 17.30446997,{ "DLIQ","R410A.mix","TD","molar SI",250,10 } },
            { 0.768067188,{ "D","butane;propadiene|0.7;0.3 mass","TS","E",150,0.6 } },
            { 0.567871366,{ "dHdT_D","argon","TP","MIXED",150,200 } },
            { 999.9251311,{ "D","water","Tliq","C",4 } },
            { 0.004968591,{ "P","hexane;butane|.6;.4 mass","DE","SI",600,-120 } },
            { 33.24750174,{ "vis","argon;co2;propane;acetylene|0.8;0.15;0.03;0.02 mass","TD","SI",450,200 } },
            { 0.77364522 ,{ "qmass","argon;co2;propane;acetylene|0.8;0.15;0.03;0.02 mass","TH<","SI",160,55 } },
            { 369.89     ,{ "TCRIT(3)","methane;ethane;propane;butane|0.8;0.15;0.03;0.02" } },
            { 246.1166525,{ "P","MM;MDM;MD3M|0.8;0.15;0.05","TH<","E",500,177 } },
            { 30566.38807,{ "E","D4;D5;D6|0.8;0.15;0.05","TD","molar si",500,1 } },
            { 37.16306405,{ "E","XENON","HS","SI",44,0.175 } },
            { 14.2793971 ,{ "JT","CO2;WATER|0.5;0.5","Tvap","SI",400 } }
        };

        std::vector<double> zz(1, 1);
        auto res = REFPROP("", "FLAGS", "SETREF", 0, 0, 2, 0, 0, zz);
        //int k = -1;
        //FLAGS("SETREF", 2, k, true);
        //CHECK(res.ierr == 0);
    }

    void payload() {

        for (auto &theinput : inputs) {
            double expected; XLSin in;
            std::tie(expected, in) = theinput;
            switch (in.Ninputs) {
                case 2:
                {   
                    int iMass = 0, iFlag = 0;
                    int unit_system = get_enum("DEFAULT");
                    CAPTURE(in.FluidName);
                    CAPTURE(in.OutputCode);
                    CAPTURE(in.Units);
                    double a = in.Prop1, b = in.Prop2;
                    std::vector<double> z(20,0.0);
                    CAPTURE(unit_system);
                    auto r = REFPROP(in.FluidName, "", in.OutputCode, unit_system, iMass, iFlag, a, b, z);
                    CAPTURE(r.herr);
                    CHECK(r.ierr < 100);
                    CHECK(r.Output[0] == Approx(expected).epsilon(1e-5));
                    break;
                }
                case 3:
                {
                    int iMass = 0, iFlag = 0;
                    double a = in.Prop1, b = in.Prop2; std::vector<double> z(20, 0.0);
                    int unit_system = get_enum("DEFAULT");
                    CAPTURE(in.Units);
                    CAPTURE(unit_system);
                    std::string outString = in.OutputCode + "(" + std::to_string(int(in.Prop1)) + ")";
                    CAPTURE(outString);
                    auto r = REFPROP(in.FluidName, "", outString, unit_system, iMass, iFlag, a, b, z);
                    CAPTURE(r.herr);
                    CHECK(r.ierr < 100);
                    CHECK(r.Output[0] == Approx(expected).epsilon(1e-5));
                    break;
                }
                case 5:
                case 6:
                case 7:
                {
                    std::string outString = in.OutputCode;
                    if (in.Ninputs == 7){
                        outString += "(" + std::to_string(int(in.Prop3)) + ")";
                    }
                    int iMass = 0, iFlag = 0;
                    double a = in.Prop1, b = in.Prop2; std::vector<double> z(20, -1.0);
                    int unit_system  = get_enum(in.Units);
                    CAPTURE(unit_system);
                    CAPTURE(in.FluidName);
                    CAPTURE(in.InpCode);
                    CAPTURE(outString);
                    CAPTURE(in.Ninputs);
                    CAPTURE(in.Units);
                    CAPTURE(a);
                    CAPTURE(b);

                    // Check that the flag for resetting of reference state is still enabled
                    int kSETREF = -1;
                    FLAGS("SETREF", -999, kSETREF, true);
                    CHECK(kSETREF == 2);

                    if (in.Ninputs == 7) {
                        CAPTURE(in.Prop3);
                    }
                    auto r = REFPROP(in.FluidName, in.InpCode, outString, unit_system, iMass, iFlag, a, b, z);
                    CAPTURE(r.herr);
                    if (r.ierr > 100) {
                        int rrrr = 0;
                    }
                    CHECK(r.ierr < 100);
                    CHECK(r.Output[0] == Approx(expected).epsilon(1e-5));
                    break;
                }
                default: {
                    throw "Bad number of arguments";
                }
            }
        }      
    }
};
TEST_CASE_METHOD(XLSChecksumFixture, "Check checksum calculations from the Excel spreadsheet", "[Excel]") { payload(); };
