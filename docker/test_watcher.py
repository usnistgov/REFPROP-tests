"""
Given a path, sit in an infinite loop, and for each update to anything 
in the folder or subfolders, fire off all the tests and report results

Ian Bell, Nov. 2020
"""
import glob, sys, os, json, time, timeit, shutil

import run_one
import emailing

def are_new_files(root_path):
    """
    Check if there are updates to anything in the root_path folder or subfolders.

    Return True if a modification is found, or if the cache file does not exist
    Return False if files are up-to-date
    """
    
    time_function = os.path.getmtime

    # Find the most recently modified file in root and all folders (recursively)
    globs = glob.glob(root_path + '/*') + glob.glob(root_path + '/**/*') + glob.glob(root_path + '/.*')
    LatestFile = max(globs, key=time_function)
    seconds_since_epoch = time_function(LatestFile)
    print('Most recent update:', LatestFile, '@', time.ctime(seconds_since_epoch))

    # Hash the given root_path, so there is one cachefile per root path
    # Perhaps overkill...
    path_hash = run_one.get_path_hash(root_path)
    cache_file = path_hash + '_ctimecache.json'
    def write_cache(seconds_since_epoch):
        with open(cache_file, 'w') as fp:
            fp.write(json.dumps({'seconds_since_epoch': seconds_since_epoch}))

    # If cache file is missing, then no reference time is available, new files must be present
    if not os.path.exists(cache_file):
        write_cache(seconds_since_epoch)
        return True
    else:
        # Load the last modified time from cache
        former_time = json.load(open(cache_file))['seconds_since_epoch']
        if seconds_since_epoch > former_time:
            write_cache(seconds_since_epoch)
            return True
        else:
            return False

if not os.path.exists('.email_recipients.json'):
    raise ValueError('You need to create the secret file listing email recipients')
email_recipients = json.load(open('.email_recipients.json'))['recipients']

def email_results(tests, *, root_path):
    """
    Send an email with the results files
    """
    path_hash = run_one.get_path_hash(root_path)
    os.makedirs(path_hash)
    files = []
    for test in tests:
        prefix = path_hash + '_' + test
        shutil.copy2(prefix+'.zip', os.path.join(path_hash, test+'.zip'))
        shutil.copy2(prefix+'_build_run.html', os.path.join(path_hash, test+'_build_run.html'))
    files = glob.glob(os.path.join(path_hash, '*'))
    emailing.send_email(files, recipients=email_recipients, subject='Test results from path:'+root_path)
    shutil.rmtree(path_hash)

def infinite_watch(root_path, *, force=False):
    while True:
        print('Starting watching of path:', root_path)
        if are_new_files(root_path) or force:
            print('Testing beginning...')
            for test in test_suite:
                tic = timeit.default_timer()
                print('Starting test:',test)
                run_one.run_test(root=root_path, test=test)
                toc = timeit.default_timer()
                print('Completed',test,'in',toc-tic,'seconds')
            email_results(test_suite, root_path=root_path)
        print('No update, going to sleep for 60 seconds...')
        time.sleep(60)

test_suite = ['test','gcov','valgrind']

if __name__ == '__main__':
    mnt = '/media/Q/'
    if sys.platform.startswith('win'):
        mnt = 'Q:/Public/'
    # sys.argv += [mnt+'Eric/INSTALL/1000/10.0']
    # sys.argv += [mnt+'Eric/INSTALL/ALPHA']
    infinite_watch(root_path=sys.argv[-1])