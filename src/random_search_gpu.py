import random
import os


def draw_val(array):
    return array[random.randint(0, len(array)-1)]


file_exists = True

while file_exists:
    embedding_set = [('glove.6B.100d.txt', 100), ('santos.txt', 400), ('matteo.txt', 600)]
    token_pretrained_embedding_filepath, token_embedding_dimension = draw_val(embedding_set)

    type_review_set = ['Restaurant']#, 'Laptop']
    type_review = draw_val(type_review_set)

    char_emb_dim = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100]
    character_embedding_dimension = draw_val(char_emb_dim)

    char_hidden_dim = [10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 350, 375, 400]
    character_lstm_hidden_state_dimension = draw_val(char_hidden_dim)

    token_hidden_dim = [100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600]
    token_lstm_hidden_state_dimension = draw_val(token_hidden_dim)

    optimizer_set = ['sgd', 'adam']
    optimizer = draw_val(optimizer_set)

    learning_rate_set = [0.0001, 0.00025, 0.0005, 0.00075, 0.001, 0.0025, 0.005]
    learning_rate = draw_val(learning_rate_set)

    dropout_rate_set = [0.5, 0.6, 0.7]
    dropout_rate = draw_val(dropout_rate_set)

    parameters = [token_pretrained_embedding_filepath, token_embedding_dimension, type_review, character_embedding_dimension, character_lstm_hidden_state_dimension, token_lstm_hidden_state_dimension, learning_rate, dropout_rate]
    filename = 'parameters_' + '_'.join([str(x) for x in parameters]) + '.ini'
    file_exists = os.path.isfile(filename)

print(filename)

