import torch
import math
import random
import os
import numpy as np
from collections import deque
from tensorboardX import SummaryWriter
import matplotlib.pyplot as plt
import afea_config
import copy
from Reinforcement_Agent import *
from AutoEncoder import *


class AFEA_model_dqn(torch.nn.module):
	def __init__(self,config):
		self.config = config
		self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
		self.autoencoder = AutoEncoder(afea_config.Config['AutoEncoder'])
		if config['retrain_autoencoder']:
			
			self.autoencoder.train()
		else:
			self.autoencoder.autoencoder = torch.load(config['AE_MODEL_PATH'])
			self.autoencoder.init_cluster()
		self.QNET = DQN(config['code_length']).to(self.device)
		if config['RL_LOAD_HISTORY']:
			self.QNET = torch.load(config['RL_MODEL_PATH']).to(device)
		self.encoder = self.autoencoder.encoder.to(self.device)
		self.decoder = self.autoencoder.decoder.to(self.device)
	def forward(self,x):
		codes = self.encoder(x)
		x_prime = self.decoder(codes)
		with torch.no_grad():
			qvalues = self.QNET(codes)
		return codes,x_prime,qvalues

class AFEA_dqn():
	def __init__(self,config):
		self.config = config
		self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
		self.model = AFEA_model_dqn(config).to(device)
		self.optimizer = torch.optim.Adam(self.model.parameters(),self.config['lr'])
        self.criterion = torch.nn.MSELoss().to(device)
        self.memory = deque(maxlen = self.config['MEMORY_SIZE'])
        self.ALPHA = config['W_reconstruction']
        self.BETA = config['W_clustering']

    # def update(self,s,y):
    #     y_pred = self.model(s)
    #     loss = self.criterion(y_pred,y)
    #     self.optimizer.zero_grad()
    #     loss.backward()
    #     self.optimizer.step()

    def update_memory(self, transitions):
    	for transition in transitions:
    		self.memory.append(transition)


    def predict(self,x):
        with torch.no_grad():
            return self.model(x)

    def predict_q_value(self,x):
    	with torch.no_grad():
    		return self.model.QNET(x)

    def epsilon_greedy_policy(self,qvalues,epsilon):
	    if random.random() < epsilon:
	        return random.randint(0,1)
	    else:
	        return torch.argmax(qvalues,dim=1).item()

	def predict_codes(self,x):
		with torch.no_grad():
			return self.model.encoder(x)


    def replay(self):
        targets = []
        ys = []
        if len(self.memory) >= self.config['replay_size']:
            transitions = random.sample(self.memory,self.config['replay_size'])
            for state,action,next_state,reward,is_done in transitions:
                states.append(state)
                qvalues = self.predict_q_value(state).tolist()
                ys.append(self.model.QNET(state).tolist())
                if is_done:
                    qvalues[action] = reward
                else:
                    qvalues[action] = reward + self.config['gamma'] * torch.max(self.predict_q_value(next_state)).item()
                targets.append(qvalues)

            
            return self.criterion(torch.Tensot(targets),torch.Tensor(ys))
        return torch.Tensor(0)

    def forward(self,x):
    	return self.model(x)

    def learning(self):
    	# reconstruction erre
    	# clustering error
    	reconstruction_loss,clustering_loss = self.autoencoder.learning(return_loss = True)
    	
    	# bellman error
    	q_loss = self.replay()

    	total_loss = q_loss + self.ALPHA * reconstruction_loss + self.BETA * clustering_loss
    	self.optimizer.zero_grad()
    	total_loss.backward()
    	self.optimizer.step()

    	# adjust clustering
    	self.autoencoder.update_centers()
    	self.autoencoder.reassignment()

    def get_label(self, idx):
        return self.autoencoder.assignments[idx]

    def update_label(self, request_list):
    	for idx, req, code in request_list:
    		self.autoencoder.dataset.replace_request(idx, req)
    		min_dis_idx = torch.argmin(torch.sum(torch.pow(code.cpu()-self.centers,2),dim=1)).cpu().item()
    		self.npoints[self.assignments[idx]] -= 1
    		self.assignments[idx] = min_dis_idx
    		self.npoints[min_dis_idx] += 1

				    





