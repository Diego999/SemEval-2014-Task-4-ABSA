import subprocess
import os
import random


def get_wordnet_supersense_tags(sentences, PATH_DIR):
    annotated_sentences = []

    filename_temp = '/tmp/' + str(random.random())
    with open(filename_temp, 'w', encoding='utf-8') as fp:
        for tokens in sentences:
                fp.write(' '.join(tokens) + '\n')

    perl_script = subprocess.Popen([PATH_DIR + 'sst', 'multitag-line', filename_temp, '0', '0', PATH_DIR + 'DATA/GAZ/gazlistall_minussemcor', PATH_DIR + 'MODELS/WSJPOSc_base_20', PATH_DIR + 'DATA/WSJPOSc.TAGSET', PATH_DIR + 'MODELS/SEM07_base_12', PATH_DIR + 'DATA/WNSS_07.TAGSET'], stdout=subprocess.PIPE, cwd=PATH_DIR)
    stdout = perl_script.communicate()[0].decode('utf-8').split('\n')[:-1]

    assert len(stdout) == len(sentences)

    os.remove(filename_temp)

    for idx in range(len(sentences)):
        ann = stdout[idx].split(' ')
        annotated_sentences.append([[token, 'adv/adj' if 'adj.' in wtag or 'adv.' in wtag else (wtag[2:] if wtag != '0' else wtag)] for token, pos, lemma, wtag in [ann[i:i + 4] for i in range(0, len(ann), 4)]]) # list of 4 elements for each token

    return annotated_sentences