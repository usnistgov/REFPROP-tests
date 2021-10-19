#include <cstdlib>
#include <boost/algorithm/string/trim.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "REFPROPtests/baseclasses.h"
#include "REFPROPtests/utils.h"
#include <numeric>

TEST_CASE_METHOD(REFPROPDLLFixture, "Version", "[setup]") {
    char hpath[10001] = "";
    RPVersion(hpath, 10000);
    CHECK(hpath[0] != '*');
    std::vector<double> z(20, 0.0); 
    auto r = REFPROP("", "", "DLL#", DEFAULT, 0, 0, 0, 0, z);
    CHECK(hpath == r.herr);
}

TEST_CASE_METHOD(REFPROPDLLFixture, "R + 0 = Total?", "[flash],[rplus0]") {
    std::vector<double> z(20, 0.0);
    auto rR = REFPROP("Water","TP","PR;ER;HR;SR;CVR;CPR;AR;GR",0,0,0,298, 101.325, z );
    auto r0 = REFPROP("Water", "TP", "P0;E0;H0;S0;CV0;CP0;A0;G0", 0, 0, 0, 298, 101.325, z);
    auto rT = REFPROP("Water", "TP", "P;E;H;S;CV;CP;A;G", 0, 0, 0, 298, 101.325, z);
    // Check they add up to the total
    for (auto i = 0; i < 8; ++i) {
        CHECK(rT.Output[i] == Approx(rR.Output[i] + r0.Output[i]));
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Check NBP of water in SI unit systems", "[nbp]"){
    double T_K = 373.1242958477, T_C = T_K - 273.15;
    std::vector<double> z(20,0.0);
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("DEFAULT"), 0, 0, 101.325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("MOLAR SI"), 0, 0, 0.101325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("MASS SI"), 0, 0, 0.101325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("SI WITH C"), 0, 0, 0.101325, 0, z).Output[0] == Approx(T_C));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("MOLAR BASE SI"), 0, 0, 101325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("MASS BASE SI"), 0, 0, 101325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("MKS"), 0, 0, 101.325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("CGS"), 0, 0, 0.101325, 0, z).Output[0] == Approx(T_K));
    CHECK(REFPROP("WATER", "PQ", "T", get_enum("MEUNITS"), 0, 0, 1.01325, 0, z).Output[0] == Approx(T_C));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check spinodals", "[spinodal]") {
    std::vector<double> z(20, 0.0); z[1] = 0;
    double p = 101.325;
    auto r = REFPROP("WATER", "PQ", "DLIQ;LIQSPNDL;VAPSPNDL;DVAP;T",DEFAULT, 0, 0, p, 0, z);
    double dliq = r.Output[0], liqspndl = r.Output[1], vapspndl = r.Output[2], dvap = r.Output[3], T = r.Output[4]; 
    double p_vapspndl = REFPROP("WATER", "TD&", "P", DEFAULT, 0, 0, T, vapspndl, z).Output[0];
    double p_vapspndl2 = REFPROP("WATER", "DT&", "P", DEFAULT, 0, 0, vapspndl, T, z).Output[0];
    double p_liqspndl = REFPROP("WATER", "TD&", "P", DEFAULT, 0, 0, T, liqspndl, z).Output[0];
    REQUIRE(p_liqspndl < p);
    REQUIRE(p_vapspndl > p);
    //REQUIRE(p_vapspndl2 == Approx(p_vapspndl)); // TODO: this should be possible, but p < 0
    
    //auto T1 = REFPROP("WATER", "PD>", "T", DEFAULT, 0, 0, p_liqspndl, liqspndl, z).Output[0];
    auto T2 = REFPROP("WATER", "PD<", "T", DEFAULT, 0, 0, p_vapspndl, vapspndl, z).Output[0];
    //auto T1b = REFPROP("WATER", "DP<", "T", DEFAULT, 0, 0, liqspndl, p_liqspndl, z).Output[0];
    auto T2b = REFPROP("WATER", "DP>", "T", DEFAULT, 0, 0, vapspndl, p_vapspndl, z).Output[0];
    //REQUIRE(T1 == Approx(T));
    REQUIRE(T2 == Approx(T));
    //REQUIRE(T1b == Approx(T));
    REQUIRE(T2b == Approx(T));
    
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Try to load all predefined mixtures", "[setup],[predef_mixes]") {
    for (auto &&mix : get_predefined_mixtures_list()) {
        // Load it
        std::vector<double> z(20, 0.0);
        auto r = REFPROP(mix+".MIX", " ", "TRED", 1, 0, 0, 0.101325, 300, z);
        CAPTURE(mix+".MIX");
        CAPTURE(r.herr);
        CHECK(r.ierr < 100);
        // Get the predefined mixture critical values
        auto vals = get_predef_mix_values(mix+".MIX");
        // Turn on splines
        int ierr = 0; char herr[255] = "";
        SATSPLNdll(&(z[0]), ierr, herr, 255U);
        // Get critical point
        double Tcspl = -1, Pcspl = -1, Dcspl = -1;
        double Wmol; WMOLdll(&(z[0]), Wmol);
        CRITPdll(&(z[0]),Tcspl,Pcspl,Dcspl,ierr,herr,255U);
        CHECK(vals.Wmol == Approx(Wmol).epsilon(1e-2)); 
        CHECK(vals.Tc == Approx(Tcspl).epsilon(1e-2));
        CHECK(vals.pc == Approx(Pcspl).epsilon(1e-2));
        CHECK(vals.rhoc == Approx(Dcspl).epsilon(1e-2));
        // Check molar composition matches what we loaded
        for (auto i = 0; i < vals.molar_composition.size(); ++i) {
            CHECK(z[i] == Approx(vals.molar_composition[i]));
        }
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "CRITP w/o splines for all predefined mixtures", "[setup],[predef_mixes],[crit]") {
    for (auto &&mix : get_predefined_mixtures_list()) {
        // Load it
        std::vector<double> z(20, 0.0);
        auto r = REFPROP(mix + ".MIX", " ", "TRED", 1, 0, 0, 0.101325, 300, z);
        CAPTURE(mix + ".MIX");
        CAPTURE(r.herr);
        CHECK(r.ierr < 100);
        
        // Get critical point
        int ierr = 0; char herr[255] = ""; 
        double Tc = -1, Pc = -1, Dc = -1;
        CRITPdll(&(z[0]), Tc, Pc, Dc, ierr, herr, 255U);
        CAPTURE(herr);
        CHECK(ierr < 0);
    }
};


TEST_CASE_METHOD(REFPROPDLLFixture, "Sanity check phase equilibrium", "[flash],[fug]") {
    std::vector<double>z(20,0); z[0] = 0.4, z[1] = 0.6;
    // Flash call
    auto r0 = REFPROP("Propane * R125","PQ","T;DLIQ;DVAP",DEFAULT, 0,0,101.325,0,z);
    double T = r0.Output[0], dL = r0.Output[1], dV = r0.Output[2];
    // Check fugacity, chem. pot., and fug. coeff match as they should
    auto rL = REFPROP("", "TD&", "CPOT(1);CPOT(2);F(1);F(2);FC(1);FC(2)",DEFAULT,0,0,T,dL,r0.x);
    auto rV = REFPROP("", "TD&", "CPOT(1);CPOT(2);F(1);F(2);FC(1);FC(2)",DEFAULT,0,0,T,dV,r0.y);
    CHECK(rL.Output[0] == Approx(rV.Output[0]));
    CHECK(rL.Output[1] == Approx(rV.Output[1])); 
    CHECK(rL.Output[2] == Approx(rV.Output[2]));
    CHECK(rL.Output[3] == Approx(rV.Output[3]));
    CHECK(rL.Output[4]*r0.x[0] == Approx(rV.Output[4]*r0.y[0]));
    CHECK(rL.Output[5]*r0.x[1] == Approx(rV.Output[5]*r0.y[1]));
};

struct PRTvalue{
    std::string name;
    double P, e, h, s, Cv, Cp, w, hjt;
};

class PRTValues : public REFPROPDLLFixture {

public:
    std::vector<PRTvalue> values;

