from xml.dom import minidom
import spacy
import sys
import itertools
import re
import string
import random
import math
import sys


def replace(text):
    return text.replace('"', "&quot;").replace('( ', '(').replace(' )', ')')


if len(sys.argv) < 3:
    pred_file = '../output/Laptop_2017-05-11_11-49-12-834263/000_train.txt'
    ref_file = '../data/SemEval-2014/Scorer/ref_Laptop_train.xml'
else:
    pred_file = sys.argv[1]
    ref_file = sys.argv[2]

ASPECT_TAG = 'B-MISC'


def map_aspect_to_text(text, current_aspect, current_from, current_to):
    aspects_exploded = current_aspect.split(' ')
    offset = 0 if (current_from - 6) < 0 else (current_from - 6)
    current_from = text.find(aspects_exploded[0], offset)
    if current_from < 0:
        current_from = text.find(aspects_exploded[0])
    current_to = text.rfind(aspects_exploded[-1], current_from, current_from + len(current_aspect) + 4)
    if current_to < 0:
        current_to = text.find(aspects_exploded[-1])
    current_to += len(aspects_exploded[-1])
    if current_from == current_to and current_from != 0:
        current_from -= 1
    current_aspect = text[current_from:current_to]

    if not(current_from >= 0 and current_to > 0 and current_from < current_to):
        print("wrong mapping ! : " + text + ", " + current_aspect + " (" + str(current_from) + ',' + str(current_to) + ')')

    return current_aspect, current_from, current_to


preds = []
with open(pred_file, 'r', encoding='utf-8') as fp:
    sentence = []
    aspects = []
    for l in fp:
        l = l.rstrip()
        if l == '':
            preds.append((sentence, aspects))
            sentence, aspects = [], []
        else:
            token, _, fr, to, _, _, truth, tag = l.split(' ')

            # Just to check if the mapping (conversion) is correct from truth to truth !
            # Should give a F1-score of 1.0
            # tag = truth

            if tag == ASPECT_TAG:
                aspects.append((token, int(fr), int(to)))
            sentence.append(token)


with open(pred_file.replace('txt', 'xml'), 'w', encoding='utf-8') as fp:
    fp.write('<?xml version="1.0" encoding="UTF-8" standalone="yes"?>\n<sentences>\n')

    if len(preds) > 0:
        id_sent = 0
        xmldoc = minidom.parse(ref_file)
        sentence_list = xmldoc.getElementsByTagName('sentence')
        for sentence in sentence_list:
            text = sentence.getElementsByTagName('text')[0].childNodes[0].nodeValue
            id = sentence.attributes['id'].value

            fp.write('\t<sentence id="' + str(id) + '">\n')
            fp.write('\t\t<text>' + text + '</text>\n')

            aspects = preds[id_sent][1]
            if len(aspects) > 0:
                fp.write('\t\t<aspectTerms>\n')
                i = 0
                current_aspect, current_from, current_to = '', 0, 0
                while i < len(aspects):
                    asp, fr, to = aspects[i]
                    if current_aspect == '':
                        current_aspect, current_from, current_to = asp, fr, to
                    elif fr - 1 == current_to:
                        current_aspect += ' ' + asp
                        current_to = to
                    else:
                        current_aspect, current_from, current_to = map_aspect_to_text(text, current_aspect, current_from, current_to)
                        fp.write('\t\t\t<aspectTerm term="' + replace(current_aspect) + '" polarity="" from="' + str(current_from) + '" to="' + str(current_to) + '"/>\n')
                        current_aspect, current_from, current_to = asp, fr, to
                    i += 1

                if current_aspect != '':
                    current_aspect, current_from, current_to = map_aspect_to_text(text, current_aspect, current_from, current_to)
                    fp.write('\t\t\t<aspectTerm term="' + replace(current_aspect) + '" polarity="" from="' + str(current_from) + '" to="' + str(current_to) + '"/>\n')
                    current_aspect, current_from, current_to = '', 0, 0

                fp.write('\t\t</aspectTerms>\n')
            fp.write('\t</sentence>\n')

            id_sent += 1
    fp.write('</sentences>\n')
