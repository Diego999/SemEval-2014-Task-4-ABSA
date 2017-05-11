from os import listdir
from os.path import isfile, join

path = 'output/'
types = ['Restaurant', 'Laptop']
patience = 10

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

        epoch = str(int(files[-1][0:3]) - patience).rjust(3, '0')
        for f in files:
            if f.startswith(epoch) and f.endswith('aspect_evaluation.txt'):
                type_set = f.split('_')[1].split('.')[0]
                with open(join(path_dir, f), 'r', encoding='utf-8') as fp:
                    lines = fp.readlines()
                    prec, rec, f1 = lines[-1].rstrip().split(';')
                    prec, rec = [float(t.split(' ')[-1][:-1]) for t in [prec, rec]]
                    f1 = float(f1.split(':')[1].lstrip().split(' ')[0])
                    final_res[type_dataset][type_set].append((str(int(epoch)), str(prec), str(rec), str(f1)))

for d, t in final_res.items():
    for i in range(len(final_res[d]['train'])):
        print('\t'.join([d, final_res[d]['train'][i][0], final_res[d]['train'][i][-1], final_res[d]['valid'][i][-1], final_res[d]['test'][i][-1]]))


