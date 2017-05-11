from xml.dom import minidom
import spacy
import sys
import itertools
import re
import string
import random
import math

random.seed(28111993)

file = '/Users/diego/Github/SemEval-2014-Task-4-ABSA/data/SemEval-2014/Laptop/train.xml'
tokenizer = spacy.load('en')
xmldoc = minidom.parse(file)
sentence_list = xmldoc.getElementsByTagName('sentence')

ASPECT_TAG = 'B-MISC'


def is_valid_char(char):
    return char in string.ascii_lowercase or char in string.ascii_uppercase or char.isdigit()


final_data = []

for sentence in sentence_list:
    text = sentence.getElementsByTagName('text')[0].childNodes[0].nodeValue.replace(u'\xa0', u' ')

    id = sentence.attributes['id'].value
    if is_valid_char(text[-1]):
        text += ' .'
    else:
        text = text[:-1] + ' ' + text[-1]

    aspects = []
    for aspect_list in sentence.getElementsByTagName('aspectTerms'):
        for aspect in aspect_list.getElementsByTagName('aspectTerm'):
            aspects.append((aspect.attributes['term'].value.replace(u'\xa0', u' '), int(aspect.attributes['from'].value),
                            int(aspect.attributes['to'].value)))

    # Sort by "from" ascending
    aspects = sorted(aspects, key=lambda x: x[1])
    tokens_temp = [str(t).replace(u'\xa0', u' ') for t in tokenizer(text)]
    tokens = []
    for token in tokens_temp:
        if len(token) <= 3:
            tokens.append(token)
        else:
            if '.' in token:
                point_pos = token.find('.')
                if point_pos == len(token)-1:
                    tokens.append(token[:-1])
                    tokens.append('.')
                elif token[point_pos-1].isdigit() and token[point_pos+1].isdigit():
                    tokens.append(token)
                else:
                    tokens.append(token[:point_pos])
                    tokens.append('.')
                    tokens.append(token[point_pos+1:])
            else:
                if not is_valid_char(token[0]):
                    tokens.append(token[0])
                    token = token[1:]
                if not is_valid_char(token[-1]):
                    tokens.append(token[:-1])
                    tokens.append(token[-1])
                else:
                    tokens.append(token)

    tags = ['O'] * len(tokens)

    for i in range(len(aspects)):
        aspect, fr, to = aspects[i]
        for aspect in [str(t) for t in tokenizer(text[fr:to])]:
            current_length = 0
            for j, token in enumerate(tokens):
                if current_length >= fr and current_length < to:
                    if token == aspect and tags[j] == 'O':
                        tags[j] = ASPECT_TAG
                        break

                current_length += len(token)
                if j + 1 < len(tokens):
                    current_length += len(text[current_length:text.find(tokens[j + 1], current_length)])

    if not (len([t for t in tags if t == ASPECT_TAG]) == len(list(itertools.chain.from_iterable([[str(t) for t in tokenizer(aspect[0])] for aspect in aspects])))):
        print(tokens)
        print(tags)
        print(aspects)
        # print('')
        # assert len(tokens) == len(tags)
        print([t for t in tags if t == ASPECT_TAG],list(itertools.chain.from_iterable([aspect[0].split(' ') for aspect in aspects])))
        print('')
    assert (len([t for t in tags if t == ASPECT_TAG]) == len(list(itertools.chain.from_iterable([[str(t) for t in tokenizer(aspect[0])] for aspect in aspects]))))

    final_data.append((tokens, tags, id))

data = final_data

if 'train' in file:
    random.shuffle(final_data)
    split = math.ceil(len(final_data)*0.8)

    train = final_data[:split]
    val = final_data[split:]
    data = train

    train_ids, val_ids = [x[2] for x in train], [x[2] for x in val]
    assert len(train_ids) == len(train) and len(val_ids) == len(val)

    # Create reference XML file
    for name, ids in {'train':train_ids, 'valid':val_ids}.items():
        filename = file[:file.rfind('/')+1] + 'ref_' + name + '.xml'
        with open(filename, 'w', encoding='utf-8') as fp:
            fp.write('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<sentences>\n')

            tokenizer = spacy.load('en')
            xmldoc = minidom.parse(file)
            sentence_list = xmldoc.getElementsByTagName('sentence')
            for id in ids:
                for sentence in sentence_list:
                    if sentence.attributes['id'].value == id:
                        fp.write('    ' + sentence.toxml() + '\n')
                        break

            fp.write('</sentences>\n')

while True:
    pattern = re.compile("^ +")
    with open(file.replace('xml', 'txt'), 'w', encoding='utf-8') as fp:
        for tokens, tags, _ in data:
            fp.write('-DOCSTART- -X- -X- O\n\n')
            for i in range(len(tokens)):
                if not pattern.match(tokens[i]):
                    fp.write(tokens[i] + ' IGNORED IGNORED ' + tags[i] + '\n')

            fp.write('\n')
    if 'train' not in file:
        break
    else:
        data = val
        file = file.replace('train', 'valid')
