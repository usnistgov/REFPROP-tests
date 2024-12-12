template = """HMX               !Mnemonic for mixture model, must match hfmix on call to SETUP.
5                 !Version number

! Changelog:
! ---------
!
!  THIS FILE IS FOR TESTING ONLY!!!!!
!


#BNC              !Binary mixing coefficients
BNC
? Binary mixing coefficients for the various mixing rules used with the HMX model:
?
? KWi:  (i = 1,2,3,...,A,B,...)  --->  Kunz-Wagner mixing rules
?   model     BetaT     GammaT    BetaV     GammaV    Fij      not used
?
!
?BUTANE/ETHANE                      [BUTANE/ETHANE]
?Bell (202X)
  7b3b4080/434e2a40
    XR0    1.     0.990830895552353       1.       1.    0.             0.             0. 0. 0. 0. 0. 0.
    TC5    0.0      0.0     0.0       0.             0.             0.             0. 0. 0. 0. 0. 0.
    VC5    0.0      0.0     0.0       0.             0.             0.             0. 0. 0. 0. 0. 0.
!

#MXM              !Mixture model specification
XR0  Reducing functions only
?
?```````````````````````````````````````````````````````````````````````````````
? For testing only
!```````````````````````````````````````````````````````````````````````````````
 BetaT    GammaT   BetaV    GammaV  not used not used      !Descriptors for binary-specific parameters
  1.0      1.0      1.0      1.0      0.0      0.0         !Default values (i.e. ideal-solution)
  1 4      0        0 0      0 0      0 0      0 0         !# terms and # coefs/term for normal terms, Kunz-Wagner terms, and Gaussian terms.  3rd column is not used.
  0.0 1.0 1.0 1.0

%MXM%

@END"""

MXMblock = """#MXM              !Mixture model specification
{CODE}  Reducing functions only
?
?```````````````````````````````````````````````````````````````````````````````
? For testing only
!```````````````````````````````````````````````````````````````````````````````
 BetaT    GammaT   BetaV    GammaV  not used not used      !Descriptors for binary-specific parameters
  1.0      1.0      1.0      1.0      0.0      0.0         !Default values (i.e. ideal-solution)
  1 4      0        0 0      0 0      0 0      0 0         !# terms and # coefs/term for normal terms, Kunz-Wagner terms, and Gaussian terms.  3rd column is not used.
  0.0 1.0 1.0 1.0
"""

for Nmax in [20, 45, 50, 99, 200]:
    MXMS = '\n\n'.join([MXMblock.format(CODE=f'X{i:02d}') for i in range(Nmax)])
    HMX = template.replace('%MXM%', MXMS)
    with open(f'../resources/HMX{Nmax}.BNC','w') as fp:
        fp.write(HMX)
    