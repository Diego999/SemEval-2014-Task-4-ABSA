import sklearn.preprocessing
import utils
import collections
import codecs
import utils_nlp
import re
import time
import token
import os
import pickle
import spacy
import sys
import pickle
sys.path.append('..')
from sst_Ciaramita_and_Altun_2006.wordnet_utils import get_wordnet_supersense_tags


class Dataset(object):
    """A class for handling data sets."""

    def __init__(self, name='', verbose=False, debug=False):
        self.name = name
        self.verbose = verbose
        self.debug = debug

    # Hack to make defaultdict picklable
    def _def_dd(self):
        return 0

    def _parse_dataset(self, dataset_filepath, nlp):
        token_count = collections.defaultdict(self._def_dd)
        label_count = collections.defaultdict(self._def_dd)
        pos_count = collections.defaultdict(self._def_dd)
        wn_count = collections.defaultdict(self._def_dd)
        ner_count = collections.defaultdict(self._def_dd)
        character_count = collections.defaultdict(self._def_dd)

        line_count = -1
        tokens = []
        labels = []
        characters = []
        token_lengths = []
        new_token_sequence = []
        new_label_sequence = []
        if dataset_filepath:
            f = codecs.open(dataset_filepath, 'r', 'UTF-8')
            for line in f:
                line_count += 1
                line = line.strip().split(' ')
                if len(line) == 0 or len(line[0]) == 0 or '-DOCSTART-' in line[0]:
                    if len(new_token_sequence) > 0:
                        labels.append(new_label_sequence)
                        tokens.append(new_token_sequence)
                        new_token_sequence = []
                        new_label_sequence = []
                    continue
                token = str(line[0])
                label = str(line[-1])
                token_count[token] += 1
                label_count[label] += 1

                new_token_sequence.append(token)
                new_label_sequence.append(label)

                for character in token:
                    character_count[character] += 1

                if self.debug and line_count > 200: break# for debugging purposes

            if len(new_token_sequence) > 0:
                labels.append(new_label_sequence)
                tokens.append(new_token_sequence)

        # POS and NER
        poss = []
        ners = []
        if dataset_filepath:
            filename_pos_ner = dataset_filepath + '_pos_ner.pickle'
            if os.path.isfile(filename_pos_ner):
                with open(filename_pos_ner, 'rb') as fp:
                    poss, ners, pos_count, ner_count = pickle.load(fp)
            else:
                for sentence in tokens:
                    parsed_sentence = nlp.tokenizer.tokens_from_list(sentence)
                    nlp.tagger(parsed_sentence)
                    nlp.entity(parsed_sentence)

                    new_pos_sequence = []
                    new_ner_sequence = []
                    for token in parsed_sentence:
                        new_pos_sequence.append(token.pos_)
                        new_ner_sequence.append(token.ent_type_)
                        pos_count[token.pos_] += 1
                        ner_count[token.ent_type_] += 1
                    poss.append(new_pos_sequence)
                    ners.append(new_ner_sequence)

                with open(filename_pos_ner, 'wb') as fp:
                    data = [poss, ners, pos_count, ner_count]
                    pickle.dump(data, fp)

        # WordNet Hypernym
        wns = []
        if dataset_filepath:
            filename_wn = dataset_filepath + '_wn.pickle'
            if os.path.isfile(filename_wn):
                with open(filename_wn, 'rb') as fp:
                    wns, wn_count = pickle.load(fp)
            else:
                wns_temp = get_wordnet_supersense_tags(tokens, '../sst_Ciaramita_and_Altun_2006/')

                assert len(tokens) == len(wns_temp)


                for i in range(len(wns_temp)):
                    new_wn_sequence = ['0'] * len(tokens[i])

                    # Loop over wordnet tags
                    for j in range(0, len(wns_temp[i])):
                        token_wn, wn = wns_temp[i][j]

                        # Loop over tokens
                        for k in range(0, len(wns_temp[i][j])):
                            token_init = tokens[i][k]

                            if token_init == token_wn:
                                new_wn_sequence[k] = wn
                                wn_count[wn] += 1

                    wns.append(new_wn_sequence)

                with open(filename_wn, 'wb') as fp:
                    data = [wns, wn_count]
                    pickle.dump(data, fp)

        return labels, tokens, poss, ners, wns, token_count, label_count, character_count, pos_count, ner_count, wn_count

    def load_dataset(self, dataset_filepaths, parameters):
        '''
        dataset_filepaths : dictionary with keys 'train', 'valid', 'test', 'deploy'
        '''
        start_time = time.time()
        print('Load dataset... ', end='', flush=True)
        nlp = spacy.load('en')
        all_pretrained_tokens = []
        if parameters['token_pretrained_embedding_filepath'] != '':
            all_pretrained_tokens = utils_nlp.load_tokens_from_pretrained_token_embeddings(parameters)
        if self.verbose: print("len(all_pretrained_tokens): {0}".format(len(all_pretrained_tokens)))

        # Load pretraining dataset to ensure that index to label is compatible to the pretrained model,
        #   and that token embeddings that are learned in the pretrained model are loaded properly.
        all_tokens_in_pretraining_dataset = []
        if parameters['use_pretrained_model']:
            pretraining_dataset = pickle.load(open(os.path.join(parameters['pretrained_model_folder'], 'dataset.pickle'), 'rb'))
            all_tokens_in_pretraining_dataset = pretraining_dataset.index_to_token.values()

        remap_to_unk_count_threshold = 1
        self.UNK_TOKEN_INDEX = 0
        if parameters['use_pos']:
            self.UNK_POS_INDEX = 0
        if parameters['use_ner']:
            self.UNK_NER_INDEX = 0
        if parameters['use_wn']:
            self.UNK_WN_INDEX = 0
        self.PADDING_CHARACTER_INDEX = 0
        self.tokens_mapped_to_unk = []
        self.UNK = 'UNK'
        self.unique_labels = []
        labels = {}
        tokens = {}
        poss = {}
        ners = {}
        wns = {}
        characters = {}
        token_lengths = {}
        label_count = {}
        token_count = {}
        if parameters['use_pos']:
            pos_count = {}
            pos_lengths = {}
        if parameters['use_ner']:
            ner_count = {}
            ner_lengths = {}
        if parameters['use_wn']:
            wn_count = {}
            wn_lengths = {}
        character_count = {}
        for dataset_type in ['train', 'valid', 'test', 'deploy']:
            labels[dataset_type], tokens[dataset_type], poss[dataset_type], ners[dataset_type], wns[dataset_type], token_count[dataset_type], label_count[dataset_type], character_count[dataset_type], pos_count[dataset_type], ner_count[dataset_type], wn_count[dataset_type] \
                = self._parse_dataset(dataset_filepaths.get(dataset_type, None), nlp)

            if self.verbose: print("dataset_type: {0}".format(dataset_type))
            if self.verbose: print("len(token_count[dataset_type]): {0}".format(len(token_count[dataset_type])))

        token_count['all'] = {}
        for token in list(token_count['train'].keys()) + list(token_count['valid'].keys()) + list(token_count['test'].keys()) + list(token_count['deploy'].keys()):
            token_count['all'][token] = token_count['train'][token] + token_count['valid'][token] + token_count['test'][token] + token_count['deploy'][token]

        for dataset_type in dataset_filepaths.keys():
            if self.verbose: print("dataset_type: {0}".format(dataset_type))
            if self.verbose: print("len(token_count[dataset_type]): {0}".format(len(token_count[dataset_type])))

        if parameters['use_pos']:
            pos_count['all'] = {}
            for pos in list(pos_count['train'].keys()) + list(pos_count['valid'].keys()) + list(pos_count['test'].keys()) + list(pos_count['deploy'].keys()):
                pos_count['all'][pos] = pos_count['train'][pos] + pos_count['valid'][pos] + pos_count['test'][pos] + pos_count['deploy'][pos]

        if parameters['use_ner']:
            ner_count['all'] = {}
            for ner in list(ner_count['train'].keys()) + list(ner_count['valid'].keys()) + list(ner_count['test'].keys()) + list(ner_count['deploy'].keys()):
                ner_count['all'][ner] = ner_count['train'][ner] + ner_count['valid'][ner] + ner_count['test'][ner] + ner_count['deploy'][ner]

        if parameters['use_wn']:
            wn_count['all'] = {}
            for wn in list(wn_count['train'].keys()) + list(wn_count['valid'].keys()) + list(wn_count['test'].keys()) + list(wn_count['deploy'].keys()):
                wn_count['all'][wn] = wn_count['train'][wn] + wn_count['valid'][wn] + wn_count['test'][wn] + wn_count['deploy'][wn]

        character_count['all'] = {}
        for character in list(character_count['train'].keys()) + list(character_count['valid'].keys()) + list(character_count['test'].keys()) + list(character_count['deploy'].keys()):
            character_count['all'][character] = character_count['train'][character] + character_count['valid'][character] + character_count['test'][character] + character_count['deploy'][character]

        label_count['all'] = {}
        for character in list(label_count['train'].keys()) + list(label_count['valid'].keys()) + list(label_count['test'].keys()) + list(label_count['deploy'].keys()):
            label_count['all'][character] = label_count['train'][character] + label_count['valid'][character] + label_count['test'][character] + label_count['deploy'][character]

        token_count['all'] = utils.order_dictionary(token_count['all'], 'value_key', reverse = True)
        if parameters['use_pos']:
            pos_count['all'] = utils.order_dictionary(pos_count['all'], 'value_key', reverse = True)
        if parameters['use_ner']:
            ner_count['all'] = utils.order_dictionary(ner_count['all'], 'value_key', reverse = True)
        if parameters['use_wn']:
            wn_count['all'] = utils.order_dictionary(wn_count['all'], 'value_key', reverse = True)
        label_count['all'] = utils.order_dictionary(label_count['all'], 'key', reverse = False)
        character_count['all'] = utils.order_dictionary(character_count['all'], 'value', reverse = True)
        if self.verbose: print('character_count[\'all\']: {0}'.format(character_count['all']))

        # TOKEN
        token_to_index = {}
        token_to_index[self.UNK] = self.UNK_TOKEN_INDEX
        iteration_number = 0
        number_of_unknown_tokens = 0
        if self.verbose: print("parameters['remap_unknown_tokens_to_unk']: {0}".format(parameters['remap_unknown_tokens_to_unk']))
        if self.verbose: print("len(token_count['train'].keys()): {0}".format(len(token_count['train'].keys())))
        for token, count in token_count['all'].items():
            if iteration_number == self.UNK_TOKEN_INDEX: iteration_number += 1

            if parameters['remap_unknown_tokens_to_unk'] == 1 and \
                (token_count['train'][token] == 0 or \
                parameters['load_only_pretrained_token_embeddings']) and \
                not utils_nlp.is_token_in_pretrained_embeddings(token, all_pretrained_tokens, parameters) and \
                token not in all_tokens_in_pretraining_dataset:
                if self.verbose: print("token: {0}".format(token))
                if self.verbose: print("token.lower(): {0}".format(token.lower()))
                if self.verbose: print("re.sub('\d', '0', token.lower()): {0}".format(re.sub('\d', '0', token.lower())))
                token_to_index[token] =  self.UNK_TOKEN_INDEX
                number_of_unknown_tokens += 1
                self.tokens_mapped_to_unk.append(token)
            else:
                token_to_index[token] = iteration_number
                iteration_number += 1
        if self.verbose: print("number_of_unknown_tokens: {0}".format(number_of_unknown_tokens))

        infrequent_token_indices = []
        for token, count in token_count['train'].items():
            if 0 < count <= remap_to_unk_count_threshold:
                infrequent_token_indices.append(token_to_index[token])
        if self.verbose: print("len(token_count['train']): {0}".format(len(token_count['train'])))
        if self.verbose: print("len(infrequent_token_indices): {0}".format(len(infrequent_token_indices)))

        # POS
        if parameters['use_pos']:
            pos_to_index = {}
            pos_to_index[self.UNK] = self.UNK_POS_INDEX
            iteration_number = 0
            number_of_unknown_pos = 0
            if self.verbose: print("parameters['remap_unknown_pos_to_unk']: {0}".format(parameters['remap_unknown_pos_to_unk']))
            if self.verbose: print("len(pos_count['train'].keys()): {0}".format(len(pos_count['train'].keys())))
            for pos, count in pos_count['all'].items():
                if iteration_number == self.UNK_POS_INDEX: iteration_number += 1
                pos_to_index[pos] = iteration_number
                iteration_number += 1
            if self.verbose: print("number_of_unknown_pos: {0}".format(number_of_unknown_pos))

            infrequent_pos_indices = []
            for pos, count in pos_count['train'].items():
                if 0 < count <= remap_to_unk_count_threshold:
                    infrequent_pos_indices.append(pos_to_index[pos])
            if self.verbose: print("len(pos_count['train']): {0}".format(len(pos_count['train'])))
            if self.verbose: print("len(infrequent_pos_indices): {0}".format(len(infrequent_pos_indices)))

        # NER
        if parameters['use_ner']:
            ner_to_index = {}
            ner_to_index[self.UNK] = self.UNK_NER_INDEX
            iteration_number = 0
            number_of_unknown_ner = 0
            if self.verbose: print("parameters['remap_unknown_ner_to_unk']: {0}".format(parameters['remap_unknown_ner_to_unk']))
            if self.verbose: print("len(ner_count['train'].keys()): {0}".format(len(ner_count['train'].keys())))
            for ner, count in ner_count['all'].items():
                if iteration_number == self.UNK_NER_INDEX: iteration_number += 1
                ner_to_index[ner] = iteration_number
                iteration_number += 1
            if self.verbose: print("number_of_unknown_ner: {0}".format(number_of_unknown_ner))

            infrequent_ner_indices = []
            for ner, count in ner_count['train'].items():
                if 0 < count <= remap_to_unk_count_threshold:
                    infrequent_ner_indices.append(ner_to_index[ner])
            if self.verbose: print("len(ner_count['train']): {0}".format(len(ner_count['train'])))
            if self.verbose: print("len(infrequent_ner_indices): {0}".format(len(infrequent_ner_indices)))

        # WN
        if parameters['use_wn']:
            wn_to_index = {}
            wn_to_index[self.UNK] = self.UNK_WN_INDEX
            iteration_number = 0
            number_of_unknown_wn = 0
            if self.verbose: print("parameters['remap_unknown_wn_to_unk']: {0}".format(parameters['remap_unknown_wn_to_unk']))
            if self.verbose: print("len(wn_count['train'].keys()): {0}".format(len(wn_count['train'].keys())))
            for wn, count in wn_count['all'].items():
                if iteration_number == self.UNK_WN_INDEX: iteration_number += 1
                wn_to_index[wn] = iteration_number
                iteration_number += 1
            if self.verbose: print("number_of_unknown_wn: {0}".format(number_of_unknown_wn))

            infrequent_wn_indices = []
            for wn, count in wn_count['train'].items():
                if 0 < count <= remap_to_unk_count_threshold:
                    infrequent_wn_indices.append(wn_to_index[wn])
            if self.verbose: print("len(wn_count['train']): {0}".format(len(wn_count['train'])))
            if self.verbose: print("len(infrequent_wn_indices): {0}".format(len(infrequent_wn_indices)))

        # Ensure that both B- and I- versions exist for each label
        labels_without_bio = set()
        for label in label_count['all'].keys():
            new_label = utils_nlp.remove_bio_from_label_name(label)
            labels_without_bio.add(new_label)
        for label in labels_without_bio:
            if label == 'O':
                continue
            if parameters['tagging_format'] == 'bioes':
                prefixes = ['B-', 'I-', 'E-', 'S-']
            else:
                prefixes = ['B-', 'I-']
            for prefix in prefixes:
                l = prefix + label
                if l not in label_count['all']:
                    label_count['all'][l] = 0
        label_count['all'] = utils.order_dictionary(label_count['all'], 'key', reverse = False)

        if parameters['use_pretrained_model']:
            self.unique_labels = sorted(list(pretraining_dataset.label_to_index.keys()))
            # Make sure labels are compatible with the pretraining dataset.
            for label in label_count['all']:
                if label not in pretraining_dataset.label_to_index:
                    raise AssertionError("The label {0} does not exist in the pretraining dataset. ".format(label) +
                                         "Please ensure that only the following labels exist in the dataset: {0}".format(', '.join(self.unique_labels)))
            label_to_index = pretraining_dataset.label_to_index.copy()
        else:
            label_to_index = {}
            iteration_number = 0
            for label, count in label_count['all'].items():
                label_to_index[label] = iteration_number
                iteration_number += 1
                self.unique_labels.append(label)

        if self.verbose: print('self.unique_labels: {0}'.format(self.unique_labels))

        character_to_index = {}
        iteration_number = 0
        for character, count in character_count['all'].items():
            if iteration_number == self.PADDING_CHARACTER_INDEX: iteration_number += 1
            character_to_index[character] = iteration_number
            iteration_number += 1

        # TOKEN
        if self.verbose: print('token_count[\'train\'][0:10]: {0}'.format(list(token_count['train'].items())[0:10]))
        token_to_index = utils.order_dictionary(token_to_index, 'value', reverse = False)
        if self.verbose: print('token_to_index: {0}'.format(token_to_index))
        index_to_token = utils.reverse_dictionary(token_to_index)
        if parameters['remap_unknown_tokens_to_unk'] == 1: index_to_token[self.UNK_TOKEN_INDEX] = self.UNK
        if self.verbose: print('index_to_token: {0}'.format(index_to_token))

        # POS
        if parameters['use_pos']:
            if self.verbose: print('pos_count[\'train\'][0:10]: {0}'.format(list(pos_count['train'].items())[0:10]))
            pos_to_index = utils.order_dictionary(pos_to_index, 'value', reverse = False)
            if self.verbose: print('pos_to_index: {0}'.format(pos_to_index))
            index_to_pos = utils.reverse_dictionary(pos_to_index)
            if parameters['remap_unknown_pos_to_unk'] == 1: index_to_pos[self.UNK_POS_INDEX] = self.UNK
            if self.verbose: print('index_to_pos: {0}'.format(index_to_pos))

        # NER
        if parameters['use_ner']:
            if self.verbose: print('ner_count[\'train\'][0:10]: {0}'.format(list(ner_count['train'].items())[0:10]))
            ner_to_index = utils.order_dictionary(ner_to_index, 'value', reverse = False)
            if self.verbose: print('ner_to_index: {0}'.format(ner_to_index))
            index_to_ner = utils.reverse_dictionary(ner_to_index)
            if parameters['remap_unknown_ner_to_unk'] == 1: index_to_ner[self.UNK_NER_INDEX] = self.UNK
            if self.verbose: print('index_to_ner: {0}'.format(index_to_ner))

        # WN
        if parameters['use_wn']:
            if self.verbose: print('wn_count[\'train\'][0:10]: {0}'.format(list(wn_count['train'].items())[0:10]))
            wn_to_index = utils.order_dictionary(wn_to_index, 'value', reverse = False)
            if self.verbose: print('wn_to_index: {0}'.format(wn_to_index))
            index_to_wn = utils.reverse_dictionary(wn_to_index)
            if parameters['remap_unknown_wn_to_unk'] == 1: index_to_wn[self.UNK_WN_INDEX] = self.UNK
            if self.verbose: print('index_to_wn: {0}'.format(index_to_wn))

        if self.verbose: print('label_count[\'train\']: {0}'.format(label_count['train']))
        label_to_index = utils.order_dictionary(label_to_index, 'value', reverse = False)
        if self.verbose: print('label_to_index: {0}'.format(label_to_index))
        index_to_label = utils.reverse_dictionary(label_to_index)
        if self.verbose: print('index_to_label: {0}'.format(index_to_label))

        character_to_index = utils.order_dictionary(character_to_index, 'value', reverse = False)
        index_to_character = utils.reverse_dictionary(character_to_index)
        if self.verbose: print('character_to_index: {0}'.format(character_to_index))
        if self.verbose: print('index_to_character: {0}'.format(index_to_character))


        if self.verbose: print('labels[\'train\'][0:10]: {0}'.format(labels['train'][0:10]))
        if self.verbose: print('tokens[\'train\'][0:10]: {0}'.format(tokens['train'][0:10]))

        if self.verbose:
            # Print sequences of length 1 in train set
            for token_sequence, label_sequence in zip(tokens['train'], labels['train']):
                if len(label_sequence) == 1 and label_sequence[0] != 'O':
                    print("{0}\t{1}".format(token_sequence[0], label_sequence[0]))

        # Map tokens and labels to their indices
        token_indices = {}
        if parameters['use_pos']:
            pos_indices = {}
        if parameters['use_ner']:
            ner_indices = {}
        if parameters['use_wn']:
            wn_indices = {}
        label_indices = {}
        character_indices = {}
        character_indices_padded = {}
        for dataset_type in dataset_filepaths.keys():
            token_indices[dataset_type] = []
            if parameters['use_pos']:
                pos_indices[dataset_type] = []
                pos_lengths[dataset_type] = []
            if parameters['use_ner']:
                ner_indices[dataset_type] = []
                ner_lengths[dataset_type] = []
            if parameters['use_wn']:
                wn_indices[dataset_type] = []
                wn_lengths[dataset_type] = []
            characters[dataset_type] = []
            character_indices[dataset_type] = []
            token_lengths[dataset_type] = []
            character_indices_padded[dataset_type] = []

            # Tokens and Chars
            for token_sequence in tokens[dataset_type]:
                token_indices[dataset_type].append([token_to_index[token] for token in token_sequence])
                characters[dataset_type].append([list(token) for token in token_sequence])
                character_indices[dataset_type].append([[character_to_index[character] for character in token] for token in token_sequence])
                token_lengths[dataset_type].append([len(token) for token in token_sequence])

                longest_token_length_in_sequence = max(token_lengths[dataset_type][-1])
                character_indices_padded[dataset_type].append([ utils.pad_list(temp_token_indices, longest_token_length_in_sequence, self.PADDING_CHARACTER_INDEX)
                                                                for temp_token_indices in character_indices[dataset_type][-1]])

            # POS
            if parameters['use_pos']:
                for pos_sequence in poss[dataset_type]:
                    pos_indices[dataset_type].append([pos_to_index[pos] for pos in pos_sequence])
                    pos_lengths[dataset_type].append([len(pos) for pos in pos_sequence])

            # NER
            if parameters['use_ner']:
                for ner_sequence in ners[dataset_type]:
                    ner_indices[dataset_type].append([ner_to_index[ner] for ner in ner_sequence])
                    ner_lengths[dataset_type].append([len(ner) for ner in ner_sequence])

            # WN
            if parameters['use_wn']:
                for wn_sequence in wns[dataset_type]:
                    wn_indices[dataset_type].append([wn_to_index[wn] for wn in wn_sequence])
                    wn_lengths[dataset_type].append([len(wn) for wn in wn_sequence])

            label_indices[dataset_type] = []
            for label_sequence in labels[dataset_type]:
                label_indices[dataset_type].append([label_to_index[label] for label in label_sequence])

        if self.verbose: print('token_lengths[\'train\'][0][0:10]: {0}'.format(token_lengths['train'][0][0:10]))
        if self.verbose: print('characters[\'train\'][0][0:10]: {0}'.format(characters['train'][0][0:10]))
        if self.verbose: print('token_indices[\'train\'][0:10]: {0}'.format(token_indices['train'][0:10]))
        if self.verbose: print('label_indices[\'train\'][0:10]: {0}'.format(label_indices['train'][0:10]))
        if self.verbose: print('character_indices[\'train\'][0][0:10]: {0}'.format(character_indices['train'][0][0:10]))
        if self.verbose: print('character_indices_padded[\'train\'][0][0:10]: {0}'.format(character_indices_padded['train'][0][0:10]))

        # Vectorize the labels
        # [Numpy 1-hot array](http://stackoverflow.com/a/42263603/395857)
        label_binarizer = sklearn.preprocessing.LabelBinarizer()
        label_binarizer.fit(range(max(index_to_label.keys())+1))
        label_vector_indices = {}
        for dataset_type in dataset_filepaths.keys():
            label_vector_indices[dataset_type] = []
            for label_indices_sequence in label_indices[dataset_type]:
                label_vector_indices[dataset_type].append(label_binarizer.transform(label_indices_sequence))

        if self.verbose: print('label_vector_indices[\'train\'][0:2]: {0}'.format(label_vector_indices['train'][0:2]))

        if self.verbose: print('len(label_vector_indices[\'train\']): {0}'.format(len(label_vector_indices['train'])))
        self.token_to_index = token_to_index
        self.index_to_token = index_to_token
        self.token_indices = token_indices
        self.label_indices = label_indices
        self.character_indices_padded = character_indices_padded
        self.index_to_character = index_to_character
        self.character_to_index = character_to_index
        self.character_indices = character_indices
        self.token_lengths = token_lengths
        self.characters = characters
        self.tokens = tokens
        self.labels = labels
        self.label_vector_indices = label_vector_indices
        self.index_to_label = index_to_label
        self.label_to_index = label_to_index

        # POS
        if parameters['use_pos']:
            self.pos_to_index = pos_to_index
            self.index_to_pos = index_to_pos
            self.pos_indices = pos_indices
            self.pos_lengths = pos_lengths
            self.poss = poss

        # NER
        if parameters['use_ner']:
            self.ner_to_index = ner_to_index
            self.index_to_ner = index_to_ner
            self.ner_indices = ner_indices
            self.ner_lengths = ner_lengths
            self.ners = ners

        # WN
        if parameters['use_wn']:
            self.wn_to_index = wn_to_index
            self.index_to_wn = index_to_wn
            self.wn_indices = wn_indices
            self.wn_lengths = wn_lengths
            self.wns = wns

        if self.verbose: print("len(self.token_to_index): {0}".format(len(self.token_to_index)))
        if self.verbose: print("len(self.index_to_token): {0}".format(len(self.index_to_token)))

        self.number_of_classes = max(self.index_to_label.keys()) + 1
        self.vocabulary_size = max(self.index_to_token.keys()) + 1
        # POS
        if parameters['use_pos']:
            self.pos_size = max(self.index_to_pos.keys()) + 1
        # NER
        if parameters['use_ner']:
            self.ner_size = max(self.index_to_ner.keys()) + 1
        # WN
        if parameters['use_wn']:
            self.wn_size = max(self.index_to_wn.keys()) + 1

        self.alphabet_size = max(self.index_to_character.keys()) + 1
        if self.verbose: print("self.number_of_classes: {0}".format(self.number_of_classes))
        if self.verbose: print("self.alphabet_size: {0}".format(self.alphabet_size))
        if self.verbose: print("self.vocabulary_size: {0}".format(self.vocabulary_size))

        # unique_labels_of_interest is used to compute F1-scores.
        self.unique_labels_of_interest = list(self.unique_labels)
        self.unique_labels_of_interest.remove('O')

        self.unique_label_indices_of_interest = []
        for lab in self.unique_labels_of_interest:
            self.unique_label_indices_of_interest.append(label_to_index[lab])

        self.infrequent_token_indices = infrequent_token_indices
        # POS
        if parameters['use_pos']:
            self.infrequent_pos_indices = infrequent_pos_indices
        # NER
        if parameters['use_ner']:
            self.infrequent_ner_indices = infrequent_ner_indices
        # WN
        if parameters['use_wn']:
            self.infrequent_wn_indices = infrequent_wn_indices
        if self.verbose: print('self.unique_labels_of_interest: {0}'.format(self.unique_labels_of_interest))
        if self.verbose: print('self.unique_label_indices_of_interest: {0}'.format(self.unique_label_indices_of_interest))

        elapsed_time = time.time() - start_time
        print('done ({0:.2f} seconds)'.format(elapsed_time))

