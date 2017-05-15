from os import listdir
from os.path import isfile, join
import sys
import glob

no_final = len(sys.argv) > 1

path = 'output/'
types = ['Restaurant', 'Laptop']
patience = 0 if no_final else 10

dirs = [f for f in listdir(path) if not isfile(join(path, f))]

final_res = {}
for d in dirs:
    start_with_type = False
    type_dataset = ''
    for t in types:
        start_with_type = d.startswith(t)
        type_dataset = t
        if start_with_type:
            break
    if start_with_type:
        if type_dataset not in final_res:
            final_res[type_dataset] = {'train':[], 'valid':[], 'test':[]}

        path_dir = join(path, d)
        files = []
        for f in [f for f in listdir(path_dir) if isfile(join(path_dir, f))]:
            digits = f[0:3]
            try:
                digits = int(digits)
                files.append(f)
            except ValueError:
                pass

        if len(files) > 0:
            epoch = str(int(files[-1][0:3]) - patience).rjust(3, '0')
            for f in files:
                if f.startswith(epoch) and f.endswith('aspect_evaluation.txt'):
                    type_set = f.split('_')[1].split('.')[0]
                    with open(join(path_dir, f), 'r', encoding='utf-8') as fp:
                        lines = fp.readlines()
                        try:
                            prec, rec, f1 = lines[-1].rstrip().split(';')
                            prec, rec = [float(t.split(' ')[-1][:-1]) for t in [prec, rec]]
                            f1 = float(f1.split(':')[1].lstrip().split(' ')[0])
                            param = glob.glob(join(path_dir, 'model') + '/' + '*.ini')[0]
                            param = param[param.rfind('parameters') + len('parameters') + 1:]
                            if param == 'ini':
                                param = 'default'
                            param = param.ljust(75, ' ')
                            final_res[type_dataset][type_set].append((path_dir[path_dir.rfind('/')+1:].ljust(40, ' '), param, str(int(epoch)), str(prec), str(rec), str(f1)))
                        except:
                            pass

for d, t in sorted(final_res.items(), key=lambda x:x[0]):
    temp = []
    for i in range(len(final_res[d]['train'])):
        temp.append([final_res[d]['train'][i][0], final_res[d]['train'][i][2], final_res[d]['train'][i][-1], final_res[d]['valid'][i][-1], final_res[d]['test'][i][-1], final_res[d]['train'][i][1]])

    for t in sorted(temp, key=lambda x:float(x[-2]), reverse=True):
        print('\t\t'.join(t))