    PRTValues(){
        values = {
            { "Water", 24.849011529434357,43449.4350971244,45934.336250067834,137.24630883849454,25.29228974284363,33.70328569334078,427.90770560908226,0.02852920990531785 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "R134a", 24.8297546904379,41191.02415542999,43673.99962447378,206.31902919360374,77.0807371072858,85.54179472961884,163.96055923048175,0.015666800549275234 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "R125", 24.848731296316714,41163.91989090856,43648.79302054023,217.74451880648144,86.49051235401214,94.93381617090002,150.4600510728894,0.012179047769408409 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "R32", 24.876153775680027,26961.15254024502,29448.767917813024,149.59931875512206,34.784226309000374,43.18393387427896,243.3169468298829,0.01813780009219237 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "Oxygen", 24.938603099055026,6238.24162408063,8732.101933986132,216.87503233905244,21.07254086089509,29.398820192951575,329.7113102631311,0.003071020272960896 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "Nitrogen", 24.941723116750076,6227.915504883862,8722.08781655887,203.32990053328456,20.81403263601845,29.13934130581091,353.0423077877676,0.002468394235469467 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "IOCTANE", 24.445822239809036,13665.89662033258,16110.478844313484,50.43996038041584,177.57991212038976,186.4512376933278,148.3707569544151,0.029045883953275713 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "Hydrogen", 24.946371636674385,5525.970735110894,8020.607898778332,121.53346303402857,20.532877602547682,28.849052980223362,1318.6559681475019,-5.576784543245538e-05 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "Helium", 24.945886220468637,3756.9441974451415,6251.532819492006,122.80068252087574,12.471618020207677,20.786588609482052,1019.2485401287822,-0.0003322784782630463 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "Ethanol", 24.72790858647901,33316.71909994209,35789.509958589995,112.89382941263747,57.222678122802726,65.80655942505888,247.36832497689073,0.037930151168937234 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "C12", 23.573097591817913,-24224.578032844598,-21867.268273662805,-66.13417062717195,271.61018443293597,281.6162744362908,116.27173045790552,0.05773099298887043 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
            { "Argon", 24.938669888688068,3739.3982388134787,6233.2652276822855,166.51603474739923,12.473569687651963,20.799476356941305,322.6106183915966,0.00422011713703071 }, //  P,e,h,s,Cv,Cp,w,hjt @ T=300K, D=0.01 mol/L
        };
    };
    void payload() {
        for (auto &val : values) {
            int nc = 1;
            {
                int ierr = 0;
                char h1[4] = "EOS", h2[4] = "NBS", h3[255] = "PRT", herr[255] = "";
                SETMODdll(nc, h1, h2, h3, ierr, herr, 3, 3, 255, 255);
            }
            std::vector<double> z(20, 0.0); z[0] = 1;
            auto r = REFPROP(val.name, "TD&", "P;E;H;S;CV;CP;W;JT", DEFAULT, 0, 0, 300, 0.01, z);
            CAPTURE(r.herr);
            CAPTURE(r.ierr);
            CAPTURE(val.name);
            CHECK(r.Output[0] == Approx(val.P).epsilon(1e-2));
            CHECK(r.Output[1] == Approx(val.e).epsilon(1e-2));
            CHECK(r.Output[2] == Approx(val.h).epsilon(1e-2));
            CHECK(r.Output[3] == Approx(val.s).epsilon(1e-2));
            CHECK(r.Output[4] == Approx(val.Cv).epsilon(1e-2));
            CHECK(r.Output[5] == Approx(val.Cp).epsilon(1e-2));
            CHECK(r.Output[6] == Approx(val.w).epsilon(1e-2));
            CHECK(r.Output[7] == Approx(val.hjt).epsilon(1e-2));
        }
    }
};
TEST_CASE_METHOD(PRTValues, "CHECK 9.1.1 values w/ PRT model", "[911],[PRT]") { payload(); }

TEST_CASE_METHOD(REFPROPDLLFixture, "CHECK 9.1.1 values w/ 4-component refrigerant mixture", "[flash],[911]") {
    std::vector<double> z(20, 0.0); for (auto i = 0; i < 4; ++i){ z[i] = 0.25; }
    std::vector<double> vals_911 = { 1623.7467678192363, 33190.76537542725, 34814.51214324648, 156.16069343722648, 90.2775245338805, 162.39122497807574, 128.3393178811631, 0.025476123512775832 }; // p,e,h,s,cv,cp,w,htj @ 300 K, 1.0 mol/L
    std::vector<std::string> keys = { "p","e","h","s","cv","cp","w", "JT" };
    REQUIRE(std::accumulate(z.begin(), z.end(), 0.0) == Approx(1.0));
    {
        reload();
        INFO("Default flags w/ Rgas=2");
        auto r0 = REFPROP("R32 * R125 * R134A * R143A", "TD&", "W", 0, 0, 0, 300, 1.0, z);
        REQUIRE(r0.ierr == 0);
        int k = -1;
        FLAGS("GAS CONSTANT", 2, k);

        for (auto i = 0; i < keys.size(); ++i) {
            CAPTURE(keys[i]);
            auto r = REFPROP("", "TD&", keys[i]+";R", 0, 0, 0, 300, 1.0, z);
            REQUIRE(r.ierr == 0);
            double R = r.Output[1];
            CAPTURE(R);
            auto v = vals_911[i];
            CHECK(r.Output[0] == Approx(v));
        }
    }
    {
        reload();
        INFO("w/ PX0=1 & w/ Rgas=2");
        auto r0 = REFPROP("R32 * R125 * R134A * R143A * R152A", "", "", 0, 0, 0, 300, 1.0, z); 
        CAPTURE(r0.herr);
        REQUIRE(r0.ierr == 0);
        int k = -1;
        FLAGS("PX0", 1, k);
        FLAGS("GAS CONSTANT", 2, k);
        for (auto i = 0; i < keys.size(); ++i) {
            CAPTURE(keys[i]);
            auto r = REFPROP("", "TD&", keys[i], 0, 0, 0, 300, 1.0, z);
            REQUIRE(r.ierr == 0);
            auto v = vals_911[i];
            CHECK(r.Output[0] == Approx(v));
        }
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "CHECK values from GUI", "[flash],[911]") {
    struct Value {
        double v;
        std::string k;
    };
    std::vector<Value> vals = {
        {300, "T"},
        { 0.1,"P" },
        { 0.040642,"D" },
        { 24.605,"V" },
        { 31858,"E" },
        { 34318,"H" },
        { 163.42,"S" },
        { 76.289,"CV" },
        { 85.147,"CP" },
        { 84.24,"CP0" },
        { 1.1161,"CP/CV" },
        { 181.68,"W" },
        { 0.98643,"Z" },
        { 13.426,"JT" },
        { -0.3348,"Bvir" },
        { 0.020927,"Cvir" },
        { 0.0043423,"Dvir" },
        { -17167,"A" },
        { -14707,"G" },
        { 0.049299,"F(1)" },
        { 0.04936,"F(2)" },
        { 0.98598,"FC(1)" },
        { 0.98721,"FC(2)" },
        { -10522.5553945935,"CPOT(1)" },
        { -18891.2293993222,"CPOT(2)" },
        { 82.059,"M" },
        { -0.29569,"B12" },
        { 16.522,"TCX" },
        { 11.273, "ETA" },
        { 0.0338,"KV" },
        { 0.047744,"TD" },
        { 0.70796,"PRANDTL" },
        { 1109500,"HG" },
        { 1095200,"HGLQ" },
        { 44479e-3,"HGVOL" },
        { 1010600,"HN" },
        { 996370,"HNLQ" },
        { 40515e-3,"HNVOL" },
        { 2.8333,"RDAIR" },
        { 0.0033381,"RDH2O" },
        { 42259,"API" },
        { 10.139,"KAPPA" },
        { 0.0034882,"BETA" },
        { 1.1008,"ISENK" },
        { 0.98628,"KT" },
        { 9.0843,"BETAS" },
        { 0.11008,"BS" },
        { 0.098628,"KKT" },
        { -1.1432,"THROTT" },
        { 2.4267,"DPDD" },
        { -1.6573,"D2PDD2" },
        { 0.00034403,"DPDT" },
        { -4.9391E-08,"D2PDT2" },
        { 0.0086156,"D2PDTD" },
        { -0.00014177,"DDDT" },
        { 0.41208,"DDDP" },
        { 1.0407E-06,"D2DDT2" },
        { 0.11597,"D2DDP2" },
        { -0.0015029,"D2DDPT" },
        { 0.0025964,"DBVIRDT" },
        { 84.754,"DHDT_D" },
        { 85.147,"DHDT_P" },
        { -2774.1,"DHDD_T" },
        { -600600,"DHDD_P" },
        { -1143.2,"DHDP_T" },
        { 246350,"DHDP_D" },
        { -0.52744,"BA" },
        { 0.071385,"CA" },
        { -0.0033333,"N1/T" },
        { 2.4605,"P*V" },
        { -0.13574,"(Z-1)/P" },
        { -0.33398,"(Z-1)/D" },
        { 11383,"EXERGY" },
        { 8946.8,"CEXERGY" },
        { 24410,"SPHT" },
        { 0.99892,"FPV" },
        { 0.63372,"CSTAR" },
        { 0.1,"P" },
        { 34318,"H" },
        { 163.42,"S" },
        { -17167,"A" },
        { -14707,"G" },
        { 0.98643,"Z" },
        { 0.00321,"PINT" },
        { 0.10321,"PREP" },
        { -0.00321,"PATT" },
        // { 0.040091,"D0" }, // [TODO]
        { 34431,"H0" },
        { 163.57,"S0" },
        { -14639,"G0" },
        { -17133,"A0" },
        //{ -6.8825,"ALPHA" }, // [TODO]
        { -6.8689,"PHIG00" },
        { -0.01359,"PHIR00" },
        { -0.031661,"PHIR10" },
        { -0.043721,"PHIR20" },
        { -0.06136,"PHIR30" },
        { -0.013572,"PHIR01" },
        { 0.000035131,"PHIR02" },
        { 5.4486E-07,"PHIR03" },
        { -0.031664,"PHIR11" },
        { -5.1392E-06,"PHIR12" },
        { -0.043848,"PHIR21" },
        { -0.0002494,"PHIR22" },
        { -0.061419,"PHIR31" },
        { 3.0933E-06,"PHIR13" },
        { 0.00019107,"PHIR32" },
        { 0.000013914,"PHIR23" },
        { 0.00090613,"PHIR33" },
        { -0.0013761,"PR" },
        { 0.11096,"GRUN" },
        { 0.95443,"PIP" },
        { 0.077751,"RIEM" },
        { 0.040316,"VE" },
        { 8.9995,"EE" },
        { 13.031,"HE" },
        { 0.030202,"SE" },
        { -0.060951,"AE" },
        { 3.9706,"GE" },
        //{ -0.11159,"(T-TC)/TC" }, // [TODO]
        { 363.48,"TMF" },
        { 6.6416,"S*D" },
        { 0.5,"XMOLE(1)" },
        { 0.5,"XMOLE(2)" }
    };
    int kflag = 0; FLAGS("SETREF",2,kflag);
    for (auto &&val : vals){
        std::vector<double> z(20,0); z[0] = 0.5; z[1] = 0.5;
        auto r = REFPROP("Propane * R125","TP",val.k,MOLAR_SI, 0,0,300, 0.1, z);
        CAPTURE(r.hUnits);
        CAPTURE(val.v);
        CAPTURE(val.k);
        CHECK(val.v == Approx(r.Output[0]).epsilon(1e-2));
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Uninitialized ierr", "[ierr]") {
    std::vector<double> z(20, 0); z[0] = 1.0;
    auto r = REFPROP("1-Butyne", "DP", "T;P;S;H;S", 2, 0, 0, 300, 4.2, z);
    CAPTURE(r.hUnits);
    CHECK(r.ierr == -1);
}

TEST_CASE_METHOD(REFPROPDLLFixture, "DSAT compositions", "[DSAT]") {
    auto mix_str = "AIR.MIX";
    auto m = SETMIXTURE(mix_str);
    int UNITS_SI = get_enum("SI");
    auto [splierr, splherr] = SATSPLN(std::get<0>(m));
    auto density = 301.0;
    auto r = REFPROP("", "DSAT", "T;DLIQ;DVAP", UNITS_SI, 0, 0, density, 0, std::get<0>(m));
    CAPTURE(r.hUnits);
    auto T = r.Output[0], Dliq = r.Output[1], Dvap = r.Output[2];
    CHECK(std::abs(Dliq - Dvap) > 1);
}

TEST_CASE_METHOD(REFPROPDLLFixture, "DSAT nitrogen", "[DSAT]") {

    // At the triple point, get the densities
    std::vector<double> z(20, 0.0);
    auto rTRP = REFPROP("NITROGEN", "TRIP", "DLIQ;DVAP", MOLAR_BASE_SI, 0, 0, 0, 0, z);
    REQUIRE(rTRP.ierr == 0);
    auto Dmin = rTRP.Output[1], Dmax = rTRP.Output[0];
    for (auto density : linspace(Dmin, Dmax, 100)){
        auto rD = REFPROP("", "DSAT", "T;DLIQ;DVAP", MOLAR_BASE_SI, 0, 0, density, 0, z);
        CAPTURE(rD.hUnits);
        CHECK(rD.ierr == 0);
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Chempot=Gibbs for pure?", "[flash],[chempot]") {
    int kflag = 0; FLAGS("SETREF", 2, kflag);
    std::vector<double> z(20, 0); z[0] = 1;
    auto r = REFPROP("Propane", "TP", "G;CPOT(1)", MOLAR_SI, 0, 0, 300, 0.1, z);
    CAPTURE(r.hUnits);
    CHECK(r.Output[1] == Approx(r.Output[0]));
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Order of R32+yf should not matter", "[setup],[fluidorder]") {
    std::vector<double> z1 = { 0.6867261958191739, 0.3132738041808261 }, z2 = { 0.3132738041808261, 0.6867261958191739};
    auto r1 = REFPROP("R32*R1234YF", "TQmolar", "D", 20, 0, 0, 352.448, 0.0, z1);
    auto r2 = REFPROP("R1234YF*R32", "TQmolar", "D", 20, 0, 0, 352.448, 0.0, z2);
    CHECK(r1.Output[1] == Approx(r2.Output[0]).epsilon(1e-11));
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Order of ternary should not matter", "[setup],[fluidorder]") {
    std::vector<std::string> fluidset = {"13BUTADIENE", "1PENTENE", "1BUTYNE"};
    double T = 300; // K
    double p = 10; // MPa
    std::vector<double> z = { 1.0/3.0, 1.0/3.0, 1.0/3.0 };
    std::vector<double> outputs;
    // Check that ALL permutations of fluids give the same answer
    std::sort(fluidset.begin(), fluidset .end());
    do {
        std::string fluidstr = str_join(fluidset, "*");
        auto r = REFPROP(fluidstr, "TP", "D", 1, 0, 0, T, p, z);
        outputs.push_back(r.Output[0]);
    } while (std::next_permutation(fluidset.begin(), fluidset.end()));
    double minval = *std::min_element(outputs.begin(), outputs.end()),
           maxval = *std::max_element(outputs.begin(), outputs.end());
    CHECK(minval == Approx(maxval).epsilon(1e-11));
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Homogeneous phase flash roundtrips", "[roundtrips]") {
    std::string keys = "T;P;D;H;S;E";
    std::vector<std::string> unit_strings = { "DEFAULT", "MOLAR SI", "MASS SI", "SI WITH C", "MOLAR BASE SI", "MASS BASE SI", "ENGLISH", "MOLAR ENGLISH", "MKS", "CGS", "MIXED", "MEUNITS", "USER" };
    for (auto iMass : {0}){//,1}){
        for (std::string fld: {"XENON"}){ //"AMARILLO.MIX"
            for (bool satspln: {true}){//, false}){
                for (std::string & unit_string : unit_strings) {
                    CAPTURE(iMass);
                    CAPTURE(fld);
                    CAPTURE(satspln);
                    CAPTURE(unit_string);
                    int UNITS = get_enum(unit_string);

                    auto get_props = [keys](const REFPROPResult &res) {
                        auto i = 0;
                        std::map<std::string, double> props;
                        for (auto && k : str_split(keys, ";")) {
                            props[k] = res.Output[i];
                            i++;
                        }
                        return props;
                    };

                    // Force a reset (TODO: remove this)
                    int kflag = 0;
                    FLAGS("RESET ALL",1,kflag);
                    FLAGS("CACHE", 3, kflag);
                
                    // Calculation at critical point
                    std::vector<double> zc(20,0);
                    auto r0c = REFPROP(fld,"","TC;DC",UNITS,iMass,satspln,0,0,zc);
                    double Tc = r0c.Output[0], Dc = r0c.Output[1];
                    // Find a point above the isopleth of the phase envelope(single-phase); this is our 
                    // baseline state point in the desired unit system
                    auto rc = REFPROP(fld, "TD&", keys, UNITS, iMass, satspln, Tc, Dc*1.3, r0c.z);
                    auto props0 = get_props(rc);
                    REQUIRE(props0["D"] == Approx(Dc*1.3));
                    REQUIRE(props0["T"] == Approx(Tc));

                    // Run through all the flash calculations
                    // Check the round-trip to get back to the starting point again
                    for (std::string && pair : { "TP", "DT", "TD", "HP", "PH", "PS", "SP", "TS", "PE", // "ST",
                                                 "EP", "PD", "DP", "DH", "HD", "DS","SD", "DE", "ED", "TS", "HS", "SH" }) {
                        std::string k0 = std::string(1,pair[0]), k1 = std::string(1,pair[1]);
                        double v1 = props0[k0], v2 = props0[k1];
                        auto r = REFPROP(fld, pair, keys, UNITS, iMass, satspln, v1, v2, rc.z);
                        auto props = get_props(r);
                        double T = props["T"], T0 = props0["T"];
                        double p = props["P"], p0 = props0["P"];
                        double d = props["D"], d0 = props0["D"];
                        CAPTURE(v1);
                        CAPTURE(v2);
                        CAPTURE(pair);
                        CAPTURE(r.herr);
                        CHECK(r.ierr < 100);
                        CAPTURE(Tc);
                        CAPTURE(Dc);
                        CHECK(T == Approx(T0).epsilon(1e-3));
                        CHECK(p == Approx(p0).epsilon(1e-3));
                        CHECK(d == Approx(d0).epsilon(1e-3));
                    }
                }
            }
        }
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Reset all for flash", "[flags],[resetall]") {

    auto doitmole = [this]() {
        std::vector<double> zc(20, 0);
        auto r0c = REFPROP("AMARILLO.MIX", "", "TC;DC", 0, 0, 0, 0, 0, zc);
        std::vector<double> zmass = zc; double wm = 0;
        XMASSdll(&(zc[0]), &(zmass[0]), wm);
        double Tc = r0c.Output[0], Dc = r0c.Output[1];
        // Find a point inside the isopleth of the phase envelope(VLE); this is our baseline state point
        auto rc = REFPROP("AMARILLO.MIX", "TD", "H;D;S;CP;CV", 0, 0, 1, Tc, Dc*0.7, r0c.z);
        return std::make_pair(rc, zmass);
    };
    auto doitmass = [this](std::vector<double> &zcmass) {
        auto r0c = REFPROP("AMARILLO.MIX", "", "TC;DC", 0, 1, 0, 0, 0, zcmass);
        double Tc = r0c.Output[0], Dc = r0c.Output[1];
        auto rc = REFPROP("AMARILLO.MIX", "TD", "H;D;S;CP;CV", 0, 0, 1, Tc, Dc*0.7, zcmass);
        return rc;
    };
    REFPROPResult r0; std::vector<double> zmass;
    std::tie(r0, zmass) = doitmole();
    int kflag = 0;
    FLAGS("RESET ALL", 1, kflag);
    auto r1 = doitmass(zmass);
    CAPTURE(r0.herr);
    CAPTURE(r1.herr);
    CHECK(r0.ierr == r1.ierr);
}


TEST_CASE_METHOD(REFPROPDLLFixture, "Two-phase phase flash roundtrips", "[VLEroundtrips]") {
    std::string keys = "T;P;D;H;S;E;Qmass;Qmole";
    std::vector<std::string> unit_strings = { "DEFAULT", "MOLAR SI", "MASS SI", "SI WITH C", "MOLAR BASE SI", "MASS BASE SI", "ENGLISH", "MOLAR ENGLISH", "MKS", "CGS", "MIXED", "MEUNITS", "USER" };
    for (auto iMass : { 0 }) {//,1}){
        for (std::string fld : { "XENON"}) { //"AMARILLO.MIX",
            for (bool satspln : {true}) {//, false}){
                for (std::string & unit_string : unit_strings) {
                    CAPTURE(iMass);
                    CAPTURE(fld);
                    CAPTURE(satspln);
                    CAPTURE(unit_string);
                    int UNITS = get_enum(unit_string);

                    auto get_props = [keys](const REFPROPResult &res) {
                        auto i = 0;
                        std::map<std::string, double> props;
                        for (auto && k : str_split(keys, ";")) {
                            props[k] = res.Output[i];
                            i++;
                        }
                        return props;
                    };

                    // Force a reset (TODO: remove this)
                    int kflag = 0;
                    FLAGS("RESET ALL", 1, kflag);
                    FLAGS("CACHE", 3, kflag);

                    // Calculation at critical point
                    std::vector<double> zc(20, 0);
                    auto r0c = REFPROP(fld, "", "TC;DC", UNITS, iMass, satspln, 0, 0, zc);
                    std::vector<double> zmass = zc;
                    double wm = 0;
                    XMASSdll(&(zc[0]), &(zmass[0]), wm);
                    double Tc = r0c.Output[0], Dc = r0c.Output[1];
                    // Find a point inside the isopleth of the phase envelope(VLE); this is our baseline state point
                    auto rc = REFPROP(fld, "TD", keys, UNITS, iMass, satspln, Tc, Dc*0.7, r0c.z);
                    CAPTURE(rc.herr);
                    REQUIRE(rc.ierr < 100);
                    auto props0 = get_props(rc);
                    REQUIRE(props0["D"] == Approx(Dc*0.7));
                    REQUIRE(props0["T"] == Approx(Tc));

                    FLAGS("RESET ALL", 1, kflag);
                    auto r0cmass = REFPROP(fld, "", "TC;DC", UNITS, 1, satspln, 0, 0, zmass);
                    double Tcmass = r0cmass.Output[0], Dcmass = r0cmass.Output[1];
                    auto rcmass = REFPROP(fld, "TD", keys, UNITS, 1, satspln, Tcmass, Dcmass*0.7, zmass);
                    auto props0mass = get_props(rcmass);
                    REQUIRE(props0mass["Qmass"] == Approx(props0["Qmass"]));

                    auto to_key = [](const std::string &pair) {
                        std::vector<std::string> names = str_split(pair, "/");
                        for (auto i = 0; i < 2; ++i) {
                            if (names[i] == "Qmass" || names[i] == "Qmole") {
                                names[i] = "Q";
                            }
                        }
                        return names[0] + names[1];
                    };

                    // Run through all the flash calculations
                    // Check the round-trip to get back to the starting point again
                    for (std::string && pair : { "T/P", "D/T", "T/D", "H/P", "P/H", "P/S", "S/P", "T/S", "P/E", "S/T",
                        "E/P", "P/D", "D/P", "D/H", "H/D", "D/S","S/D", "D/E", "E/D", "T/S", "H/S", "S/H",
                        "Qmass/T", "T/Qmole", "Qmass/S", "S/Qmole", "D/Qmole" }) {
                        std::vector<double> z;
                        std::vector<std::string> names = str_split(pair, "/");
                        std::string k0 = names[0], k1 = names[1];
                        double v1 = props0[k0], v2 = props0[k1];
                        int iMass_ = -1;
                        if (k0 == "Qmass" || k1 == "Qmass") {
                            iMass_ = 1;
                            z = zmass; // mass composition
                        }
                        else {
                            iMass_ = 0;
                            z = r0c.z; // molar composition
                        }
                        auto r = REFPROP(fld, to_key(pair), keys, UNITS, iMass_, satspln, v1, v2, z);
                        auto props = get_props(r);
                        CAPTURE(iMass_);
                        CAPTURE(v1);
                        CAPTURE(v2);
                        CAPTURE(pair);
                        CAPTURE(r.herr);
                        CHECK(r.ierr < 100);
                        if (r.ierr > 100) { continue; } // Error already trapped
                        CAPTURE(Tc);
                        CAPTURE(Dc);
                        double T_expected = (iMass_) ? props0mass["T"] : props0["T"];
                        double P_expected = (iMass_) ? props0mass["P"] : props0["P"];
                        double D_expected = (iMass_) ? props0mass["D"] : props0["D"];
                        double Qmass_expected = (iMass_) ? props0mass["Qmass"] : props0["Qmass"];
                        double Qmole_expected = (iMass_) ? props0mass["Qmole"] : props0["Qmole"];
                        double Q_expected = (iMass_) ? props0mass["Qmass"] : props0["Qmole"];

                        //CHECK(r.q == Approx(Q_expected));
                        CHECK(props["T"] == Approx(T_expected).epsilon(1e-3));
                        CHECK(props["P"] == Approx(P_expected).epsilon(1e-3));
                        CHECK(props["D"] == Approx(D_expected).epsilon(1e-3));
                        CHECK(props["Qmass"] == Approx(Qmass_expected).epsilon(1e-3));
                        CHECK(props["Qmole"] == Approx(Qmole_expected).epsilon(1e-3));
                    }
                }
            }
        }
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Unset splines", "[flags]") {
    std::vector<double> z(20, 1.0); z[0] = 0.4; z[1] = 0.6;
    
    // Do normal calc for CRITP
    auto r0 = REFPROP("R32 * PROPANE", "", "TC", 0, 0, 0, 0, 0, z);

    // Same calc (with splines)
    auto r1 = REFPROP("R32 * PROPANE", "", "TC", 0, 0, 1, 0, 0, z);

    // Turn off splines, back to original
    int k = 0; bool check_kflag = false;
    FLAGS("SPLINES OFF", 1, k, check_kflag);
    FLAGS("SpLiNeS oFf", 1, k, check_kflag); // check case sensitivity (should not be case sensitive)

    // Final result, should be same as r0
    auto r2 = REFPROP("R32 * PROPANE", "", "TC", 0, 0, 0, 0, 0, z);

    CHECK(r0.ierr == r2.ierr);
    CHECK(r0.Output[0] == Approx(r2.Output[0]).epsilon(1e-14));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Unset bounds", "[flags]") {
    std::vector<double> z(20, 1.0); z[0] = 0.4; z[1] = 0.6;

    // Do normal calc for propane below Ttrip
    auto r0 = REFPROP("PROPANE", "TQ", "D", 0, 0, 0, 80, 0, z);
    
    // Turn off bounds
    int k = 0;
    FLAGS("BOUNDS", 1, k);
    FLAGS("BoUnDs", 1, k); // check case sensitivity (should not be case sensitive)

    // Same calc (with bounds off)
    auto r1 = REFPROP("PROPANE", "TQ", "D", 0, 0, 0, 80, 0, z);

    CHECK(r0.ierr == 1);
    CHECK(r1.ierr == 0);
    CHECK(r0.Output[0] == Approx(r1.Output[0]).epsilon(1e-14));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check AGA8 stays on after SETFLUIDS", "[flags],[AGA8],[AGA8stayon]") {
    std::vector<double> z(20, 1.0); z[0] = 0.4; z[1] = 0.6;

    // Do normal calc w/ full EOS
    auto r0 = REFPROP("Methane * Ethane", "TQ", "D", 0, 0, 0, 80, 0, z);

    // Turn on AGA8 flag
    auto r1 = REFPROP("", "flags", "aga8", 1, 0, 1, 80, 0, z);

    // Query the AGA8 flag
    int k0 = 0;
    FLAGS("AGA8", -999, k0);
    REQUIRE(k0 == 1);

    // Set fluids
    {
        int ierr = 0; std::string herrsetup;
        SETFLUIDS("Methane * Ethane", ierr, herrsetup);
        CAPTURE(herrsetup);
        CHECK(ierr == 0);
    }

    // Check AGA8; should still be on
    int k = 0;
    FLAGS("AGA8", -999, k);
    REQUIRE(k == 1);
};


TEST_CASE_METHOD(REFPROPDLLFixture, "Test surface tension H2O + X fails", "[surten]") {
    for (auto &&other : get_pure_fluids_list()) {
        if (other=="WATER"){ continue; }
        std::vector<double> z(20,0); z[0] = 0.4; z[1] = 0.6;
        int ierr = 0; std::string herr;
        CAPTURE("Water * " + other);
        SETFLUIDS("Water * " + other, ierr, herr);
        CAPTURE(herr);
        if (ierr == 117){ continue; } // Skip ones where setup was impossible
        auto r = REFPROP("", "PQ", "STN", 0, 0, 0, 101.325, 1, z);
        CAPTURE(r.herr);
        if (other == "ACETONE" || other == "METHANOL" || other == "ETHANOL"){ continue; } // These have ST1
        CHECK(r.ierr == 518);
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Test surface tension for ST1 models", "[surten]") {
    for (auto &&flds : {"Acetone * Water","Methanol * Benzene","Methanol * Water", "Ethanol * Water", "Methane * Nitrogen","Nitrogen * Argon","Nitrogen*Oxygen","Pentane*Hexadecane"}) {
        std::vector<double> z(20, 0); z[0] = 0.4; z[1] = 0.6; 
        auto r = REFPROP(flds, "PQ", "STN", 0, 0, 0, 101.325, 1, z);
        CAPTURE(r.herr);
        CHECK(r.ierr < 100);
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Test all PX0 for pures", "[setup],[PX0]") {
    auto flds_with_PH0 = fluids_with_PH0_or_PX0();
    REQUIRE(flds_with_PH0.size() > 0);
    for (auto &&fluid : flds_with_PH0) {
        std::vector<double> z(20,1.0);
        auto r = REFPROP(fluid, " ", "TRED;DRED", 1, 0, 0, 0, 0, z);
        double tau = 0.9, delta = 1.1, rho = delta*r.Output[1], T = r.Output[0] / tau;
        CAPTURE(fluid);
        CHECK(r.ierr < 100);

        reload();
        r = REFPROP(fluid, "TD&", "PHIG00;PHIG10;PHIG11;PHIG01;PHIG20", 1, 0, 0, T, rho, z);
        CHECK(r.ierr == 0);
        std::vector<double> default_ = std::vector<double>(r.Output.begin(), r.Output.begin() + 5);

        reload();
        {
            char hflag[255] = "PX0", herr[255] = "";
            int jflag = 0, kflag = -1, ierr = 0;
            FLAGSdll(hflag, jflag, kflag, ierr, herr, 255, 255);
            CAPTURE(herr);
            REQUIRE(ierr == 0);
            REQUIRE(kflag == jflag);
        }
        r = REFPROP(fluid, "TD&", "PHIG00;PHIG10;PHIG11;PHIG01;PHIG20", 1, 0, 0, T, rho, z);
        CHECK(r.ierr == 0);
        std::vector<double> normal = std::vector<double>(r.Output.begin(), r.Output.begin() + 5); 

        reload();
        {
            char hflag[255] = "PX0", herr[255] = "";
            int jflag = 1, kflag = -1, ierr = 0;
            FLAGSdll(hflag, jflag, kflag, ierr, herr, 255, 255);
            CAPTURE(herr);
            REQUIRE(ierr == 0);
            REQUIRE(kflag == jflag);
        }
        r = REFPROP(fluid, "TD&", "PHIG00;PHIG10;PHIG11;PHIG01;PHIG20", 1, 0, 0, T, rho, z);
        CHECK(r.ierr == 0);
        std::vector<double> with_PX0 = std::vector<double>(r.Output.begin(), r.Output.begin()+5);
        
        CAPTURE(T);
        CAPTURE(rho);
        CHECK(normal.size() == with_PX0.size());
        CHECK(default_.size() == with_PX0.size());
        for (auto i = 0; i < normal.size(); ++i) {
            CHECK(normal[i] == Approx(default_[i]).margin(1e-8)); 
            CHECK(normal[i] == Approx(with_PX0[i]).margin(1e-6));
        }
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Test PX0 for mixtures", "[setup],[PX0],[PX0mix]") {
    auto with_PH0 = fluids_with_PH0_or_PX0();
    REQUIRE(with_PH0.size() > 0);
    int Ncomp = 5;
    std::vector<double> z(Ncomp, 1/static_cast<double>(Ncomp)); 
    int mixes_run = 0;
    for (auto i0 = 0; i0 < with_PH0.size(); ++i0) {

        // Build fluid string
        std::string fluids = with_PH0[i0];
        for (auto j = 1; j < Ncomp; ++j){
            auto ii = (i0 + j) % (with_PH0.size() - 1); // mod to wrap around
            fluids += " * " + with_PH0[ii];
        }
        
        // Can you load and get reducing state?
        auto r = REFPROP(fluids, " ", "TRED;DRED", 1, 0, 0, 0, 0, z);
        double tau = 0.9, delta = 1.1, rho = delta*r.Output[1], T = r.Output[0] / tau;
        if (r.ierr > 100) {
            continue;
        }
        CAPTURE(fluids);
        CHECK(r.ierr < 100);
        mixes_run++;

        reload();
        r = REFPROP(fluids, "TD&", "PHIG00;PHIG10;PHIG11;PHIG01;PHIG20", 1, 0, 0, T, rho, z);
        CHECK(r.ierr < 100);
        std::vector<double> default_ = std::vector<double>(r.Output.begin(), r.Output.begin() + 5);

        reload();
        {
            char hflag[255] = "PX0", herr[255] = "";
            int jflag = 0, kflag = -1, ierr = 0;
            FLAGSdll(hflag, jflag, kflag, ierr, herr, 255, 255);
            CAPTURE(herr);
            REQUIRE(ierr == 0);
            REQUIRE(kflag == jflag);
        }
        r = REFPROP(fluids, "TD&", "PHIG00;PHIG10;PHIG11;PHIG01;PHIG20", 1, 0, 0, T, rho, z);
        CHECK(r.ierr < 100);
        std::vector<double> normal = std::vector<double>(r.Output.begin(), r.Output.begin() + 5);

        reload();
        {
            char hflag[255] = "PX0", herr[255] = "";
            int jflag = 1, kflag = -1, ierr = 0;
            FLAGSdll(hflag, jflag, kflag, ierr, herr, 255, 255);
            CAPTURE(herr);
            REQUIRE(ierr == 0);
            REQUIRE(kflag == jflag);
        }
        r = REFPROP(fluids, "TD&", "PHIG00;PHIG10;PHIG11;PHIG01;PHIG20", 1, 0, 0, T, rho, z);
        CHECK(r.ierr < 100);
        std::vector<double> w_PH0 = std::vector<double>(r.Output.begin(), r.Output.begin() + 5);

        CHECK(normal.size() == w_PH0.size());
        CHECK(default_.size() == w_PH0.size());
        for (auto i = 0; i < normal.size(); ++i) {
            CHECK(normal[i] == Approx(default_[i]).margin(1e-8));
            CHECK(normal[i] == Approx(w_PH0[i]).margin(1e-6));
        }
    }
    REQUIRE(mixes_run > 0); // Make sure at least some tests ran
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check mysterious bug with molar mass", "[R134AMM]") {
    std::vector<double> z0(20,0.0); z0[0] = 1.0;
    //auto r0 = REFPROP("Water", "PQ", "T", MOLAR_BASE_SI, 0, 0, 101325, 0, z0);
    /*int DEFAULT = get_enum("DEFAULT");
    int SI = get_enum("SI");
    MOLAR_BASE_SI = get_enum("MOLAR BASE SI");*/
    std::vector<double> z1(20, 0.0); z1[0] = 1.0;
    auto r1 = REFPROP("R134A", "PQ", "H;S;G;R;M;W", MOLAR_BASE_SI, 0, 0, 101325, 0, z1);
    REQUIRE(r1.Output[4] == Approx(0.10203).epsilon(1e-3));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Molar mass of R134a", "[file_loading],[setup]") {
    std::vector<double> z = { 1.0 };
    REQUIRE(REFPROP("R134A", " ", "M", MOLAR_BASE_SI, 0, 0, 0, 0, z).Output[0] == Approx(REFPROP("R134A", " ", "M", MOLAR_SI, 0, 0, 0, 0, z).Output[0] / 1000));
    REQUIRE(REFPROP("R134A", " ", "M", MOLAR_BASE_SI, 0, 0, 0, 0, z).Output[0] == Approx(0.10203).epsilon(1e-3));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check super long list of fluids", "[100comps],[setup]") {
    std::vector<double> z = { 1.0 };
    std::string flds = "Water";
    for (auto i = 0; i < 100; ++i) { flds += "*Water"; }
    auto r = REFPROP(flds, " ", "M", MOLAR_BASE_SI, 0, 0, 0, 0, z);
    CAPTURE(r.herr);
    REQUIRE(r.ierr > 100); // [TODO] force to be a 109 error
};

//TEST_CASE_METHOD(REFPROPDLLFixture, "Test mixture models of Thol", "[flash],[TholLNG]") {
//    std::vector<double> z = { 0.5, 0.5 };
//    int k = -1;
//    FLAGS("GERG", 1, k);
//
//    CHECK(REFPROP("Methane * Butane", "TP", "D", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(244.686));
//    CHECK(REFPROP("Methane * Butane", "TP", "CP", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(99.050));
//    CHECK(REFPROP("Methane * Butane", "TP", "CV", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(89.637));
//    CHECK(REFPROP("Methane * Butane", "TP", "W", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(346.02));
//
//    CHECK(REFPROP("Methane * isobutane", "TP", "W", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(345.720358243));
//    CHECK(REFPROP("Methane * isobutane", "TP", "D", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(244.6790489429));
//    CHECK(REFPROP("Methane * pentane", "TP", "W", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(312.623));
//    CHECK(REFPROP("Methane * pentane", "TP", "CP", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(116.636));
//    CHECK(REFPROP("Methane * pentane", "TP", "W", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(311.216));
//    CHECK(REFPROP("Methane * isopentane", "TP", "D", MOLAR_BASE_SI, 0, 0, 500, 1e6, z).Output[0] == Approx(247.6510495243));
//};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check R404A", "[R404A]") {
    std::vector<double> z = { 1.0 };
    auto r = REFPROP("R404A.MIX", "PQ", "T", MOLAR_BASE_SI, 0, 0, 101325, 0, z);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Qmass for single-phase point", "[flash],[props]") {
    std::vector<double> z = { 1.0 };
    auto r = REFPROP("Propane", "TP", "Qmass", MOLAR_BASE_SI, 0, 0, 273.15, 101325, z);
    auto Qmass2 = r.q;
    REQUIRE(Qmass2 == Approx(998).margin(1));
    auto Qmass = r.Output[0];
    REQUIRE(Qmass == Approx(998).margin(1));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check fluid files with dash in them", "[file_loading],[setup],[resources]") {
    char * RESOURCES = std::getenv("RESOURCES");
    REQUIRE(RESOURCES != nullptr);
    auto resources = normalize_path(std::string(RESOURCES));
        
    int ierr = 1, nc = 1;
    char herr[255], hfld[] = "-10.0.FLD", hhmx[] = "HMX.BNC", href[] = "DEF";
    char path[256] = ""; memset(path, 255, ' '); strcpy(path, resources.c_str());
    SETPATHdll(path, 255);
    SETUPdll(nc, hfld, hhmx, href, ierr, herr, 10000, 255, 3, 255);
    CAPTURE(herr);
    REQUIRE(ierr == 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check fluid files with unicode in them", "[file_loading],[setup],[resources]") {
    char * RESOURCES = std::getenv("RESOURCES");
    REQUIRE(RESOURCES != nullptr);
    auto resources = normalize_path(std::string(RESOURCES));
    int ierr = 1, nc = 1;
    char herr[255], hfld[] = "été.FLD", hhmx[] = "HMX.BNC", href[] = "DEF";
    char path[256] = ""; memset(path, 255, ' '); strcpy(path, resources.c_str());
    SETPATHdll(path, 255);
    SETUPdll(nc, hfld, hhmx, href, ierr, herr, 10000, 255, 3, 255);
    CAPTURE(herr);
    REQUIRE(ierr == 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check TP flash of multicomponent mixture", "[flash],[TPflash]"){
    char hfld[10001] = "NITROGEN|WATER|CO2|H2S|METHANE|ETHANE|PROPANE|ISOBUTAN|BUTANE|IPENTANE|PENTANE";
    std::vector<double> z = { 1.2000036000108E-03,7.000021000063E-06,.828792486377459,2.000006000018E-04,.160400481201444,7.6000228000684E-03,1.4000042000126E-03,1.000003000009E-04,2.000006000018E-04,0,1.000003000009E-04 };
    while (z.size() < 20) {
        z.push_back(0.0);
    }
    int ierr = 0, nc = 11;
    char herr[280] = "", hhmx[256] = "HMX.BNC", href[4] = "DEF";
    SETUPdll(nc, hfld, hhmx, href, ierr, herr, 10000, 255, 3, 255);
    REQUIRE(ierr == 0);

    ierr = 0;
    std::vector<double> x(20, 0.0), y(20, 0.0);
    double T = 313.15, p = 400, d = -1, dl = -1, dv = -1, h = -1, s = -1, u = -1, cp = -1, cv = -1, q = 0, w = -1;
    TPFLSHdll(T, p, &(z[0]), d, dl, dv, &(x[0]), &(y[0]), q, u, h, s, cp, cv, w, ierr, herr, 255);
    std::string sherr(herr);
    CAPTURE(sherr);
    REQUIRE(ierr == 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check TP flash of multicomponent mixture w/ REFPROP function", "[flash],[REFPROPTPflash]") {
    std::string fluids = "NITROGEN*WATER*CO2*H2S*METHANE*ETHANE*PROPANE*ISOBUTAN*BUTANE*IPENTANE*PENTANE";
    std::vector<double> z = { 1.2000036000108E-03,7.000021000063E-06,.828792486377459,2.000006000018E-04,.160400481201444,7.6000228000684E-03,1.4000042000126E-03,1.000003000009E-04,2.000006000018E-04,0,1.000003000009E-04 };
    double T = 313.15, p = 400;
    auto r2 = REFPROP(fluids, "TP", "D", 0, 0, 0, T, p, z);
    REQUIRE(r2.ierr == 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "EnablePR", "[setup]") {
    // SETUP
    int ierr = 0, nc = 2;
    char hfld[10000] = "METHANE|ETHANE";
    char herr[255], hhmx[] = "HMX.BNC", href[] = "DEF";
    SETUPdll(nc, hfld, hhmx, href, ierr, herr, 10000, 255, 3, 255);
    REQUIRE(ierr == 0);

    // Turn on PR
    int prflag = 2;
    PREOSdll(prflag);

    // Get the parameters
    int icomp = 1, jcomp = 2;
    ierr = 0;
    char hmodij[3], hfmix[255], hbinp[255], hfij[255], hmxrul[255];
    double fij[6];
    GETKTVdll(icomp, jcomp, hmodij, fij, hfmix, hfij, hbinp, hmxrul, 3, 255, 255, 255, 255);
    std::string mod = std::string(hmodij, 3);
    CAPTURE(mod);
    REQUIRE(mod == "PR ");
};

TEST_CASE_METHOD(REFPROPDLLFixture, "PRVLE calling FLAGS(\"PR\")", "[PR]") {
    
    std::vector<double> z(20, 0); z[0] = 0.4; z[1] = 0.6;

    // Normal
    auto r = REFPROP("METHANE * ETHANE", "PQ", "T;D", 0, 0, 0, 101.325, 0, z);
    CHECK(r.ierr == 0);

    // Turn on PR (no VT)
    int prflag = 2, kflag = -1; FLAGS("PR", prflag, kflag);
    auto r1 = REFPROP("METHANE * ETHANE", "PQ", "T;D", 0,0,0,101.325,0,z);
    CHECK(r1.ierr == 0);

    // Turn on PR (with VT)
    prflag = 3; kflag = -1; FLAGS("PR",prflag, kflag);
    auto r2 = REFPROP("METHANE * ETHANE", "PQ", "T;D", 0, 0, 0, 101.325, 0, z);
    CHECK(r2.ierr == 0);

    CHECK(r2.Output[0] == Approx(r1.Output[0])); // VT should not change pressure
    CHECK(r2.Output[1] != Approx(r1.Output[1]).margin(0.1)); // VT should change density
};

TEST_CASE_METHOD(REFPROPDLLFixture, "PRVLE calling PREOS", "[PR]") {

    std::vector<double> z(20, 0); z[0] = 0.4; z[1] = 0.6;

    // Normal
    auto r = REFPROP("METHANE * ETHANE", "PQ", "T;D", 0, 0, 0, 101.325, 0, z);
    CHECK(r.ierr == 0);

    // Turn on PR (no VT)
    int prflag = 2; PREOSdll(prflag);
    auto r1 = REFPROP("METHANE * ETHANE", "PQ", "T;D", 0, 0, 0, 101.325, 0, z);
    CHECK(r1.ierr == 0);

    // Turn on PR (with VT)
    prflag = 3; PREOSdll(prflag);
    auto r2 = REFPROP("METHANE * ETHANE", "PQ", "T;D", 0, 0, 0, 101.325, 0, z);
    CHECK(r2.ierr == 0);

    CHECK(r2.Output[0] == Approx(r1.Output[0])); // VT should not change pressure
    CHECK(r2.Output[1] != Approx(r1.Output[1]).margin(0.1)); // VT should change density
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check ancillaries for pure fluids", "[flash],[ancillary]") {
    for (auto &fld : get_pure_fluids_list()) {
        {
            std::vector<double> z(20,1.0); 
            auto rs = REFPROP(fld, "", "TC", DEFAULT, 0, 0, 0, 0, z);
            CAPTURE(fld);
            CAPTURE(rs.herr);
            CAPTURE(rs.ierr);
            double Tc = rs.Output[0];
            auto rv = REFPROP(fld, "Tq", "P;DLIQ;DVAP", DEFAULT, 0, 0, 0.9*Tc, 0, z); // VLE
            auto ra = REFPROP(fld, "", "ANC-TP;ANC-TDL;ANC-TDV", DEFAULT, 0, 0, 0.9*Tc, 0, z); // Ancillary
            int kph = 1,  // liquid, but it doesn't matter
                iprop = 1;  // temperature
            auto g = SATGUESS(kph, iprop, 0.9*Tc, z);
            std::valarray<double> rg = { g.p, g.D, g.Dy };

            int ierr = 0; std::string herr;
            SETFLUIDS(fld, ierr, herr);
            CAPTURE(herr);
            REQUIRE(ierr == 0);
            std::vector<std::string> names = {"ANC-TP","ANC-TDL","ANC-TDV"};

            for (auto i = 0; i < 3; ++i) {
                std::string name = names[i];
                double tol;
                if (fld == "ETHANOL" || fld == "METHANOL") {
                    tol = 0.005; // Can't achieve the desired tolerance for these fluids
                }
                else {
                    tol = 0.003;
                }
                double err_perc = std::abs(rv.Output[i]/ ra.Output[i]-1)*100;
                CAPTURE(err_perc);
                CAPTURE(name);
                CAPTURE(tol);
                CHECK(ra.Output[i] == Approx(rv.Output[i]).epsilon(tol));

                // Check error between SATGUESS and values from ANC-TXX, should be zero
                double err_g_perc = std::abs(rg[i] / ra.Output[i] - 1) * 100;
                CAPTURE(err_g_perc);
                CHECK(err_g_perc == Approx(0.0).epsilon(1e-10));
            }
        }
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Check NBP for all pure fluids (when possible)", "[flash]"){ 
    for (auto &fld : get_pure_fluids_list()) {
        {
            int ierr = 0; std::string herr;
            SETFLUIDS(fld, ierr, herr);
            CAPTURE(herr);
            CAPTURE(fld);
            REQUIRE(ierr == 0);
        }

        {
            int iMass = 0, iFlag = 0;
            std::vector<double> z = { 0.5,0.5 }; double a = 1, Q = 0.0;
            auto r = REFPROP(fld, "TRIP", "P", MOLAR_BASE_SI, iMass, iFlag, a, Q, z);
            double pt = r.Output[0];
            CAPTURE(fld);
            CAPTURE(r.herr);
            CHECK(r.ierr < 100);
            if (pt > 101325) {
                // Impossible; skip this fluid
                continue;
            }
        }

        double wmm = -1, ttrp = -1, tnbpt = -1, tc = -1, pc = -1, Dc = -1, Zc = -1, acf = -1, dip = -1, Rgas = -1, z[20] = { 1.0 };
        int icomp = 1, kq = 1;
        INFOdll(icomp, wmm, ttrp, tnbpt, tc, pc, Dc, Zc, acf, dip, Rgas);

        double T = -1, p = 101.325, D=0, Dl = 0, Dv = 0, xliq[20], xvap[20], q = 0, u = 0, h = 0, s = 0, cv = 0, cp = 0, w = 0;
        int ierr = 0; char herr[255] = "";

        char htyp[4] = "EOS"; double Tmin, Tmax, Dmax, Pmax;
        LIMITSdll(htyp, z, Tmin, Tmax, Dmax, Pmax, 3);

        // Calculate T_nbp given p_nbp (101.325 kPa)
        PQFLSHdll(p, q, z, kq, T, D, Dl, Dv, xliq, xvap, u, h, s, cv, cp, w, ierr, herr, 255);
        CAPTURE(fld);
        CAPTURE(herr);
        CHECK(ierr < 100);
        if (T < Tmin) { continue; }
        CHECK(T == Approx(tnbpt).margin(0.01));

        // Now do the inverse calculation T_nbp->p_nbp
        T = tnbpt; ierr = 0;
        TQFLSHdll(T, q, z, kq, p, D, Dl, Dv, xliq, xvap, u, h, s, cv, cp, w, ierr, herr, 255);
        CAPTURE(fld);
        CAPTURE(herr);
        CHECK(ierr < 100);
        CHECK(p == Approx(101.325).margin(0.1));
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check Ttriple for all pure fluids (when possible)", "[flash]"){
    for (auto &fld : get_pure_fluids_list()) {
        int ierr = 0; std::string herrs;
        SETFLUIDS(fld, ierr, herrs);
        CAPTURE(herrs);
        CHECK(ierr == 0);

        double wmm = -1, ttrp = -1, tnbpt = -1, tc = -1, pc = -1, Dc = -1, Zc = -1, acf = -1, dip = -1, Rgas = -1, z[20] = { 1.0 };
        int icomp = 1, kq = 1;
        INFOdll(icomp, wmm, ttrp, tnbpt, tc, pc, Dc, Zc, acf, dip, Rgas);

        double p = 101.325, D = 0, Dl = 0, Dv = 0, xliq[20], xvap[20], q = 0, u = 0, h = 0, s = 0, cv = 0, cp = 0, w = 0;
        ierr = 0; char herr[255] = "";

        char htyp[4] = "EOS"; double Tmin = -1, Tmax = -1, Dmax = -1, Pmax = -1;
        LIMITSdll(htyp, z, Tmin, Tmax, Dmax, Pmax, 3);
        if (Tmin > ttrp) {
            // Skip fluids where Tmin < Trip
            continue;
        }

        // Now do the calculation at T_trp
        TQFLSHdll(ttrp, q, z, kq, p, D, Dl, Dv, xliq, xvap, u, h, s, cv, cp, w, ierr, herr, 255);
        CAPTURE(fld);
        CAPTURE(herr);
        CHECK(ierr == 0);
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check acentric factor for all pure fluids (when possible)", "[flash],[acentric]") {
    for (auto &fld : get_pure_fluids_list()) {
        int ierr = 0; std::string herrs;
        SETFLUIDS(fld, ierr, herrs);
        CAPTURE(herrs);
        CHECK(ierr == 0);

        double wmm = -1, ttrp = -1, tnbpt = -1, tc = -1, pc = -1, Dc = -1, Zc = -1, acf = -1, dip = -1, Rgas = -1, z[20] = { 1.0 };
        int icomp = 1, kq = 1;
        INFOdll(icomp, wmm, ttrp, tnbpt, tc, pc, Dc, Zc, acf, dip, Rgas);

        char htyp[4] = "EOS"; double Tmin = -1, Tmax = -1, Dmax = -1, Pmax = -1;
        LIMITSdll(htyp, z, Tmin, Tmax, Dmax, Pmax, 3);
        std::vector<double> zz(20,0); zz[0] = 1;
        auto r = REFPROP("","TQ","P",0,0,0,0.7*tc,0,zz);

        double ACF_EOS = -log10(r.Output[0]/pc)-1;
        
        CAPTURE(fld);
        CAPTURE(r.herr);
        CHECK(r.ierr < 2);
        CHECK(ACF_EOS == Approx(acf).epsilon(0.01));
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check absolute paths are ok", "[setup]") {
    std::string fld = std::string(std::getenv("RPPREFIX")) + "/FLUIDS/ACETYLENE.FLD";
    fld = normalize_path(fld);
    
    int ierr = 0; std::string herr;
    SETFLUIDS(fld, ierr, herr);
    CAPTURE(herr);
    REQUIRE(ierr == 0);
    double wmol = 0; int i = 1;
    WMOLIdll(i, wmol);
    CHECK(wmol > 26);
    CHECK(wmol < 27);
};


TEST_CASE_METHOD(REFPROPDLLFixture, "Check mass/molar caching correct", "[setup]") {
    auto units = get_enum("DEFAULT");
    auto fname = "R433B.MIX";
    std::vector<double> z(20, 1.0 / 20.0);
    int iMass = 0;
    auto r1 = REFPROP(fname, "", "M;TC;PC;DC", units, iMass, 0, 0.101325, 300, z);
    iMass = 1;
    auto r2 = REFPROP(fname, "", "TRED", units, iMass, 1, 0, 0, z);
    CHECK(r2.ierr > 100); // Error because composition given and it is not the mass composition

    // Water, to reset for sure
    REFPROP("Water", "", "M;TC;PC;DC", units, iMass, 1, 0, 0, z);

    auto r3 = REFPROP(fname, "", "M;TC;PC;DC", units, iMass, 1, 0, 0, z);
    CHECK(r1.Output[0] == Approx(r3.Output[0]));
    CHECK(r1.Output[1] == Approx(r3.Output[1]));
    CHECK(r1.Output[2] == Approx(r3.Output[2]));

};


TEST_CASE_METHOD(REFPROPDLLFixture, "Check full absolute paths are ok", "[setup],[FDIR]") {

    std::string hmx = std::string(std::getenv("RPPREFIX")) + "/FLUIDS/HMX.BNC";
    hmx = normalize_path(hmx);
    // hmx += std::string(255-hmx.size(), '\0');
    char * hhmx = const_cast<char *>(hmx.c_str());

    for (char sep : {'*', ';'})
    {
        std::string fld0 = std::string(std::getenv("RPPREFIX")) + "/FLUIDS/R32.FLD";
        std::string fld1 = std::string(std::getenv("RPPREFIX")) + "/FLUIDS/PROPANE.FLD";
        fld0 = normalize_path(fld0);
        fld1 = normalize_path(fld1);
        std::string fld = fld0 + std::string(1, sep) + fld1;
        std::string flds = fld + std::string(10000 - fld.size()-2, '\0');
        int iMass = 0, iFlag = 0;
        std::vector<double> z = { 0.5,0.5 }; double a = 1, Q = 0.0;
        auto r = REFPROP(flds, "", "FDIR(1)", MOLAR_BASE_SI, iMass, iFlag, a, Q, z);
        CAPTURE(r.hUnits);
        CAPTURE(sep);
        CAPTURE(r.herr);
        std::string str = std::string(r.hUnits);
        boost::algorithm::trim(str);
        str = normalize_path(str);
        fld0 = normalize_path(fld0);
        CHECK(str == fld0);
    }
    {
        std::string flds = normalize_path(std::string(std::getenv("RPPREFIX")) + "/FLUIDS/R32.FLD") + "|" + normalize_path(std::string(std::getenv("RPPREFIX")) + "/FLUIDS/PROPANE.FLD");
        flds += std::string(10000 - flds.size(), '\0');
        char * hfld = const_cast<char *>(flds.c_str());
        int ierr = 0, nc = 2; char hdef[4] = "DEF", herr[255] = "";
        SETUPdll(nc, hfld, hhmx, hdef, ierr, herr, 10000, 255, 3, 255);
        CAPTURE(herr);
        CHECK(ierr == 0);
        double wmol = 0; int i = 1;
        WMOLIdll(i, wmol);
        CHECK(wmol > 50);
        CHECK(wmol < 55);
    }
    {
        std::string flds = "R32;PROPANE" + std::string(5000, '\0');
        char * hfld = const_cast<char *>(flds.c_str());
        char hin[255] = "TQ", hout[255] = "D", hUnits[255], herr[255];
        int iMass = 0, iFlag = 0, iUnit, ierr = 0;
        double Output[200], x[20], y[20], x3[20], z[2] = { 0.5,0.5 }, q, T = 300.0, Q = 0.0;
        REFPROPdll(hfld, hin, hout, MOLAR_BASE_SI, iMass, iFlag, T, Q, z, Output, hUnits, iUnit, x, y, x3, q, ierr, herr, 10000, 255, 255, 255, 255);
        CAPTURE(herr);
        CHECK(ierr == 0);
    }
};

struct KTVvalues {
    std::string hmodij; 
    std::vector<double> fij;
    std::string hfmix, hfij, hbinp, hmxrul;
};
class GETSETKTV : public REFPROPDLLFixture
{
private:
    enum class perturbations { reset, peng_robinson, AGA, set_back, jiggle_then_reset };

public:
    KTVvalues get_values(int icomp = 1, int jcomp = 2) {
        char hmodij[4] = "", hfmix[256] = "", hfij[256] = "", hbinp[256] = "", hmxrul[256] = "";
        std::vector<double> fij(6, 0.0);
        memset(hmodij, ' ',3); memset(hfmix, ' ', 255); memset(hfij, ' ', 255); memset(hbinp, ' ', 255); memset(hmxrul, ' ', 255);
        int ii = icomp, jj = jcomp;
        GETKTVdll(ii, jj, hmodij, &(fij[0]), hfmix, hfij, hbinp, hmxrul, 3, 255, 255, 255, 255);
        KTVvalues o;
        
        o.hmodij = std::string(hmodij,3); o.fij=fij; o.hfmix = std::string(hfmix,254); o.hfij = std::string(hfij,254); o.hbinp = std::string(hbinp,254); o.hmxrul = std::string(hmxrul,254);
        boost::algorithm::trim(o.hmodij); //inplace
        boost::algorithm::trim(o.hfmix); //inplace
        boost::algorithm::trim(o.hfij); //inplace
        boost::algorithm::trim(o.hbinp); //inplace
        boost::algorithm::trim(o.hmxrul); //inplace
        return o;
    }
    void set_values(const KTVvalues in, int icomp = 1, int jcomp = 2) {
        char hmodij[4] = "", hfmix[256] = "", hfij[256] = "", hbinp[256] = "", hmxrul[256] = "";
        memset(hmodij, ' ', 3); memset(hfmix, ' ', 255); memset(hfij, ' ', 255); memset(hbinp, ' ', 255); memset(hmxrul, ' ', 255);
        strcpy(hmodij, in.hmodij.c_str());
        strcpy(hfmix, in.hfmix.c_str());
        strcpy(hfij, in.hfij.c_str());
        strcpy(hbinp, in.hbinp.c_str());
        strcpy(hmxrul, in.hmxrul.c_str());
        if (in.hmodij.length() == 2) {
            hmodij[2] = ' ';
        }
        double fij[6]; for (auto i = 0; i < in.fij.size(); ++i){ fij[i] = in.fij[i]; }
        int ierr = 0; char herr[256] = ""; memset(herr, ' ', 255);
        SETKTVdll(icomp, jcomp, hmodij, fij, hfmix, ierr, herr, 3, 255, 255);
        CAPTURE(herr);
        CHECK(ierr == 0);
    }
    KTVvalues jiggle(const KTVvalues in) {
        KTVvalues out = in;
        out.fij[0] += 0.0123;
        return out;
    }
    void reset() {
        int icomp = 1, jcomp = 2;
        char hmodij[4] = "RST", hfmix[256] = ""; double fij[6] = {0,0,0,0,0,0}; 
        int ierr = 0; char herr[256] = ""; memset(herr, '\0', 255); memset(hfmix, '\0', 255);
        SETKTVdll(icomp, jcomp, hmodij, fij, hfmix, ierr, herr, 3, 255, 255);
        CAPTURE(herr);
        CHECK(ierr == 0);
    }
    void do_action(perturbations step) {
        switch (step) {
            case perturbations::peng_robinson:
            {
                // Initial values
                auto init = get_values();
                // Turn on PR
                int prflag = 2;
                PREOSdll(prflag);
                auto pr_init = get_values();
                // Check PR is on
                CHECK(pr_init.hmodij == std::string("PR"));
                // Jiggle kij parameter
                auto jig = jiggle(pr_init);
                set_values(jig);
                auto nv = get_values();
                // Check PR is on, and parameters are modified
                CHECK(nv.hmodij == std::string("PR"));
                // Turn off PR
                prflag = 0;
                PREOSdll(prflag);
                auto nv2 = get_values();
                // Check that parameters have been set back to initial parameters
                CHECK(are_same(nv2, init));
                // Reset the parameters
                reset();
                auto end = get_values();
                CHECK(are_same(end, init));
                break;
            }
            case perturbations::AGA:
            {
                auto init = get_values();
                int ierr = 0; char herr[256] = "";
                SETAGAdll(ierr, herr, 255);
                auto flags = get_values();
                // Check same as before
                CHECK(are_same(init, flags));
                // Turn off AGA
                UNSETAGAdll();
                // Check same as before
                CHECK(are_same(init, get_values()));
                break;
            }
            case perturbations::jiggle_then_reset:
            {
                auto init = get_values();
                auto jig = jiggle(init);
                set_values(jig);
                auto flags = get_values();
                // Check same as before
                CHECK(are_same(jig, flags));
                // Reset
                reset();
                // Check same as beginning
                CHECK(are_same(init, get_values()));
                break;
            }
            case perturbations::set_back:
            {
                auto init = get_values();
                set_values(init);
                auto flags = get_values();
                // Check same as before
                CHECK(are_same(init, flags));
                // Reset
                reset();
                // Check same as beginning
                CHECK(are_same(init, get_values()));
                break;
            }
        }
    }
    bool are_same(KTVvalues in1, KTVvalues in2) {
        if (in1.fij.size() != in2.fij.size()) { return false; }
        for (auto i = 0; i < in1.fij.size(); ++i) {
            if (std::abs(in1.fij[i] - in2.fij[i]) > 1e-12) {
                return false;
            }
        }
        if (in1.hmodij != in2.hmodij) { return false; }
        if (in1.hfij != in2.hfij) { return false; }
        if (in1.hmxrul != in2.hmxrul) { return false; }
        return true;
    }
    void payload() {
        auto binary_pairs = get_binary_pairs();
        CHECK(binary_pairs.size() > 0);

        for (const std::string & fluids: {"Methane * Ethane", "R1234yf * R134a" }){
            CAPTURE(fluids);
            int ierr = 0; std::string herr;
            SETFLUIDS(fluids, ierr, herr);
            CAPTURE(herr);
            CHECK(ierr == 0);
            // Get the initial state
            auto ktv = get_values();

            // Run each step, checking the output each time
            std::vector<perturbations> steps = {
                perturbations::AGA, 
                perturbations::peng_robinson, 
                perturbations::peng_robinson,
                perturbations::set_back
            };
            for (auto &step : steps) {
                do_action(step);
            }
            
            // Do we end where we started?
            auto new_ktv = get_values();
            CHECK(are_same(ktv, new_ktv));
        }
    }
};
TEST_CASE_METHOD(GETSETKTV, "Get and set B.I.P.", "[BIP]") { payload(); };

TEST_CASE_METHOD(GETSETKTV, "Check BIP for R32 + CO2", "[BIP]") { 
    int ierr =0 ; std::string herr = "";
    SETFLUIDS("R32 * CO2",ierr,herr);
    CAPTURE(herr);
    CHECK(ierr == 0);
    // Get them from GETKTV
    auto vals = get_values();
    // And also get them from the REFPROP function
    std::vector<double> z(20, 1); 
    auto R2 = REFPROP("R32 * CO2","","FIJMIX",0,0,0,1,2,z);
    // The expected values
    std::vector<double> betasFij = { 1.0, 0.9978225, 1.0, 1.0058521, 0.0};
    for (auto i = 0U; i < 5U; ++i) {
        CHECK(betasFij[i] == Approx(vals.fij[i]));
    }
    for (auto i = 0U; i < 5U; ++i) {
        CHECK(betasFij[i] == Approx(R2.Output[i]));
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Two-phase viscosity", "[transport]") {
    std::vector<double> z(20, 1);
    auto r = REFPROP("R32 * CO2", "PQ", "ETA", 0, 0, 0, 101.325, 0.5, z);
    CAPTURE(r.herr);
    CAPTURE(r.Output[0]);
    CHECK(r.ierr != 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Critical TC1, VC1", "[crit]") {
    std::string fld = "Methane * Ethane";
    int ierr = 0; std::string herr;
    SETFLUIDS(fld, ierr, herr);
    CAPTURE(herr);

    std::vector<double> z(20,0.0); z[0] = 0.4; z[1] = 0.6;
    ierr = 0;
    char herr2[255] = "";
    double Tcrit, pcrit_kPa, dcrit_mol_L;
    CRITPdll(&(z[0]), Tcrit, pcrit_kPa, dcrit_mol_L, ierr, herr2, 255);
    CAPTURE(herr2);
    CHECK(ierr <= 0); 
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Ancillary curves for D2O of Herrig", "[D2O],[ancillary]") {
    std::vector<double> z(20,1.0);
    
    // [TODO]: Invalid! Re-enable this test
    /*auto a1 = REFPROP("heavy water", "TD&", "TMELT", 0, 0, 0, 270, 0, z);
    CHECK(a1.ierr > 100); */

    auto a2 = REFPROP("heavy water", "PMELT", "T", 0, 0, 0, 0.837888413e5, 0, z);
    int ierr = 0; char herr[255] = ""; double T_K = -1, p_kPa = 0.837888413e5; MELTPdll(p_kPa, &(z[0]), T_K, ierr, herr, 255U);
    
    CHECK(a2.Output[0] == Approx(270));
    CHECK(T_K == Approx(270));

    // IAPWS from Herrig (several creative ways of getting p_sub(T))
    std::vector<REFPROPResult> b(33);
    b[0] = REFPROP("D2O", "TD&", "SUBL-TP", 0, 0, 0, 245, 0, z);
    b[1] = REFPROP("D2O", "TQ&", "SUBL-TP", 0, 0, 0, 245, -1, z);
    b[2] = REFPROP("D2O", "TSUBL", "P", 0, 0, 0, 245, -1, z);
    {
        int ierr2 = 0; char herr2[256] = ""; double zz[20] = { 1.0 }; double T2 = 245, p_kPa2 = -1; SUBLTdll(T2, zz, p_kPa2, ierr2, herr2, 255U);
        b[3].Output = std::vector<double>(20,0); b[3].Output[0] = p_kPa2;
    }
    for (auto i = 0; i < 4; ++i){
        CAPTURE(i);
        CHECK(b[i].Output[0] == Approx(0.327390934e-1));
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check hUnits are the same several ways", "[hUnits]") {
    std::vector<double> z = { 1.0 };
    auto r = REFPROP("PROPANE", " ", "TC", 0, 0, 0, 0, 0, z);
    auto r2 = REFPROP("PROPANE", " ", "TC;PC", 0, 0, 0, 0, 0, z);
    auto r3 = REFPROP("PROPANE", "TQ", "TC;PC", 0, 0, 0, 200, 0, z);
    auto r4 = REFPROP("PROPANE", "CRIT", "T", 0, 0, 0, 0, 0, z);
    boost::algorithm::trim(r.hUnits); //inplace
    boost::algorithm::trim(r2.hUnits); //inplace
    boost::algorithm::trim(r3.hUnits); //inplace
    boost::algorithm::trim(r4.hUnits); //inplace
    CAPTURE(r.herr);
    CHECK(r.ierr == 0);
    CHECK(r.hUnits == r2.hUnits);
    CHECK(r.hUnits == r3.hUnits);
    //CHECK(r.hUnits == r4.hUnits);  // [TODO]: re-enable
};

TEST_CASE_METHOD(REFPROPDLLFixture, "check all enumerated values for units are correct", "[enum]") {
    std::vector<std::string> keys = {"DEFAULT", "MOLAR SI", "MASS SI", "SI WITH C", "MOLAR BASE SI", "MASS BASE SI", "ENGLISH", "MOLAR ENGLISH", "MKS", "CGS", "MIXED", "MEUNITS", "USER"};
    std::vector<int> vals = {0,1,2,3,100,101,5,6,7,8,9,10,11};
    REQUIRE(keys.size() == vals.size());
    for (auto i = 0; i < keys.size(); ++i) {
        CAPTURE(get_enum(keys[i]) == vals[i])
    }
};
TEST_CASE_METHOD(REFPROPDLLFixture, "Check that invalid unit systems causes reasonable error", "[enum]") {
    std::vector<double> z = { 1.0 };
    auto r = REFPROP("PROPANE", "PQ", "T", -1234, 0, 0, 101.325, 0, z);
    CAPTURE(r.herr);
    CHECK(r.ierr > 100);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "CAS# for PROPANE", "[CAS]") { 
    std::vector<double> z = {1.0};
    auto r = REFPROP("PROPANE", " ", "CAS#", 0,0,0,0,0,z);
    CAPTURE(r.herr);
    CHECK(r.ierr == 0);
    boost::algorithm::trim(r.hUnits); //inplace
    CHECK(r.hUnits == "74-98-6");
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check InChI for D6", "[InChI]") {
    std::vector<double> z = { 1.0 };
    auto r = REFPROP("D6","","INCHI",0,0,0,0,0,z);
    CAPTURE(r.herr);
    CHECK(r.ierr == 0);
    boost::algorithm::trim(r.hUnits); //inplace
    CHECK("InChI="+r.hUnits == "InChI=1S/C12H36O6Si6/c1-19(2)13-20(3,4)15-22(7,8)17-24(11,12)18-23(9,10)16-21(5,6)14-19/h1-12H3");
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Reload the DLL 100 times", "[100Reloads]") {
    for (auto i =0; i < 100; ++i){ 
        reload(); 
    }
};

class ModelSettingFixture: public REFPROPDLLFixture{
public:
    std::map<std::string, std::vector<std::string>> model_options = {
        {"EOS",{"BWR","ECS","FE1","FE2","FE3","FE4","FEK","FEQ","FES"}},
        {"ETA",{"ECS","VS0","VS1","VS2","VS3","VS3","VS4","VS5","VS6","VS7","ECS","TRN"}},
        {"TCX",{"TK0","TC1","TC2","TC3","TC4","TC5","TC6","TC7","ECS","TRN"}},
        {"SBL",{"SB1","SB2","SB3"}},
        {"DEC",{"DE2","DE3","DE4","DE5"}}
    };
    
    void set_all(const std::string &key) {
        for (const auto &fluid: get_pure_fluids_list()){
            for (const auto & mod : model_options[key]) {
                CAPTURE(fluid);
                CAPTURE(mod);
                // Request the model, the next call to SETFLUIDS will turn it on
                int ierr = 0; std::string herr;
                std::tie(ierr, herr) = SETMOD(1, key, "HMX", mod);
                CAPTURE(herr);
                CHECK(ierr == -912); // Should be a warning that the model is not yet enabled
                // Turn it on by setting up the fluids
                int ierrsetup = 0; std::string herrsetup;
                SETFLUIDS(fluid, ierrsetup, herrsetup);
                CAPTURE(ierrsetup);
                // Either ierrsetup is 0 (success), or 
                /*if (ierrsetup == 105){ 
                    WARN("Could not load EOS model "+mod+" for " + fluid);
                    continue;
                }*/
                // If the model has been initialized without error, do a calculation
                // with it
                if (key == "ETA" || key == "TCX" || key == "EOS") {
                    std::vector<double> z(20,1);
                    auto r = REFPROP(fluid,"","TC;DC",0,0,0,0,0,z);
                    double Tc = r.Output[0], Dc = r.Output[1];
                    auto r2 = REFPROP(fluid, "TD&", (key != "EOS") ? key : "D",0,0,0,1.1*Tc,1.1*Dc,z);
                    if (r2.ierr == 540){
                        continue; // Fluid doesn't have a transport property model, so that's not actually an error in this case
                    }
                    CAPTURE(r2.herr);
                    CHECK(r2.ierr < 100);
                }
                else if (key == "MLT"){
                    std::vector<double> z(20, 1);
                    auto r = REFPROP(fluid, "", "PTRP", 0, 0, 0, 0, 0, z);
                    double ptrip = r.Output[0];
                    auto r2 = REFPROP(fluid, "PMELT", "T", 0, 0, 0, 1.5*ptrip, 0, z);
                    CAPTURE(r2.herr);
                    if (r2.ierr == 501) {
                        continue; // Fluid doesn't have a melting line model, so that's not actually an error in this case
                    }
                    CHECK(r2.ierr < 100);
                }
            }
        }
    }
};

TEST_CASE_METHOD(ModelSettingFixture, "Test setting of every possible EOS model for all fluids", "[SETMOD],[EOS]") { set_all("EOS"); }
TEST_CASE_METHOD(ModelSettingFixture, "Test setting of every possible ETA model for all fluids", "[SETMOD],[ETA]") { set_all("ETA"); }
TEST_CASE_METHOD(ModelSettingFixture, "Test setting of every possible TCX model for all fluids", "[SETMOD],[TCX]") { set_all("TCX"); }
TEST_CASE_METHOD(ModelSettingFixture, "Test setting of every possible MLT model for all fluids", "[SETMOD],[MLT]") { set_all("MLT"); }
//TEST_CASE_METHOD(ModelSettingFixture, "Test setting of every possible SBL model for all fluids", "[SETMOD],[SBL]") { set_all("SBL"); }
TEST_CASE_METHOD(ModelSettingFixture, "SETMOD without SETUP should be warning", "[SETMOD]") { 
    int ierr; std::string herr;
    std::tie(ierr, herr) = SETMOD(1, "ETA", "HMX", "VS1");
    CHECK(ierr != 0);
}


TEST_CASE_METHOD(REFPROPDLLFixture, "Torture test calling of DLL", "[veryslow]") {
    /*{
        char hflag[256] = "Debug", herr[256] = "";
        int jflag = 1, kflag = -1, ierr = 0;
        FLAGSdll(hflag, jflag, kflag, ierr, herr, 256, 256);
    }*/
    for (auto &&pair : get_binary_pairs()) {
        for (std::string &&k : { "ETA", "TCX", "CP", "P", "STN", "TC", "PC" }) {
            for (bool forwards : {true, false}) {
                std::vector<double> z;
                if (forwards) {
                    z = { 0.4,0.6 };
                }
                else {
                    z = { 0.6,0.4 };
                }
                std::string fluids = (forwards) ? pair.first + "*" + pair.second : pair.second + "*" + pair.first;
                REFPROP(fluids, "PT", k, 1, 0, 0, 0.101325, 300, z);
            }
        }
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check all alphar derivatives", "[alphar]") {
    for (auto &&pair : get_binary_pairs()) {
        // Setup the fluids
        std::string joined = pair.first + ";" + pair.second;
        int ierr = 0; std::string herr;
        SETFLUIDS(joined, ierr, herr);
        CAPTURE(herr);
        if (ierr == 101) {
            continue;
        }
        CHECK(ierr == 0);

        double tau = 1.3, delta = 0.9;
        std::vector<double> z = { 0.0, 1.0 };
        auto deriv = [this, &z](int itau, int idelta, double tau, double delta) {
            double arderiv;
            PHIXdll(itau, idelta, tau, delta, &(z[0]), arderiv);
            return arderiv / (pow(tau, itau)*pow(delta, idelta));
        };
        for (int itau = 0; itau < 3; ++itau) {
            for (int idelta = 0; idelta < 3; ++idelta) {
                if (itau + idelta > 4 || itau + idelta == 0) {
                    continue;
                }
                // centered derivative based on the lower-order derivative
                double numeric;
                if (itau > 0) {
                    double dtau = 1e-6;
                    numeric = (-deriv(itau - 1, idelta, tau + 2 * dtau, delta)
                        + 8 * deriv(itau - 1, idelta, tau + dtau, delta)
                        - 8 * deriv(itau - 1, idelta, tau - dtau, delta)
                        + deriv(itau - 1, idelta, tau - 2 * dtau, delta)) / (12 * dtau);
                }
                else {
                    double ddelta = 1e-6;
                    numeric = (-deriv(itau, idelta - 1, tau, delta + 2 * ddelta)
                        + 8 * deriv(itau, idelta - 1, tau, delta + ddelta)
                        - 8 * deriv(itau, idelta - 1, tau, delta - ddelta)
                        + deriv(itau, idelta - 1, tau, delta - 2 * ddelta)) / (12 * ddelta);
                }
                double analytic = deriv(itau, idelta, tau, delta);
                CAPTURE(itau);
                CAPTURE(idelta);
                CAPTURE(joined);
                CHECK(analytic == Approx(numeric).epsilon(1e-6));
            }
        }
    }
};

/// From Wagner and Pruss, JPCRD, 2002
class WaterAlpharDerivs : public REFPROPDLLFixture
{
public:
    double deriv(int itau, int idelta, double tau, double delta) {
        double arderiv;
        std::vector<double> z = { 1.0 };
        PHIXdll(itau, idelta, tau, delta, &(z[0]), arderiv);
        return arderiv / (pow(tau, itau)*pow(delta, idelta));
    };
    void lowT() {
        double delta = 838.025/322, tau = 647.096/500;
        CHECK(deriv(0,0,tau,delta) == Approx(-0.342693206e1).epsilon(1e-8));
        CHECK(deriv(0,1,tau,delta) == Approx(-0.364366650).epsilon(1e-8));
        CHECK(deriv(0,2,tau,delta) == Approx(0.856063701).epsilon(1e-8));
        CHECK(deriv(1,0,tau,delta) == Approx(-0.581403435e1).epsilon(1e-8));
        CHECK(deriv(2,0,tau,delta) == Approx(-0.223440737e1).epsilon(1e-8));
        CHECK(deriv(1,1,tau,delta) == Approx(-0.112176915e1).epsilon(1e-8));
    }
    void highT() {
        double delta = 358.0/322, tau = 647.096/647;
        CHECK(deriv(0, 0, tau, delta) == Approx(-0.121202657e1).epsilon(1e-8));
        CHECK(deriv(0, 1, tau, delta) == Approx(-0.714012024).epsilon(1e-8));
        CHECK(deriv(0, 2, tau, delta) == Approx(0.475730696).epsilon(1e-8));
        CHECK(deriv(1, 0, tau, delta) == Approx(-0.321722501e1).epsilon(1e-8));
        CHECK(deriv(2, 0, tau, delta) == Approx(-0.996029507e1).epsilon(1e-8));
        CHECK(deriv(1, 1, tau, delta) == Approx(-0.133214720e1).epsilon(1e-8));
    }
    
    void payload() {
        // Setup the fluids
        int ierr = 0; std::string herr;
        SETFLUIDS("Water", ierr, herr);
        CAPTURE(herr);
        CHECK(ierr == 0);
        lowT();
        highT();
    }
};
TEST_CASE_METHOD(WaterAlpharDerivs, "Water alphar derivatives from IAPWS95", "[water],[alphar]") { payload(); };

TEST_CASE_METHOD(REFPROPDLLFixture, "Tabulated check values from IAPWS release", "[water]") {
    // Table 7 of http://www.iapws.org/relguide/IAPWS95-2018.pdf
    struct point { double T_K, rho_kgm3, p_MPa,cv_kJkgK,w_ms,s_kJkgK; };
    std::vector<point> points = { {300, 0.9965560e3, 0.992418352e-1, 0.413018112e1, 0.150151914e4, 0.393062643},
    { 300,0.1005308e4,0.200022515e2,0.406798347e1,0.153492501e4,0.387405401 },
    { 300,0.1188202e4,0.700004704e3,0.346135580e1,0.244357992e4,0.132609616 },
    { 500,0.4350000,0.999679423e-1,0.150817541e1,0.548314253e3,0.794488271e1 },
    { 500,0.4532000e1,0.999938125,0.166991025e1,0.535739001e3,0.682502725e1 },
    { 500,0.8380250e3,0.100003858e2,0.322106219e1,0.127128441e4,0.256690919e1 },
    { 500,0.1084564e4,0.700000405e3,0.307437693e1,0.241200877e4,0.203237509e1 },
    { 647,0.3580000e3,0.220384756e2,0.618315728e1,0.252145078e3,0.432092307e1 },
    { 900,0.2410000,0.100062559,0.175890657e1,0.724027147e3,0.916653194e1 },
    { 900,0.5261500e2,0.200000690e2,0.193510526e1,0.698445674e3,0.659070225e1 },
    { 900,0.8707690e3,0.700000006e3,0.266422350e1,0.201933608e4,0.417223802e1 } };
    auto MASS_SI = get_enum("MASS SI");
    std::vector<double> z(1, 1.0);
    for (auto& pt: points){
        auto r = REFPROP("WATER", "TD", "P;CV;W;S", MASS_SI, 0, 0, pt.T_K, pt.rho_kgm3, z);
        CAPTURE(r.herr);
        REQUIRE(r.ierr == 0);
        CHECK(r.Output[0] == Approx(pt.p_MPa));
        CHECK(r.Output[1] == Approx(pt.cv_kJkgK));
        CHECK(r.Output[2] == Approx(pt.w_ms));
        CHECK(r.Output[3] == Approx(pt.s_kJkgK));
    }
}

TEST_CASE_METHOD(REFPROPDLLFixture, "Check all variables that do not require state information", "[Stateless]") {
    // These need splines on...
    //std::vector<std::string> variable_names = { "TMAXT","PMAXT","DMAXT","TMAXP","PMAXP","DMAXP"};

    std::vector<std::string> variable_names = { "TC","PC","DC","TCEST","PCEST","DCEST","TRED","DRED","TMIN","TMAX","DMAX","PMAX" };
    std::string joined = variable_names[0];
    for (auto i = 1; i < variable_names.size(); ++i) {
        joined += ";" + variable_names[i];
    }
    for (auto &&pair : get_binary_pairs()) {
        std::vector<double> zz(20,1.0);
        if (REFPROP(pair.first + "*" + pair.second, " ", joined, 0, 0, 0, 0, 0, zz).ierr == 101) {
            WARN("Unable to load the fluids:"+ pair.first + "*" + pair.second);
            continue;
        }
        auto get_Tred = [this, &pair, &joined, &variable_names](bool forwards) {
            auto flds = (forwards) ? pair.first + "*" + pair.second : pair.second + "*" + pair.first;
            std::vector<double> z;
            if (forwards) {
                z = { 0.4, 0.6 };
            }
            else {
                z = { 0.6, 0.4 };
            };
            auto Nvars = variable_names.size();
            auto r = REFPROP(flds, " ", joined, 0, 0, 0, 0, 0, z);
            CAPTURE(r.herr);
            CAPTURE(r.ierr);
            bool acceptable_ierr = (r.ierr < 0 ||  r.ierr == 318);
            CHECK(acceptable_ierr);
            return std::vector<double>(r.Output.begin(), r.Output.begin() + Nvars);
        };
        auto pair_joined = std::get<0>(pair) +  "+" + std::get<1>(pair);
        auto forwards = get_Tred(true), back = get_Tred(false), diff = forwards;
        for (auto i = 0; i < forwards.size(); ++i) {

            diff[i] = std::abs(forwards[i] - back[i]);
            CAPTURE(pair_joined); 
            CAPTURE(variable_names[i]);
            CAPTURE(forwards[i]);
            CAPTURE(back[i]);
            CHECK(forwards[i] > 0);
            CHECK(diff[i]< 1e-8);
        }
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Mixture at STP", "[STP]") {
    std::string flds = "Methane * Helium" + std::string(500, '\0');
    std::vector<double> z = { 0.4, 0.6 };
    auto r = REFPROP(flds, "PT", "ETA", 1, 0, 0, 0.101325, 298, z);
    CAPTURE(r.herr);
    REQUIRE(r.ierr == 0);
    double rho = r.Output[0];
    CHECK(rho == Approx(15).epsilon(0.1));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "One call to transport for N2", "[OneN2]") {
    std::string flds = "Nitrogen" + std::string(500, '\0');
    std::vector<double> z(20, 1.0);
    auto r = REFPROP(flds, "TD&", "ETA", 0, 0, 0, 300, 0, z);
    CAPTURE(r.herr);
    CHECK(r.ierr == 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "CheckZEZEstimated", "[setup]") {
    std::string flds = "R134A*R1234ZEZ" + std::string(500, '\0');
    std::vector<double> z = { 0.5,0.5 };
    auto r = REFPROP(flds, " ", "FIJMIX", 0, 0, 0, 1, 2, z);
    CAPTURE(r.herr);
    CHECK(r.ierr == -117);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check that R1234ze is an invalid fluid name", "[setup]") {
    int ierr = 0; std::string herr;
    SETFLUIDS("R1234ze", ierr, herr);
    CAPTURE(herr);
    CHECK(ierr > 100);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check that all fluids load properly", "[setup]") {
    for (auto &fld : get_pure_fluids_list()) {
        CAPTURE(fld);
        int ierr = 0; std::string herr;
        SETFLUIDS(fld, ierr, herr);
        CAPTURE(herr);
        CHECK(ierr == 0);
    }
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check that it is possible to set predefined mixture and get NCOMP", "[setup],[setmix]") {
    std::string mix = "AIR.MIX";
    CAPTURE(mix);
    auto [z, ierr, herr] = SETMIXTURE(mix);
    CAPTURE(herr);
    CHECK(ierr == 0); 
    auto a = ALLPROPS("NCOMP", 1, 0, 0, 0, 0, z);
    CAPTURE(a.herr);
    CHECK(a.ierr == 0);
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check that setting transport model for benzene doesn't change ammonia model", "[ETA],[NH3]") {
    std::vector<double> z(20, 1.0);

    auto r0 = REFPROP("AMMONIA", "TQ", "ETA", 0, 0, 0, 260, 0, z);
    CHECK(r0.Output[0] == Approx(192.06).epsilon(1e-4));

    auto m0 = GETMOD(1, "ETA");
    SETMOD(1, "ETA", "HMX", "VS1");
    auto r = REFPROP("BENZENE", "TP", "ETA", 0, 0, 0, 298, 101.325, z);
    auto m1 = GETMOD(1, "ETA");

    // Set to backup model(in the fluid file)
    SETMOD(1, "ETA", "HMX", "VS5");
    auto r2 = REFPROP("BENZENE", "TP", "ETA", 0, 0, 0, 298, 101.325, z);
    auto m2 = GETMOD(1, "ETA");

    auto r3 = REFPROP("AMMONIA", "TQ", "ETA", 0, 0, 0, 260, 0, z);
    CHECK(r3.Output[0] == Approx(192.06).epsilon(1e-4));
};

TEST_CASE_METHOD(REFPROPDLLFixture, "Check that REDX works properly", "[REDX]") {
    double x[] = { 0.5,0.5 }, Tr = 1e20, Dr = 1e20;
    int ierr = 0; std::string herr;
    SETFLUIDS("Methane * Nitrogen", ierr, herr);
    CHECK(ierr == 0);
    REDXdll(x, Tr, Dr);
    CAPTURE(Tr);
    CAPTURE(Dr);
    CHECK(Tr > 50);
    CHECK(Tr < 250);
};

class SUBTVALIDATION : public REFPROPDLLFixture
{
public:
    struct PsubCheck {
        std::string name;
        double T_K, p_Pa, eps_tol;
    };
    std::vector<PsubCheck> points = {
        {"SF6", 209.7416748, 0.101325e6, 0.001}, // Guder and Span
        {"H2S", 164.932, 0.0031937e6,0.001},
        {"AMMONIA", 176.957, 8.0746e2, 0.01},
        {"CO",61.55, 3.7596e3, 0.00001},
        {"ETHYLENE",90, 3.5004, 0.0001},
        {"ETHANE", 80.808, 4.2065E-2, 0.0001},
        {"FLUORINE", 50, 6.75998e2,0.001},
        {"N2O", 153.84615384615384, 0.00474895372822e6, 0.001 },
        {"PARAHYD", 10, 2.6206e2, 0.001},
        //{"HYDROGEN", 8.76, 44.4713821784, 0.001 }, NOT USED, updated sublimation curve is questionable
        {"NEON", 20, 3.81191E3,0.001 },
        {"D2O", 245, 0.327390934e2, 0.002} // IAPWS from Herrig
    };
    void payload() {
        for (auto &&pt : points){
            std::string fld = pt.name;
            int ierr = 0; std::string herrsetup;
            SETFLUIDS(fld, ierr, herrsetup);
            CAPTURE(herrsetup);
            REQUIRE(ierr == 0);

            double x[1] = {1.0};
            double p_kPa;
            char herr[256];
            SUBLTdll(pt.T_K, x, p_kPa, ierr,herr,256);
            double p_Pa = p_kPa*1000;
            CAPTURE(fld);
            CAPTURE(herr);
            REQUIRE(ierr == 0);
            CHECK(p_Pa == Approx(pt.p_Pa).epsilon(pt.eps_tol));
        }
    }
};
TEST_CASE_METHOD(SUBTVALIDATION, "Check sublimation pressures", "[sublimation]") { payload(); };