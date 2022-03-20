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


class AFEA_model_dqn(torch.nn.Module):
    def __init__(self,config,writter):
        super(AFEA_model_dqn, self).__init__()
        self.config = config
        self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
        self.autoencoder = AutoEncoder(afea_config.Config['AutoEncoder'],writter)
        if config['retrain_autoencoder']:

            self.autoencoder.train()
        else:
            self.autoencoder.autoencoder = torch.load(config['AE_MODEL_PATH'])
            self.autoencoder.init_cluster()
        self.QNET = DQN(config['code_length']).to(self.device)
        if config['RL_LOAD_HISTORY']:
            self.QNET = torch.load(config['RL_MODEL_PATH']).to(self.device)
        self.encoder = self.autoencoder.encoder
        self.decoder = self.autoencoder.decoder
    def forward(self,x):
        x = x.to(self.device)

        # x_prime = self.decoder(codes)
        with torch.no_grad():
            codes = self.encoder(x)
            qvalues = self.QNET(codes)
        return codes,qvalues

class AFEA_dqn():
    def __init__(self,config,writter):
        self.config = config
        self.device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
        self.model = AFEA_model_dqn(config,writter).to(self.device)
        self.autoencoder = self.model.autoencoder
        self.optimizer = torch.optim.Adam(self.model.parameters(),self.config['lr'])
        self.criterion = torch.nn.MSELoss().to(self.device)
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
        actions = []
        for i in range(qvalues.shape[0]):
            if random.random() < epsilon:
                actions.append(random.randint(0,1))
            else:
                actions.append(torch.argmax(qvalues[i]).item())
            # actions[-1] = 1
        return actions

    def predict_codes(self,x):
        with torch.no_grad():
            return self.model.encoder(x)

    def replay(self, DB_center):
        targets = []
        ys = []
        if len(self.memory) >= self.config['replay_size']:
            transitions = random.sample(self.memory,self.config['replay_size'])
            for state,action,next_state,reward,is_done in transitions:
                codes = self.model.encoder(state.to(self.device))
                label = torch.argmin(torch.sum(torch.pow(codes - self.autoencoder.centers.to(self.device), 2), dim=1)).cpu().item()
                codes = codes - DB_center[label].to(self.device)
                qvalues = self.predict_q_value(codes).tolist()
                ys.append(self.model.QNET(codes))
                if is_done:
                    qvalues[action] = reward
                else:
                    qvalues[action] = reward + self.config['gamma'] * torch.max(self.predict_q_value(next_state)).item()
                targets.append(qvalues)

            
            return self.criterion(torch.Tensor(targets).to(self.device),torch.stack(ys))
        return torch.tensor(0.,requires_grad=True).to(self.device)
    def forward(self,x):
        return self.model(x)

    def learning(self, DB_centers):
        # reconstruction erre
        # clustering error
        self.optimizer.zero_grad()
        reconstruction_loss,clustering_loss = self.autoencoder.learning(return_loss = True)
        q_loss = self.replay(DB_centers)
        total_loss = q_loss + self.ALPHA * reconstruction_loss + self.BETA * clustering_loss
        total_loss.backward()
        self.optimizer.step()

        # torch.autograd.set_detect_anomaly(True)
        # with torch.autograd.detect_anomaly():
        #     total_loss.backward(retain_graph = True)


        # adjust clustering
        self.autoencoder.update_centers()
        self.autoencoder.reassignment()
        return reconstruction_loss.cpu().item(), clustering_loss.cpu().item(), q_loss.cpu().item(), self.autoencoder.calculate_entropy()

        # return 0, 0, q_loss.cpu().item(), self.autoencoder.calculate_entropy()

    def get_label(self, idx):
        return self.autoencoder.assignments[idx]









