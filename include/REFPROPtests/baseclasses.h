#ifndef REFPROP_TESTS_BASECLASSES
#define REFPROP_TESTS_BASECLASSES

#include "REFPROP_lib.h"

#include "manyso/native.h"
#include "manyso/exceptions.h"
#include "REFPROPtests/utils.h"

#include <string>
#include <cstring>

struct REFPROPResult {
    std::vector<double> z;
    std::vector<double> Output;
    std::string hUnits;
    int iUnit;
    std::vector<double> x, y, x3;
    double q;
    int ierr;
    std::string herr;
};

struct ALLPROPSResult {
    std::vector<double> Output;
    std::string hUnits;
    std::vector<int> iUnit;
    int ierr;
    std::string herr;
};

struct SATGUESSResult {
    double T, p, D, h, s, Dy;
    std::vector<double> y;
    int ierr;
    char herr[256] = "";
};

class REFPROPDLLFixture
{
private:
    std::unique_ptr<NativeSharedLibraryWrapper> RP;
public:
    REFPROPDLLFixture() {
        reload();
        DEFAULT = get_enum("DEFAULT");
        MOLAR_SI = get_enum("MOLAR SI");
        MOLAR_BASE_SI = get_enum("MOLAR BASE SI");
        MASS_BASE_SI = get_enum("MASS BASE SI");
    }
    int DEFAULT, MOLAR_SI, MOLAR_BASE_SI, MASS_BASE_SI;
    void reload() {
        char* RPPREFIX = std::getenv("RPPREFIX");
        REQUIRE(RPPREFIX != nullptr);
        REQUIRE(strlen(RPPREFIX) != 0);
#if defined(__MANYSOISWINDOWS__)

#if defined(IS_32_BIT)
        std::string shared_library_filename = "REFPROP.DLL";
#else
        std::string shared_library_filename = "REFPRP64.DLL";
#endif

#else
#if !defined(__APPLE__)
        std::string shared_library_filename = "librefprop.so";
#else
        std::string shared_library_filename = "librefprop.dylib";
#endif
#endif

        std::string shared_library_path = path_join_and_norm(std::string(RPPREFIX), shared_library_filename);
        CAPTURE(shared_library_path);

#if defined(__MANYSOISWINDOWS__)

#if defined(IS_32_BIT)
        auto load_method = AbstractSharedLibraryWrapper::load_method::LOAD_LIBRARY;
#else
        auto load_method = AbstractSharedLibraryWrapper::load_method::FROM_FILE;
#endif

#else
        auto load_method = AbstractSharedLibraryWrapper::load_method::LOAD_LIBRARY;
#endif
        RP.reset(new NativeSharedLibraryWrapper(shared_library_path, load_method));

        // Check that the load was a success
        bool loaded_properly = std::get<0>(RP->is_locked());
        REQUIRE(loaded_properly);

        // Set the path in REFPROP
        std::string path(RPPREFIX);
        char hpth[256] = "";
        strcpy(hpth, (path + std::string(255 - path.size(), ' ')).c_str());
        SETPATHdll(hpth, 255);
    }
    virtual ~REFPROPDLLFixture() {
        RP.reset(nullptr);
    }
    // And now, totally magical, the use of variadic function arguments in concert with type macros
    // Add many methods, each corresponding to a 1-to-1 wrapper of a function from the shared library
#define X(name) template<class ...Args> void name(Args&&... args){ return RP->getAddress<name ## _POINTER>(#name)(std::forward<Args>(args)...); };
    LIST_OF_REFPROP_FUNCTION_NAMES
#undef X

        int get_enum(const std::string &key) {
        char henum[256] = "", herr[256] = "";
        int ienum = 0, ierr = 0;
        REQUIRE(key.size() < 254);
        strcpy(henum, (key + std::string(255 - key.size(), ' ')).c_str());
        int ii = 0;
        GETENUMdll(ii, henum, ienum, ierr, herr, 255, 255);
        CAPTURE(key);
        CAPTURE(herr);
        REQUIRE(ierr == 0);
        return ienum;
    }

