import zipfile, glob, os, re

import run_one
import pandas

class ClassRunner():
    def __init__(self, *, paths, test):
        self.paths = paths
        self.test = test

    def _build_one(self, path):
        print(f'Running {path} w/ hash of: ', run_one.get_path_hash(path))
        run_one.run_test(root=path, test=self.test)

    def build(self):
        for p in self.paths:
            self._build_one(p)

    def _expand_DLL(self):
        for path in self.paths:
            h = run_one.get_path_hash(path)
            test = self.test
            with zipfile.ZipFile(f'{h}_{test}.zip') as z:
                z.extractall(path=f'{h}')

    def _collect_tag_list(self):
        tags = []
        for path in self.paths:
            h = run_one.get_path_hash(path)
            for f in glob.glob(h+'/errors/*.txt') + glob.glob(h+'/ok/*.txt'):
                tags.append(os.path.split(f)[1].split('.txt')[0].replace('err_',''))
        return [t for t in set(tags)] 

    def _get_tag_info(self, *, tagpath, pathhash):
        contents = open(tagpath).read()
        if 'All tests passed' in contents:
            failure_count = 0
        else:
            groups = re.search(r'assertions: (\d+) \| (\d+) passed \| (\d+) failed', contents)
            if groups is None:
                raise ValueError('no match for test result')
            failure_count = groups.group(3)
        return {
            'failure_count': failure_count,
            'url': tagpath,
            'pathhash': pathhash
        }

    def build_tag_report(self, tags):
        o = []
        unhash = {}
        for tag in tags:
            results = []
            for path in self.paths:
                pathhash = run_one.get_path_hash(path)
                unhash[pathhash] = path 
                candidates = [os.path.join(pathhash,'errors',f'err_{tag}.txt'), os.path.join(pathhash,'ok',f'err_{tag}.txt')]
                tagpath = [p for p in candidates if os.path.exists(p)][0]
                results.append(self._get_tag_info(tagpath=tagpath, pathhash=pathhash))
            if not all([r['failure_count'] == 0 for r in results]):
                # print(tag, results)
                j = {'tag': tag}
                for r in results:
                    url = r['url']
                    failure_count = r['failure_count']
                    j[unhash[r['pathhash']]] = f'<a href="{url}">{failure_count} failures</a>'
                o.append(j)
        pandas.DataFrame(o).to_html('report.html', index=False, escape=False)

    def compare(self):
        self._expand_DLL()
        self.tags = self._collect_tag_list()
        self.build_tag_report(self.tags)

if __name__ == '__main__':
    print('Can\'t run this file directly')
    quit()

    """ Example:
    from regression_tester import *
    paths = ['path1', 'path2']
    cr = ClassRunner(paths=paths, test='test')
    cr.build()
    cr.compare()
    """