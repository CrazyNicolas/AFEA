PROBLEM_TYPE = 'TSP'
REQUEST_GENERATE_INTERVAL = 2
AGENT_TYPE = 'DQN'
REQUEST_QUEUE_SIZE = 512
BATCH_SIZE = 32
EPSILON = 0.3
EPSILON_DECAY_RATE = 0.99
DB_SIZE = 1024
THRESHOLD = 0.8
K_NEAREST_SOLUTIONS = 1
Config = {
	'AutoEncoder':{
		'batch_size':32,
		'code_length':32,
		'data_source':r'..\data\TSP.dat',
		'data_size':60000,
		'max_cites':128,
		'num_of_clusters':8,
		'lr':1e-3,
		'pre_train_epoch':10,
		'clustering_train_epoch':6,
		'problem_type': PROBLEM_TYPE
	},
	'AFEA':{
		'retrain_autoencoder':True,
		'AE_MODEL_PATH':'',
		'code_length':32,
		'RL_LOAD_HISTORY':False;
		'RL_MODEL_PATH':'',
		'MEMORY_SIZE':4096,
		'replay_size':
	}



}