    REFPROPResult REFPROP(const std::string &_hFld, const std::string &_hIn, const std::string &_hOut, int unit_system, int iMass, int iFlag, double a, double b, std::vector<double> &z) {
        char hFld[10001], hIn[256], hOut[256];
        REQUIRE(_hFld.size() < 9999);
        REQUIRE(_hIn.size() < 254);
        REQUIRE(_hOut.size() < 254);
        // Pad z with zeros
        if (z.size() < 20){
            auto old_size = z.size();
            z.resize(20);
            for (auto i = old_size; i < 20; ++i) {
                z[i] = 0;
            }
        }
        REQUIRE(z.size() >= 20);
        strcpy(hFld, (_hFld + std::string(10000-_hFld.size(),' ')).c_str());
        strcpy(hIn, (_hIn + std::string(255-_hIn.size(), ' ')).c_str());
        strcpy(hOut, (_hOut + std::string(255-_hOut.size(), ' ')).c_str());

        std::vector<double> Output(200, 0.0), x(20, 0.0), y(20, 0.0), x3(20, 0.0);
        double q = 0;
        int iUnit = 0, ierr = 0;
        char herr[256] = "", hUnits[256] = "";

        REFPROPdll(hFld, hIn, hOut, unit_system, iMass, iFlag, a, b, &(z[0]), &(Output[0]), hUnits, iUnit, &(x[0]), &(y[0]), &(x3[0]), q, ierr, herr, 10000, 255, 255, 255, 255);
        REFPROPResult res  = {z, Output, std::string(hUnits), iUnit, x, y, x3, q, ierr, std::string(herr) };
        return res;
    }
    auto ALLPROPS(const std::string& _hOut, int unit_system, int iMass, int iFlag, double T_K, double rho_moldm3, const std::vector<double>& z) {
        // Prepare hOut string
        char hOut[10001];
        REQUIRE(_hOut.size() < 10000);
        strcpy(hOut, (_hOut + std::string(10000 - _hOut.size(), ' ')).c_str());

        // Pad z with zeros
        auto znew = z;
        if (z.size() < 20) {
            auto old_size = z.size();
            znew.resize(20);
            for (auto i = old_size; i < 20; ++i) {
                znew[i] = 0;
            }
        }
        REQUIRE(znew.size() >= 20);
        
        std::vector<double> Output(200, 0.0);
        int ierr = 0;
        std::vector<int> iUnit(200);
        char herr[256] = "", hUnits[10001] = "";

        ALLPROPSdll(hOut, unit_system, iMass, iFlag, T_K, rho_moldm3, &(znew[0]), &(Output[0]), hUnits, &(iUnit[0]), ierr, herr, 10000, 10000, 255);
        ALLPROPSResult res = {Output, std::string(hUnits), iUnit, ierr, std::string(herr) };
        return res;
    }
    void FLAGS(const std::string &_hFlag, int jflag, int &kflag, bool check_kflag = true){
        char hFlag[256] = "";
        REQUIRE(_hFlag.size() <= 255);
        strcpy(hFlag, (_hFlag.c_str() + std::string(255 - _hFlag.size(), ' ')).c_str());

        char herr[256] = "";
        int ierr = 0;
        FLAGSdll(hFlag, jflag, kflag, ierr, herr, 255, 255);
        CAPTURE(herr);
        REQUIRE(ierr == 0);
        if (jflag != -999 && check_kflag){
            REQUIRE(kflag == jflag);
        }
    }
    void SETREF(const std::string &ref, int ixflag, std::vector<double> &z, double& h0, double& s0, double& T0, double& p0, int &ierr, std::string &herr) {
        char hFlds[3] = "";
        REQUIRE(ref.size() <= 3);
        strcpy(hFlds, (ref.c_str() + std::string(3 - ref.size(), ' ')).c_str());
        char herrset[255] = "";
        if (z.size() != 20){ throw std::invalid_argument("z must be of length 20"); };
        SETREFdll(hFlds, ixflag, &(z[0]), h0, s0, T0, p0, ierr, herrset, 3, 255);
        herr = std::string(herrset);
        
    }
    void SETFLUIDS(const std::string &flds, int &ierr, std::string &herr) {
        char hFlds[10001] = "";
        REQUIRE(flds.size() <= 10000);
        strcpy(hFlds, (flds.c_str() + std::string(10000 - flds.size(), ' ')).c_str());
        ierr = 0;
        SETFLUIDSdll(hFlds, ierr, 10000);
        char herrsetup[255] = "";
        if (ierr != 0) {
            ERRMSGdll(ierr, herrsetup, 255);
        }
        herr = std::string(herrsetup);
    }
    void SETUP(const int nc_, const std::string& flds, const std::string &HMX, const std::string &ref, int& ierr, std::string& herr) {
        char hFlds[10001] = "", hhmx[256] = "", href[4] = "";
        REQUIRE(flds.size() <= 10000);
        strcpy(hFlds, (flds.c_str() + std::string(10000 - flds.size(), ' ')).c_str());
        strcpy(hhmx, (HMX.c_str() + std::string(255 - HMX.size(), ' ')).c_str());
        strcpy(href, (ref.c_str() + std::string(3 - ref.size(), ' ')).c_str());
        char herrsetup[255] = ""; 
        int nc = nc_;
        SETUPdll(nc, hFlds, hhmx, href, ierr, herrsetup, 10000, 255, 3, 255);
        herr = std::string(herrsetup);
    }

