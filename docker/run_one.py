import argparse, zipfile, os, glob, sys, subprocess, shutil, hashlib

from ansi2html import Ansi2HTMLConverter

def get_path_hash(root_path):
    """ 
    Utility function to hash the path to remove all the annoying path separtors and so on... 
    """
    return hashlib.sha224(root_path.encode('ascii')).hexdigest()

def build_REFPROP_zip(*, root, zippath):
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
            for stem in ['DLLFILES/*.*','*.*']:
                folder_path = os.path.join(dequote(root), folder, stem)
                files = glob.glob(folder_path)
                if not files and stem == '*.*':
                    raise ValueError('No files match '+folder_path)
                for file in files:
                    rpzip.write(file, arcname=depath(file, folder=folder))

        taglist = os.path.join(root,'.just_these_tags')
        if os.path.exists(taglist):
            rpzip.write(taglist, arcname=depath(taglist, folder='.'))

def run_test(*, root, test):
    """ Actually run the specified test """

    prefix = get_path_hash(root) + '_' + test

    # Wipe the output folder of the test
    #outfold = os.path.join(test, 'output')
    #if os.path.exists(outfold):
    #    shutil.rmtree(outfold)
    #os.makedirs(outfold)

    # Collect the REFPROP files
    build_REFPROP_zip(root=root, zippath=os.path.join(test, 'REFPROP.zip'))

    # Run the test
    with open(prefix+'_build_run.log', 'w') as stdout:
        subprocess.call('docker-compose up --build', cwd=test, shell=True, stdout=stdout, stderr=stdout)

    # Convert ANSI color codes in the log to html
    html = Ansi2HTMLConverter().convert(open(prefix+'_build_run.log').read())
    with open(prefix+'_build_run.html', 'w') as fp:
        fp.write(html)

    # Collect the bits and pieces from the run into a zip file
    shutil.make_archive(prefix, 'zip', os.path.join(test, 'output'))

if __name__ == '__main__':

    mnt = '/media/Q/'
    if sys.platform.startswith('win'):
        mnt = 'Q:/Public/'
    sys.argv += ['--root', mnt+'Eric/INSTALL/1000/10.0','--test','test']
    parser = argparse.ArgumentParser(description='Run a specified test, and generate a zip file with perhaps relevant output')
    parser.add_argument('--root', type=str, required=True, nargs=1, 
        help='The root path of the REFPROP installation from which to take the folders FLUIDS, MIXTURES, and FORTRAN')
    parser.add_argument('--test', type=str, required=True, nargs=1, 
        help='The selected test to run', choices=['gcov','valgrind','test'])
    args = parser.parse_args()
    run_test(root=args.root[0], test=args.test[0])