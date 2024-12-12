import pint
import pandas, io
df = pandas.read_csv(io.StringIO("""what    key    DEFAULT    MOLAR SI    MASS SI    SI WITH C    MOLAR BASE SI    MASS BASE SI    ENGLISH    MOLAR ENGLISH    MKS    CGS    MIXED    MEUNITS
iUnits   XX     0    1    2    3    20    21    5    6    7    8    9    10
Temperature    T    K    K    K    C    K    K    F    F    K    K    K    C
Pressure    P    kPa    MPa    MPa    MPa    Pa    Pa    psia    psia    kPa    MPa    psia    bar
Density    D    mol/dm^3    mol/dm^3    kg/m^3    kg/m^3    mol/m^3    kg/m^3    lbm/ft^3    lbmol/ft^3    kg/m^3    g/cm^3    g/cm^3    g/cm^3
Enthalpy    H    J/mol    J/mol    J/g    J/g    J/mol    J/kg    Btu/lbm    Btu/lbmol    J/g    J/g    J/g    J/g
Entropy    S    (J/mol)/K    (J/mol)/K    (J/g)/K    (J/g)/K    (J/mol)/K    (J/kg)/K    (Btu/lbm)/R    (Btu/lbmol)/R    (J/g)/K    (J/g)/K    (J/g)/K    (J/g)/K
Speed    U    m/s    m/s    m/s    m/s    m/s    m/s    ft/s    ft/s    m/s    cm/s    m/s    cm/s
Kinematic vis.    KV    cm^2/s    cm^2/s    cm^2/s    cm^2/s    m^2/s    m^2/s    ft^2/s    ft^2/s    cm^2/s    cm^2/s    cm^2/s    cm^2/s
Viscosity    VIS    uPa-s    uPa-s    uPa-s    uPa-s    Pa-s    Pa-s    lbm/(ft-s)    lbm/(ft-s)    uPa-s    uPa-s    uPa-s    cpoise
Thermal    TCX    W/(m-K)    mW/(m-K)    mW/(m-K)    mW/(m-K)    W/(m-K)    W/(m-K)    Btu/(h-ft-R)    Btu/(h-ft-R)    W/(m-K)    mW/(m-K)    mW/(m-K)    mW/(m-K)
Surface    STN    N/m    mN/m    mN/m    mN/m    N/m    N/m    lbf/ft    lbf/ft    mN/m    dyne/cm    mN/m    mN/m
Molar    M    g/mol    g/mol    g/mol    g/mol    kg/mol    kg/mol    lbm/lbmol    lbm/lbmol    g/mol    g/mol    g/mol    g/mol"""),sep=' {2,}', engine='python')
df.info()
df.to_csv('aaa.csv')

import ctREFPROP.ctREFPROP as ct 
root = '/Users/ihb/Documents/Code/REFPROP-tests/RPbuilds/1733254076_e65b9ec6937edd36b1af26851eed114768649598'
RP = ct.REFPROPFunctionLibrary(root+'/librefprop.dylib')
RP.SETPATHdll(root)

Prfactors = {}
nufactors = {}
tdfactors = {}

ureg = pint.UnitRegistry()

ureg.define("pound_mass = 0.45359237 kg = lbm")
ureg.define("pound_mole = 0.45359237 kg*mol = lbmol")

for icol, col in enumerate(df):
    if icol < 2: continue
    def get_units(s):
        return ureg.Unit(s.replace('-','*').replace('psia','psi'))
    
    iUnits = int(df[col].iloc[0])
    
    uCP = get_units(df[col].iloc[5])
    uKV = get_units(df[col].iloc[7])
    uETA = get_units(df[col].iloc[8])
    uD = get_units(df[col].iloc[3])
    uTCX = get_units(df[col].iloc[9])
    uM = get_units(df[col].iloc[11])
    
    if 'mol' in str(uD):
        uD = uD*uM
        uCP = uCP/uM
    
    FLD = 'NITROGEN'
    US = col
    
    aa = RP.REFPROPdll(FLD,'TRIP','TD',iUnits,0,0,-1,-1,[1.0]) # Note you'll need to instantiate REFPROP
    print(US, aa.hUnits, aa.Output[0], aa.ierr, aa.herr)
    tdfactor = (1*uTCX/(uD*uCP)).to_base_units()/ureg.Quantity(1.0, aa.hUnits).to_base_units()
    tdfactors[iUnits] = tdfactor.m
    
    nufactor = (1*uETA/uD).to_base_units()/(1*uKV).to_base_units()
    assert(nufactor.dimensionless)
    nufactors[iUnits] = nufactor.m
    
    Prfactor = (1*uETA*uCP/uTCX).to_base_units()
    if Prfactor.dimensionless:
        Prfactors[iUnits] = Prfactor.m
        
print(nufactors)
print(Prfactors)
print(tdfactors)