with open(filename, 'w', encoding='utf-8') as fp:
    fp.write("\n")
    fp.write("#----- Possible modes of operation -----------------------------------------------------------------------------------------------------------------#\n")
    fp.write("# training mode (from scratch): set continue_training to True, and use_pretrained_model to False (if training from scratch).                        #\n")
    fp.write("#				 				Must have train and valid sets in the dataset_text_folder, and test and deployment sets are optional.               #\n")
    fp.write("# training mode (from pretrained model): set continue_training to True, and use_pretrained_model to True (if training from a pretrained model).     #\n")
    fp.write("#				 						 Must have train and valid sets in the dataset_text_folder, and test and deployment sets are optional.      #\n")
    fp.write("# prediction mode (using pretrained model): set continue training to False, and use_pretrained_model to True.                                       #\n")
    fp.write("#											Must have either a test set or a deployment set.                                                        #\n")
    fp.write("# NOTE: Whenever use_pretrained_model is set to True, pretrained_model_folder must be set to the folder containing the pretrained model to use, and #\n")
    fp.write("# 		model.ckpt, dataset.pickle and parameters.ini must exist in the same folder as the checkpoint file.                                         #\n")
    fp.write("#---------------------------------------------------------------------------------------------------------------------------------------------------#\n")
    fp.write("\n")
    fp.write("[mode]\n")
    fp.write("# At least one of use_pretrained_model and train_model must be set to True.\n")
    fp.write("train_model = True\n")
    fp.write("use_pretrained_model = False\n")
    fp.write("pretrained_model_folder = ../trained_models/conll_2003_en\n")
    fp.write("\n")
    fp.write("[dataset]\n")
    fp.write("dataset_text_folder = ../data/SemEval-2014/" + type_review + "\n")
    fp.write("ref_file = ../data/SemEval-2014/Scorer/ref_" + type_review + "_train.xml\n")
    fp.write("eval_type = " + type_review + "\n")
    fp.write("scorer = ../data/SemEval-2014/Scorer/eval.jar\n")
    fp.write("#../data/SemEval-2014/Laptop\n")
    fp.write("#../data/conll2003/en\n")
    fp.write("\n")
    fp.write("# main_evaluation_mode should be either 'conll', 'bio', 'token', or 'binary'. ('conll' is entity-based)\n")
    fp.write("# It determines which metric to use for early stopping, displaying during training, and plotting F1-score vs. epoch.\n")
    fp.write("main_evaluation_mode = binary\n")
    fp.write("evaluate_aspect = True\n")
    fp.write("\n")
    fp.write("#---------------------------------------------------------------------------------------------------------------------#\n")
    fp.write("# The parameters below are for advanced users. Their default values should yield good performance in most cases.      #\n")
    fp.write("#---------------------------------------------------------------------------------------------------------------------#\n")
    fp.write("\n")
    fp.write("[ann]\n")
    fp.write("use_character_lstm = True\n")
    fp.write("character_embedding_dimension = " + str(character_embedding_dimension) + "\n")
    fp.write("character_lstm_hidden_state_dimension = " + str(character_lstm_hidden_state_dimension) + "\n")
    fp.write("\n")
    fp.write("# In order to use random initialization instead, set token_pretrained_embedding_filepath to empty string, as below:\n")
    fp.write("# token_pretrained_embedding_filepath =  \n")
    fp.write("token_pretrained_embedding_filepath = ../data/word_vectors/" + token_pretrained_embedding_filepath + "\n")
    fp.write("token_embedding_dimension = " + str(token_embedding_dimension) + "\n")
    fp.write("token_lstm_hidden_state_dimension = " + str(token_lstm_hidden_state_dimension) + "\n")
    fp.write("\n")
    fp.write("use_crf = True\n")
    fp.write("\n")
    fp.write("[training]\n")
    fp.write("patience = 10\n")
    fp.write("maximum_number_of_epochs = 100\n")
    fp.write("\n")
    fp.write("# optimizer should be either 'sgd', 'adam', or 'adadelta'\n")
    fp.write("optimizer = " + optimizer + "\n")
    fp.write("learning_rate = " + str(learning_rate) + "\n")
    fp.write("# gradients will be clipped above |gradient_clipping_value| and below -|gradient_clipping_value|, if gradient_clipping_value is non-zero \n")
    fp.write("# (set to 0 to disable gradient clipping)\n")
    fp.write("gradient_clipping_value = 5.0\n")
    fp.write("\n")
    fp.write("# dropout_rate should be between 0 and 1\n")
    fp.write("dropout_rate = " + str(dropout_rate) + "\n")
    fp.write("\n")
    fp.write("# Upper bound on the number of CPU threads NeuroNER will use \n")
    fp.write("number_of_cpu_threads = 4\n")
    fp.write("\n")
    fp.write("# Upper bound on the number of GPU NeuroNER will use \n")
    fp.write("# If number_of_gpus > 0, you need to have installed tensorflow-gpu\n")
    fp.write("number_of_gpus = 2\n")
    fp.write("\n")
    fp.write("[advanced]\n")
    fp.write("experiment_name = test\n")
    fp.write("\n")
    fp.write("# tagging_format should be either 'bioes' or 'bio'\n")
    fp.write("tagging_format = bioes\n")
    fp.write("\n")
    fp.write("# tokenizer should be either 'spacy' or 'stanford'. The tokenizer is only used when the original data is provided only in BRAT format.\n")
    fp.write("# - 'spacy' refers to spaCy (https://spacy.io). To install spacy: pip install -U spacy\n")
    fp.write("# - 'stanford' refers to Stanford CoreNLP (https://stanfordnlp.github.io/CoreNLP/). Stanford CoreNLP is written in Java: to use it one has to start a\n")
    fp.write("#              Stanford CoreNLP server, which can tokenize sentences given on the fly. Stanford CoreNLP is portable, which means that it can be run\n")
    fp.write("#              without any installation. \n")
    fp.write("#              To download Stanford CoreNLP: https://stanfordnlp.github.io/CoreNLP/download.html\n")
    fp.write("#              To run Stanford CoreNLP, execute in the terminal: `java -mx4g -cp \" * \" edu.stanford.nlp.pipeline.StanfordCoreNLPServer -port 9000 -timeout 50000`\n")
    fp.write("#              By default Stanford CoreNLP is in English. To use it in other languages, see: https://stanfordnlp.github.io/CoreNLP/human-languages.html\n")
    fp.write("#              Stanford CoreNLP 3.6.0 and higher requires Java 8. We have tested NeuroNER with Stanford CoreNLP 3.6.0. \n")
    fp.write("tokenizer = spacy\n")
    fp.write("# spacylanguage should be either 'de' (German), 'en' (English) or 'fr' (French). (https://spacy.io/docs/api/language-models)\n")
    fp.write("# To install the spaCy language: `python -m spacy.de.download`; or `python -m spacy.en.download`; or `python -m spacy.fr.download`\n")
    fp.write("spacylanguage = en\n")
    fp.write("\n")
    fp.write("# If remap_unknown_tokens is set to True, map to UNK any token that hasn't been seen in neither the training set nor the pre-trained token embeddings.\n")
    fp.write("remap_unknown_tokens_to_unk = True\n")
    fp.write("\n")
    fp.write("# If load_only_pretrained_token_embeddings is set to True, then token embeddings will only be loaded if it exists in token_pretrained_embedding_filepath \n")
    fp.write("# or in pretrained_model_checkpoint_filepath, even for the training set.\n")
    fp.write("load_only_pretrained_token_embeddings = False\n")
    fp.write("\n")
    fp.write("# If check_for_lowercase is set to True, the lowercased version of each token will also be checked when loading the pretrained embeddings.\n")
    fp.write("# For example, if the token 'Boston' does not exist in the pretrained embeddings, then it is mapped to the embedding of its lowercased version 'boston',\n")
    fp.write("# if it exists among the pretrained embeddings.\n")
    fp.write("check_for_lowercase = True\n")
    fp.write("\n")
    fp.write("# If check_for_digits_replaced_with_zeros is set to True, each token with digits replaced with zeros will also be checked when loading pretrained embeddings.\n")
    fp.write("# For example, if the token '123-456-7890' does not exist in the pretrained embeddings, then it is mapped to the embedding of '000-000-0000',\n")
    fp.write("# if it exists among the pretrained embeddings.\n")
    fp.write("# If both check_for_lowercase and check_for_digits_replaced_with_zeros are set to True, then the lowercased version is checked before the digit-zeroed version.\n")
    fp.write("check_for_digits_replaced_with_zeros = True\n")
    fp.write("\n")
    fp.write("# If freeze_token_embeddings is set to True, token embedding will remain frozen (not be trained).\n")
    fp.write("freeze_token_embeddings = False\n")
    fp.write("\n")
    fp.write("# If debug is set to True, only 200 lines will be loaded for each split of the dataset.\n")
    fp.write("debug = False\n")
    fp.write("verbose = False\n")
    fp.write("\n")
    fp.write("# plot_format specifies the format of the plots generated by NeuroNER. It should be either 'png' or 'pdf'.\n")
    fp.write("plot_format = pdf\n")
    fp.write("\n")
    fp.write("# specify which layers to reload from the pretrained model\n")
    fp.write("reload_character_embeddings = True\n")
    fp.write("reload_character_lstm = True\n")
    fp.write("reload_token_embeddings = True\n")
    fp.write("reload_token_lstm = True\n")
    fp.write("reload_feedforward = True\n")
    fp.write("reload_crf = True\n")