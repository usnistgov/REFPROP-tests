#ifndef REFPROP_TESTS_UTILS
#define REFPROP_TESTS_UTILS

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp> 
#include <boost/algorithm/string/trim.hpp>
#include <locale>
#include <regex>
#include <string>
#include <sstream>

/**  Read in an entire file in one shot
 */
static inline std::string get_file_contents(const std::string &filename) {
    using std::ios;
    std::ifstream ifs(filename.c_str(), ios::in | ios::binary);
    if (!ifs) {
        throw std::invalid_argument("filename cannot be opened: " + filename);
    }
    // See https://stackoverflow.com/a/116220/1360263
    return static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str();
}

/** Take in a string, use the delimiter to split it up into parts
 *  Inspired by http://www.cplusplus.com/faq/sequences/strings/split/#getline
 */
static std::vector<std::string> str_split(const std::string &s, 
                                          const std::string & delimiters = "\n"){
    int current;
    int next = -1;
    std::vector<std::string> o;
    do {
        current = next + 1;
        next = static_cast<int>(s.find_first_of(delimiters, current));
        o.push_back(s.substr(current, next - current));
    } while (next != std::string::npos);
    return o;
}

/** Join a vector of strings into a single string, a la Python */
static std::string str_join(const std::vector<std::string>& vals, const std::string& delimiter = "\n") {
    if (vals.empty()) {
        throw std::invalid_argument("vals can't be empty in str_join");
    }
    std::string o = vals[0];
    for (auto i = 1; i < vals.size(); ++i){
        o += delimiter + vals[i];
    }
    return o;
}

/** Get all the binary pairs that can be found in HMX.BNC
 */
static std::vector<std::pair<std::string, std::string>> get_binary_pairs() {
    char* RPPREFIX = std::getenv("RPPREFIX");
    REQUIRE(strlen(RPPREFIX) != 0);
    namespace fs = boost::filesystem;

    fs::path target(fs::path(RPPREFIX) / fs::path("FLUIDS") / fs::path("HMX.BNC"));
    std::string contents = get_file_contents(target.string());
    std::regex pattern(R"(([\w \-]+\/[\w \-]+)\s{2,}\[)");
    std::smatch pair_match;
    std::vector<std::pair<std::string, std::string>> pairs;
    for (auto &&line : str_split(contents)) {
        if (std::regex_search(line, pair_match, pattern)){
            std::string flds = pair_match[1];
            boost::algorithm::trim(flds); // inplace
            auto fld = str_split(flds, "/");
            pairs.emplace_back(fld[0], fld[1]);
        }
    }
    return pairs;
}

/** Convert a string to a double-precision number.  The "." is 
 *  forced as the delimiter, "," as decimal is not allowed
 *
 *  \note Taken from CoolProp
 */ 
static double string_to_double(const std::string &s){
    std::stringstream ss(s);
    char c = '.';

    struct delim : std::numpunct<char> {
        char m_c;
        delim(char c) : m_c(c) {};
        char do_decimal_point() const { return m_c; }
    };

    ss.imbue(std::locale(ss.getloc(), new delim(c)));
    double f;
    ss >> f;
    if (ss.rdbuf()->in_avail() != 0) {
        throw "fraction [%s] was not converted fully " + s;
    }
	return f;
}

/// Container structure for holding values
struct predef_mix_values {
    double Wmol, Tc, pc, rhoc;
    std::vector<double> molar_composition;
};
/// Read in all the predefined mixture files and extract numerical values
static predef_mix_values get_predef_mix_values(const std::string &fname) {
    char* RPPREFIX = std::getenv("RPPREFIX");
    REQUIRE(strlen(RPPREFIX) != 0);
    namespace fs = boost::filesystem;

    fs::path target(fs::path(RPPREFIX) / fs::path("MIXTURES") / fs::path(fname));
    std::string contents = get_file_contents(target.string());
    auto lines = str_split(contents);

    // Get Wmol and crit values
    auto val_string = str_split(lines[1], " ");
    std::vector<double> vals;
    for (auto &c : val_string) {
		std::string cc = c;
        boost::algorithm::trim(cc); // inplace
        if (!cc.empty()){
			vals.emplace_back(string_to_double(cc));
        }
    }
    if (vals.size() != 4) {
        throw "Length of values is not 4 in "+ fname;
    }
    boost::algorithm::trim(lines[2]);
    std::vector<double> molar_compositions;
    auto Ncomp = static_cast<int>(string_to_double(lines[2]));
    for (auto i = 0; i < Ncomp; ++i) {
        boost::algorithm::trim(lines[3+Ncomp+i]);
        molar_compositions.emplace_back(string_to_double(lines[3 + Ncomp + i]));
    }
    return predef_mix_values{vals[0], vals[1], vals[2], vals[3], molar_compositions};
}

/** Normalize the path, removing multiple path delimiters, and yielding the "correct"
 *  path separator for the given platform
 */
static std::string normalize_path(const std::string &path_as_string) {
    namespace fs = boost::filesystem;
    return boost::filesystem::canonical(fs::path(path_as_string)).string();
}

/** Glue two path segments together and then normalize
*/
static std::string path_join_and_norm(const std::string &left, const std::string &right){
    namespace fs = boost::filesystem;
    return normalize_path((fs::path(left) / fs::path("/") / fs::path(right)).string());
}

/** Find all files in a given folder that match the specified file extension
 *  The extension should be specified with the period, as in ".FLD"
 */
static std::vector<std::string> get_files_in_folder(const std::string &folder, const std::string &extension) {
    
    namespace fs = boost::filesystem;
    std::vector<std::string> files;

    fs::directory_iterator it(folder), eod;
    BOOST_FOREACH(fs::path const &p, std::make_pair(it, eod))
    {
        if (fs::is_regular_file(p) && p.extension() == fs::path(extension))
        {
            std::string s = fs::canonical(p).filename().replace_extension("").string();
            files.push_back(s);
        }
    }
    return files;
}

/// Internal method for finding files in a subfolder of RPPREFIX environmental variable
static std::vector<std::string> get_files_in_RPPREFIX_folder(const std::string &folder, const std::string &extension) {
    char* RPPREFIX = std::getenv("RPPREFIX");
    REQUIRE(strlen(RPPREFIX) != 0);
    namespace fs = boost::filesystem;

    fs::path targetDir(fs::path(RPPREFIX) / fs::path(folder));
    return get_files_in_folder(targetDir.string(), extension);
}

/// Find all the pure fluids
static std::vector<std::string> get_pure_fluids_list() {
    return get_files_in_RPPREFIX_folder("FLUIDS", ".FLD");
}

/// Find all the predefined mixtures
static std::vector<std::string> get_predefined_mixtures_list() {
    return get_files_in_RPPREFIX_folder("MIXTURES", ".MIX");
}

/// Read each fluid file and locate the ones that have either PX0 or PH0 models
static std::vector<std::string> fluids_with_PH0_or_PX0() {
    std::vector<std::string> o;
    for (auto &fluid : get_pure_fluids_list()) {
        char* RPPREFIX = std::getenv("RPPREFIX");
        REQUIRE(strlen(RPPREFIX) != 0);
        namespace fs = boost::filesystem;
        std::string path = (fs::path(RPPREFIX) / fs::path("FLUIDS") / fs::path(fluid+".FLD")).string();
        std::string contents = get_file_contents(path);
        for (auto &line : str_split(contents)) {
            std::smatch match;
            if (std::regex_search(line, match, std::regex(R"(^P[HX]0)"))) {
                o.emplace_back(fluid);
                break;
            }
        }
    }
    return o;
}

#endif
