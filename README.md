
# About

Tests written by Ian Bell, NIST, 2018, with the [Catch](https://github.com/catchorg/Catch2) testing framework. This framework allows for only a subset of the tests to be run, for instance you can list the tags that are found:

``` bash
>main -t
All available tags:
   1  [100comps]
   1  [100Reloads]
   2  [911]
   ....
49 tags

```
and then run only a subset of them:

```
main [REDX] [TCX]
```
for instance.  More information can be obtained by running ``main -h`` at the command line.

The tests compile into a small executable, and at runtime the user selects (via environment variables) which version of REFPROP is to be tested.

# "Local" Builds

The native tests require:
* CMake (downloads: https://cmake.org/download/, or better, install from your package manager (mac: homebrew, windows: chocolatey)) for building. 
* A compiler set for your platform

Building the testing executable follows the typical CMake pattern from the folder containing this file:

```
mkdir bld
cd bld
cmake ..
cmake --build .
```

Here it is assumed that default compiler is to be used on your platform (visual studio on windows, g++ on linux, apple-clang on mac). The default can be overwritten by command line arguments (see the docs for CMake).

Once the executable has been build, tunning the tests requires that the ``RPPREFIX`` environment variable be set to point to the folder that contains the shared library of REFPROP as well as the ``FLUIDS`` and ``MIXTURES`` folders. On standard installations on windows this is set by the installer. The ``RESOURCES`` environment variable may also be set to indicate the location of some files used for testing with perturbed or erroneous inputs. That folder is in the top level of the repository containing this file.

# Regression Testing

Regression testing runs the tests on multiple versions of REFPROP to see when bugs were introduced or squashed. This process can be run inside a docker container (see the ``README.md`` file in the ``docker`` folder for more information)

The ``native_regression.py`` Python script can be used to test multiple versions of REFPROP on the main computer (not inside a docker container), running all tags against the different versions of the code. The process goes like this:

## 1. Generation of candidates. 

Make some combination of:
  
* copies of folders of REFPROP sources in ``docker/sources`` 
* symlinks in ``docker/sources`` to folders of sources elsewhere
* ``.zip`` files of sources in ``docker/sources``

There is [information below in the cookbook](#cookbook) about how you can generate a ``.zip`` file at each commit in git repository that changes any files in particular folder (or any other filter you might like to do; for instance all commits that change any ``.FLD`` file)

## 2. Run the native regression tests. 

The ``native_regression.py`` script has all the bits and pieces, here is a possible driver script:

```python
from native_regression import VersionBuilder
vb = VersionBuilder(tester_exe=r'bld/Debug/main')
  # basenames needs to be defined as the things to run. You could use a pattern, something like:
  # 
  # sorted([ f.name for f in os.scandir('docker/sources') 
  #          if (not f.is_dir() and (f.name.startswith('17') or f.name.startswith('16'))) ])
  #
  # or all subfolders:
  #
  # sorted([ f.name for f in os.scandir('docker/sources') if f.is_dir() ])
sources = ['docker/sources/' + f for f in basenames] 

# TODO: this could be multithreaded as it is embarassingly parallel
for source in sources:
    try:
        # Build the version of REFPROP to be tested
        dest = 'RPbuilds/' + source.replace('.zip', '').replace('docker/sources/','')
        vb.build_REFPROP(source=source, dest=dest)
        
        # And run the tests
        outputbase = 'RPoutputs/' + source.replace('.zip', '').replace('docker/sources/','')
        os.makedirs(outputbase, exist_ok=True)
        vb.run_tests(RPbuild=dest, tags=None, outputbase=outputbase)
    except BaseException as be:
        print(be)
    
# ######## And collect all the results into a nice table
paths = ['RPoutputs/'+bn.replace('.zip', '') for bn in basenames]
vb.build_table(outputroot='RPoutputs', 
               paths=paths, 
               sort_by=[paths[-1], 'tag'], 
               sort_kwargs=dict(ascending=[False, True]))
```

In this context, the test runner (here ``'bld/Debug/main'``) must be compiled first, and then passed to the regression tester, which will generate a handy HTML output of the failure counts for each tag for each version of REFPROP. Each tag with errors then also has a hyperlink to the failures. Deciphering the individual failures can definitely be a bit tricky, and will likely require to read the C++ sources for the test to know what order of steps was required to get the error to appear (sometimes it is a multi-step process).

TODO?: use a regex on the generated .txt files to replace the paths with hyperlinks to the location in the C++ sources

# Debugging Methods

## 1. Identify and fix memory issues

Before digging deeper into a failure, it is a good idea to first make sure that there are no uninitialized variables being used in conditional branches within the Fortran code. The most reliable way to check this is to use the valgrind memory checker. The valgrind tool is only available on linux, so the valgrind docker container was built for identifying memory issues. The tests should yield a 100% pass rate, with no errors. Helpfully, running the C++ tests through the valgrind checker ALSO finds memory problems in the calling C++ code. The backtraces from valgrind are quite clear, so you need to go step-by-step through the backtrace and identify which of the functions in the call stack did not initialize a variable that was later used in a lower function in the call stack.

The downside of the valgrind checker is that it is SLOW because it is doing a lot of memory checking behind the scenes but it saves a huge amount of debugging time later on.

If you don't first fix the memory issues it is common to see different behaviors of the code depending on compiler and operating system and Debug/Release builds that are perplexing. So, first fix ALL the memory issues.

Ok, memory issues fixed. On to more detailed debugging.

## 2. Debugging

The approach used for debugging REFPROP differs based on the operating system.

### Mac

This one is straightforward. You can't. None of the debuggers for Fortran work on arm64 arch (as needed for Apple M1, M2... chips), at least as of December 2024. 

### Linux

A good approach for debugging (though perhaps not the most convenient) is to compile the shared library of REFPROP in a Debug build, and load the build into VSCode. Then you can set up the debugger to load an arbitrary program (could be Python, calling a testing script, or the Catch testing framework described above), which would call the shared library, and allow you to drop into the debugger at a particular line in the Fortran code. This works, but it can be tricky to configure properly, so... 

### Windows

The Visual Studio debugging environment is the one that is primarily used to debug REFPROP. You need to install Visual Studio first, followed by the [Intel OneAPI HPC Toolkit](https://www.intel.com/content/www/us/en/developer/tools/oneapi/hpc-toolkit.html#gs.j8gfz6). It is recommended to generate the Visual Studio Project files with the build tools in [REFPROP-cmake](https://github.com/usnistgov/REFPROP-cmake). Follow the typical CMake pattern. Something like:

```
git clone https://github.com/usnistgov/REFPROP-cmake --recursive
cd REFPROP-cmake 
mkdir bld
cd bld
cmake .. -DREFPROP_FORTAN_PATH=C:/Path/to/REFPROP 
```
where the path to your REFPROP installation folder is adjusted as necessary. Then, open up the project in Visual Studio and compile in Debug mode.

After compilation is complete, you should have a DLL in the ``Debug`` folder and you will want to copy the ``FLUIDS`` and ``MIXTURES`` folders to sit next to the ``REFPRP64.DLL`` file that was compiled. 

Once the debug version of REFPROP has been compiled, you can run any code you want to step into the Fortran debugger. In the Visual Studio Property page, you will want to set the paths to the executable you want to run, as well as any arguments the executable should take, and what working directory should be used. Make sure that in whatever code you are running, the correct REFPROP path is used, which should be the one for the Debug folder (make sure to use an absolute path!). To run a script with Python, a convenient way to quickly implement a simple test, that could look like:

<img src="imgs/VSconfig.PNG" alt="Screenshow showing how to setup Visual Studio to run a Python script" style="width: 400px;"/>

Note: you need to use absolute paths for working directory and executable. It is inconvenient, but necessary.

# License

The tests in this repository are all in the public domain, while NIST REFPROP is not open source.

# Cookbook

Here is how to walk a git repository, making a zip file from each commit that 
changes a file found in the ``FORTRAN`` folder (or subfolder) of the repository

```python
import os, sys
from git import Repo

repo = Repo(REFPROP_WORKING_CODE)
FORTRAN_commits = []
for commit in repo.iter_commits():
  print(commit, commit.author.name, commit.authored_date, commit.stats.files)
  if any(['FORTRAN' in path for path, _ in commit.stats.files.items()]):
    FORTRAN_commits.append(commit.hexsha)
    with open(f'{commit.authored_date}_{commit.hexsha}.zip', 'wb') as fp:
      repo.archive(fp, commit.hexsha, format='zip')

with open('commits_touching_FORTRAN.json', 'w') as fp:
  fp.write(json.dumps(FORTRAN_commits))
  print(len(FORTRAN_commits))
```

This can be used to do a very granular regression testing, running the tests against every single commit where some change condition is met. You could in principle also do the same at every commit, if that was of interest.