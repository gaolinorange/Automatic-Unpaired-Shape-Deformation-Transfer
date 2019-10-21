import model
import tensorflow as tf
import argparse,os
from train_simnet import *
from train_vae import *
from train_gan import *


# DEFAULT SETTINGS
parser = argparse.ArgumentParser()
parser.add_argument('--gpu', type=int, default=0, help='GPU to use [default: GPU 0]')
parser.add_argument('--n_epoch_Vae', type=int, default=5000, help='Epoch of VAE [default: 5000]')
parser.add_argument('--n_epoch_Metric_1', type=int, default=2000, help='Epoch of SimNet step 1 [default: 2000]')
parser.add_argument('--n_epoch_Metric_2', type=int, default=10000, help='Epoch of SimNet step 2 [default: 10000]')
parser.add_argument('--n_epoch_Gan', type=int, default=1000, help='Epoch of Cycle Gan [default: 10000]')
parser.add_argument('--hidden_dim', type=int, default=128, help='latent space dimension [Default: 128]')
parser.add_argument('--dataname_a', type=str, default='horse', help='data set A name [Default: cat]')
parser.add_argument('--dataname_b', type=str, default='camel', help='data set B name [Default: lion]')
parser.add_argument('--test_vae', type=bool, default=True, help='Output vae test file [Default: true]')
parser.add_argument('--test_gan', type=bool, default=True, help='Output Sim Net test file [Default: true]')
parser.add_argument('--lambda_2', type=float, default=10.0, help='the weight of reconstructing mesh [Default: 10.0]')
parser.add_argument('--vae_ablity', type=float, default=0.0, help='% percent to test the vae [Default: 0.0]')
parser.add_argument('--logfolder', type=str, default='./', help='the output dir [Default: ./]')
parser.add_argument('--tb', type=bool, default=False, help='tensorboard use or not [Default: False]')
FLAGS = parser.parse_args()


model.n_epoch_Vae = FLAGS.n_epoch_Vae
model.n_epoch_Metric_1 = FLAGS.n_epoch_Metric_1
model.n_epoch_Metric_2 = FLAGS.n_epoch_Metric_2
model.n_epoch_Gan = FLAGS.n_epoch_Gan

model.hidden_dim = FLAGS.hidden_dim

model.dataname_a = FLAGS.dataname_a
model.dataname_b = FLAGS.dataname_b
model.test_vae = FLAGS.test_vae
model.test_gan = FLAGS.test_gan
model.tb = FLAGS.tb
model.lambda_2 = FLAGS.lambda_2
model.vae_ablity = FLAGS.vae_ablity
model.logfolder = FLAGS.logfolder

os.environ["CUDA_VISIBLE_DEVICES"] = str(FLAGS.gpu)

print(model.dataname_a)
train_model = model.convMESH()

with tf.Session(config=train_model.config) as train_model.sess:
    #'''
    train_model.train_pre()
    train_VAE(train_model)
    train_metric(train_model)
    train_GAN(train_model)

    '''
    train_model.train_pre()
    test_vae(train_model,30000)
    test_metric(train_model,22000)
    test_gan(train_model,30000)


    '''

