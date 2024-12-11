import subprocess, os, sys, shutil, glob, re
import pandas 

class VersionBuilder:
    def __init__(self, tester_exe):
        self.tester_exe = tester_exe
        assert(os.path.exists(tester_exe))

    def clean(self):
        # Wipe contents of output folder 
        for g in glob.glob(self.base + '/output/*'):
            if os.path.isdir(g):
                shutil.rmtree(g)
            else:
                os.remove(g)

    def build_REFPROP(self, *, source, dest):
        
        os.makedirs(dest, exist_ok=True)
        
        if not os.path.exists(os.path.join(dest,'REFPROP-cmake')):
            subprocess.check_call('git clone https://github.com/usnistgov/REFPROP-cmake --recursive', shell=True, cwd=dest)
            
        if source.endswith('.zip'):
            shutil.unpack_archive(source, extract_dir=dest)
            PATH = os.path.abspath(dest+'/FORTRAN')
        else:
            PATH = os.path.abspath(source)+'/FORTRAN'
        DEST = os.path.abspath(dest)
            
        subprocess.check_call(f'cmake -S {DEST}/REFPROP-cmake -B {DEST} -DREFPROP_FORTRAN_PATH={PATH} -DCMAKE_BUILD_TYPE=Release', shell=True)
        subprocess.check_call(f'cmake --build {DEST} --config Release', shell=True)
        if not os.path.exists(dest+'/FLUIDS'):
            shutil.copytree(source+'/FLUIDS', dest+'/FLUIDS')
            shutil.copytree(source+'/MIXTURES', dest+'/MIXTURES')
        if 'win' in sys.platform:
            shutil.copy2(f'{DEST}/Release/REFPRP64.DLL', f'{DEST}/REFPRP64.DLL')

    def run_tests(self, RPbuild, *, outputbase, tags=None):
        
        # Collect all the tags to be run
        if tags is None:
            all_tags = []
            output = subprocess.run(f'{self.tester_exe} --list-tags', shell=True, stdout = subprocess.PIPE).stdout.decode('utf-8')
            for il, line in enumerate(output.split('\n')[1::]):
                if not line or '[' not in line: continue
                tag = '[' + line.split('[')[1]
                if 'veryslow' in tag: continue # Don't run the veryslow tests
            #    if 'predef_mix' not in tag: continue
                all_tags.append(tag.strip())
            tags = all_tags

        environ = {'RPPREFIX': RPbuild, 'RESOURCES': os.path.abspath('resources')}
        if 'win' in sys.platform:
            environ = os.environ | {'RPPREFIX': RPbuild, 'RESOURCES': os.path.abspath('resources')}
        for tag in tags:
            root =  tag.replace('[', '').replace(']','') + '.txt'
            print(tag, ' --> ', root)
            cmd = f'{self.tester_exe} ' + tag
            with open(outputbase+'/err_'+root,'w') as fp_stdout:
                fp_stdout.write('About to run command: '+cmd+'\n')
                fp_stdout.flush()
                subprocess.run(cmd, shell=True, stdout=fp_stdout, stderr=fp_stdout, env=environ)

        # Store all the outputs in zip archive
        shutil.rmtree(outputbase+'/errors', ignore_errors=True)
        shutil.rmtree(outputbase+'/ok', ignore_errors=True)
        os.makedirs(outputbase+'/errors')
        os.makedirs(outputbase+'/ok')
        for g in glob.glob(outputbase+'/err_*.txt'):
            is_ok = 'All tests passed' in open(g, encoding='utf-8', errors='ignore').read()
            if is_ok:
                shutil.move(g, outputbase+'/ok')
            else:
                shutil.move(g, outputbase+'/errors')
        shutil.make_archive(outputbase+'/errors','zip',outputbase+'/errors')
        shutil.make_archive(outputbase+'/ok','zip',outputbase+'/ok')
        
    def collect_tag_list(self, paths):
        tags = []
        for path in paths:
            for f in glob.glob(path+'/errors/*.txt') + glob.glob(path+'/ok/*.txt'):
                tags.append(os.path.split(f)[1].split('.txt')[0].replace('err_',''))
        return [t for t in set(tags)]
    
    def get_tag_info(self, *, tagpath):
        with open(tagpath, encoding='utf-8', errors='ignore') as fp:
            contents = fp.read()
        if 'All tests passed' in contents:
            failure_count = 0
        else:
            groups = re.search(r'assertions: (\d+) \| (\d+) passed \| (\d+) failed', contents)
            if groups is None:
                failure_count = -9999999 # crash!
                # raise ValueError('no match for test result')
            else:
                failure_count = groups.group(3)
        return {
            'failure_count': failure_count,
            'url': tagpath,
            'path': os.path.dirname(os.path.dirname(tagpath)),
            'tag': os.path.split(tagpath)[1].split('.')[0].replace('err_','')
        }
    
    def get_info(self, *, path):
        return [self.get_tag_info(tagpath=f) for f in glob.glob(path+'/errors/*.txt') + glob.glob(path+'/ok/*.txt')]
        
    def build_table(self, outputroot, *, sort_by, sort_kwargs):
        paths = [ f.path for f in os.scandir(outputroot) if f.is_dir() ]
        results = { path:self.get_info(path=path) for path in paths }
        o = []
        for tag in self.collect_tag_list(paths=paths):
            j = {'tag': tag}
            for path, vals in results.items():
                d = pandas.DataFrame(vals); d.set_index('tag', inplace=True)
                r = d.loc[tag]
                url = r['url']
                failure_count = r['failure_count']
                j[path] = f'<a href="{url}">{failure_count} failures</a>'
            o.append(j)
            
        def keyer(pair):
            if 'href' in pair.iloc[0]:
                return [int(s.split('>', 1)[-1].split('<')[0].replace(' failures', '')) for s in pair]
            else:
                return pair
            
        df = pandas.DataFrame(o).sort_values(by=sort_by, key=keyer, **sort_kwargs)
        df.to_csv('report.csv')
        df.to_html('report.html', index=False, escape=False)
        return

vb = VersionBuilder(tester_exe=r'bld/Debug/main.exe')

sources = ['docker/sources/' + f for f in ['10000-ce2a80', 'BETA', '1717766042_56c80cf4af3593aa1bd25e5043df4d6d603b69d6.zip']]

for source in sources:
    
    # Build the version of REFPROP to be tested
    dest = 'RPbuilds/' + source.replace('.zip', '').replace('docker/sources/','')
    vb.build_REFPROP(source=source, dest=dest)
    
    # And run the tests
    outputbase = 'RPoutputs/' + source.replace('.zip', '').replace('docker/sources/','')
    os.makedirs(outputbase, exist_ok=True)
    vb.run_tests(RPbuild=dest, tags=None, outputbase=outputbase)
    
# And collect all the results into a nice table
vb.build_table(outputroot='RPoutputs', sort_by=['RPoutputs/1717766042_56c80cf4af3593aa1bd25e5043df4d6d603b69d6', 'tag'], sort_kwargs=dict(ascending=[False, True]))