    auto SETMIXTURE(const std::string &MIX) {
        char hMix[10001] = "";
        REQUIRE(MIX.size() <= 10000);
        strcpy(hMix, (MIX.c_str() + std::string(10000 - MIX.size(), ' ')).c_str());
        int ierr = 0;
        std::vector<double> z(20);
        SETMIXTUREdll(hMix, &(z[0]), ierr, 10000);
        char herrsetup[255] = "";
        if (ierr != 0) {
            ERRMSGdll(ierr, herrsetup, 255);
        }
        std::string herr = std::string(herrsetup);
        return std::make_tuple(z, ierr, herr);
    }
    auto NAME(int icomp) {
        char hnam[13] = ""; memset(hnam,' ',12);
        char hn80[81] = ""; memset(hn80,' ',80);
        char hcasn[13] = ""; memset(hcasn,' ',12);
        NAMEdll(icomp, hnam, hn80, hcasn, 12, 80, 12);
        return std::make_tuple(std::string(hnam), std::string(hn80), std::string(hcasn));
    }
    std::pair<int, std::string> SETMOD(int Ncomp, const std::string &type, const std::string &mix, const std::string &model) {
        char htype[4] = "", hmix[4] = "", hmodel[4] = "";
        REQUIRE(type.size() == 3);
        REQUIRE(mix.size() == 3);
        REQUIRE(model.size() == 3);
        strcpy(htype, type.c_str());
        strcpy(hmix, mix.c_str());
        strcpy(hmodel, model.c_str());

        int ierr = 0; char herr[256] = ""; memset(herr,' ',255);
        SETMODdll(Ncomp, htype, hmix, hmodel, ierr, herr, 3, 3, 3, 255);
        return std::make_pair(ierr, std::string(herr));
    }

    std::tuple<std::string, std::string> GETMOD(int Ncomp, const std::string &type) {
        char htype[4] = "", hcode[4] = "   ", hcite[256] = ""; memset(hcite, ' ', 255);
        strcpy(htype, type.c_str());
        GETMODdll(Ncomp, htype, hcode, hcite, 3, 3, 255);
        return std::make_tuple(std::string(hcode), std::string(hcite));
    }
    auto SATSPLN(const std::vector<double>& z) {
        int ierr = -1;
        char herr[256] = "";

        // Pad z with zeros
        auto znew = z;
        if (z.size() < 20) {
            auto old_size = z.size();
            znew.resize(20);
            for (auto i = old_size; i < 20; ++i) {
                znew[i] = 0;
            }
        }
        REQUIRE(znew.size() >= 20);

        SATSPLNdll(&(znew[0]), ierr, herr, 255);
        return std::make_tuple(ierr, herr);
    }
    auto SATGUESS(int kph, int iprop, double val, const std::vector<double>& z) {

        // Pad z with zeros
        auto znew = z;
        if (z.size() < 20) {
            auto old_size = z.size();
            znew.resize(20);
            for (auto i = old_size; i < 20; ++i) {
                znew[i] = 0;
            }
        }
        REQUIRE(znew.size() >= 20);
        SATGUESSResult o; 
        o.y.resize(20);
        switch (iprop) {
        case 1:
            o.T = val; break;
        default:
            throw std::invalid_argument("Bah");
        }
        SATGUESSdll(kph, iprop, &(znew[0]), o.T, o.p, o.D, o.h, o.s, o.Dy, &(o.y[0]),o.ierr, o.herr, 255);
        return o;
    }
};

#endif