import argparse, zipfile, os, glob, sys, subprocess

def build_REPROP_zip(*, root, zippath):
    """ Build a zip file with all the things to go into docker container """

    def depath(fname, *, folder):
        """ Remove the leading path, leaving just folder+file """
        return os.path.join(folder, os.path.split(fname)[1])

    def dequote(path):
        if path[0] == '"' and path[1] == '"':
            return path.lstrip('"').rstrip('"')
        else:
            return path

    with zipfile.ZipFile(zippath, 'w') as rpzip:
        for folder in ['FLUIDS','MIXTURES','FORTRAN']:
            folder_path = os.path.join(dequote(root), folder, '*.*')
            files = glob.glob(folder_path)
            if not files:
                raise ValueError('No files match '+folder_path)
            for file in files:
                rpzip.write(file, arcname=depath(file, folder=folder))

def run_one(args):
    """ Actually run the specified test """
    root = args.root[0]
    test = args.test[0]
    build_REPROP_zip(root=root, zippath=os.path.join(test, 'REFPROP.zip'))
    with open('build_run.log', 'w') as stdout:
        subprocess.check_call('docker-compose up --build', cwd=test)

    def debrief(recipients):
        """ 1. Collect the things from this test, make a zip file of them """
        """ 2. Email the results to the desired email address recipients """
        pass

if __name__ == '__main__':

    mnt = '/media/Q/'
    if sys.platform.startswith('win'):
        mnt = 'Q:/'
    sys.argv += ['--root', mnt+'Public/Eric/install/ALPHA','--test','test']
    parser = argparse.ArgumentParser(description='Run a specified test, and generate a zip file with perhaps relevant output')
    parser.add_argument('--root', type=str, required=True, nargs=1, 
        help='The root path of the REFPROP installation from which to take the folders FLUIDS, MIXTURES, and FORTRAN')
    parser.add_argument('--test', type=str, required=True, nargs=1, 
        help='The selected test to run', choices=['gcov','valgrind','test'])
    args = parser.parse_args()
    run_one(args)