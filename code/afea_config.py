PROBLEM_TYPE = 'TSP'
REQUEST_GENERATE_INTERVAL = 0.2
N_EPISODE = 300
AGENT_TYPE = 'DQN'
REQUEST_QUEUE_SIZE = 512
BATCH_SIZE = 1
EPSILON = 0.3
EPSILON_DECAY_RATE = 0.99
DB_SIZE = 1024
THRESHOLD = 0.6
K_NEAREST_SOLUTIONS = 3
EA_DATA_PATH = "../data/sing_GA_data.dat"
EA_PATH = "./basic_algorithm/GA_permu"
CODE_LENGTH = 64

Config = {
	'AutoEncoder':{
		'batch_size':128,
		'code_length':CODE_LENGTH,
		'data_source':r'..\data\TSP.dat',
		'data_size':20000,
		'max_cites':128,
		'num_of_clusters':8,
		'lr':1e-3,
		'pre_train_epoch':200,
		'clustering_train_epoch':60,
		'problem_type': PROBLEM_TYPE
	},
	'AFEA':{
		'retrain_autoencoder':False,
		'AE_MODEL_PATH':'../model/AE1625742073.4378543.pkl',
		'code_length':CODE_LENGTH,
		'RL_LOAD_HISTORY':False,
		'RL_MODEL_PATH':'',
		'MEMORY_SIZE':512,
		'replay_size': 256,
		'lr':1e-3,
		'W_reconstruction': 5,
		'W_clustering': 5,
		'gamma': 0.99,
	}



}