
# About

Tests written by Ian Bell, NIST, 2018, with the [Catch](https://github.com/catchorg/Catch2) testing framework.  This framework allows for only a subset of the tests to be run, for instance you can list the tags that are found:

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

# "Local" Builds

The tests require CMake for building. Doing so follows the typical CMake pattern:

```
mkdir bld
cd bld
cmake ..
cmake --build .
```

Running the tests requires that the ``RPPREFIX`` environment variable be set to point to the folder that contains the shared library of REFPROP as well as the ``FLUIDS`` and ``MIXTURES`` folders. On standard installations on windows this is set by the installer. The ``RESOURCES`` environment variable may also be set to indicate the location of some files used for testing with perturbed or erroneous inputs. That folder is in the top level of the repository.

# Running tests

All the test runners are contained in docker containers. 

Running the tests requires:
* ``docker``
* ``docker-compose``

Zip up the ``FLUIDS``, ``FORTRAN``, and ``MIXTURES`` folders into a zip file called ``REFPROP.zip``, maybe with something like:
``` bash
cp -rv /media/R/FLUIDS/ .
cp -rv /media/R/FORTRAN/ .
cp -rv /media/R/MIXTURES/ .
zip -r REFPROP.zip FLUIDS FORTRAN MIXTURES
rm -rf FLUIDS FORTRAN MIXTURES
```

Then copy the ``REFPROP.zip`` file into the folder of the test you want to run (e.g. ``docker/gcov`` for the ``gcov`` coverage tests).  At the command line, first make sure you have the ``REFPROP.zip`` file, then away we go:
``` bash
ls REFPROP.zip
docker-compose up
```

It's the same procedure to run the normal Catch tests (``docker/test``) or to run the valgrind memory checks for each tag (``docker/valgrind``).

Within the ``docker`` container are some scripts for running one or more tests based on the docker images. Please read the ``README.md`` in the ``docker`` folder.

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