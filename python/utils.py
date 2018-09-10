# coding: utf-8
from math import sin, cos, sqrt
from six.moves import xrange

import h5py
import numpy as np
import scipy.io as sio
import tensorflow as tf


class Id:
    def __init__(self, Ia, Ib):
        self.Ia=Ia
        self.Ib=Ib
    def show(self):
        print('A: %s\nB: %s'%(self.Ia, self.Ib))



def newconvlayer_pooling(input_feature, input_dim, output_dim, nb_weights, edge_weights, nb, cotw,
                         name='meshconvpooling',
                         training=True, special_activation='fuck', no_activation=False, bn=True):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))

        padding_feature = tf.zeros([tf.shape(input_feature)[0], 1, input_dim], tf.float32)

        padded_input = tf.concat([padding_feature, input_feature], 1)
        def compute_nb_feature(input_f):
            return tf.gather(input_f, nb) * cotw

        total_nb_feature = tf.map_fn(compute_nb_feature, padded_input)
        mean_nb_feature = tf.reduce_sum(total_nb_feature, axis=2)

        nb_bias = tf.get_variable("nb_bias", [output_dim], tf.float32, initializer=tf.constant_initializer(0.0))
        nb_feature = tf.tensordot(mean_nb_feature, nb_weights, [[2], [0]]) + nb_bias

        edge_bias = tf.get_variable("edge_bias", [output_dim], tf.float32, initializer=tf.constant_initializer(0.0))
        edge_feature = tf.tensordot(input_feature, edge_weights, [[2], [0]]) + edge_bias

        total_feature = edge_feature + nb_feature

        if not bn:
            fb = total_feature
        else:
            fb = batch_norm_wrapper(total_feature, is_training=training)

        # if no_activation:
            # fa = fb
        # elif not special_activation:
            # fa = leaky_relu(fb)
        # else:
            # fa = tf.nn.tanh(fb)
        if no_activation:
            fa = fb
            print('no activation')
        elif special_activation == 'sigmoid':
            fa = 2.0*tf.nn.sigmoid(fb)-1.0
            print('sigmoid')
        elif special_activation == 'l_relu':
            fa = leaky_relu(fb)
            print('l_relu')
        elif special_activation == 'softsign':
            fa = tf.nn.softsign(fb)
            print('softsign')
        elif special_activation == 'softplusplus':
            fa = softplusplus(fb)
            print('softplusplus')
        else:
            fa = tf.nn.tanh(fb)
            print('tanh')

        return fa


def get_conv_weights(input_dim, output_dim, name='convweight'):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        n = tf.get_variable("nb_weights", [input_dim, output_dim], tf.float32,
                            tf.random_normal_initializer(stddev=0.02))
        e = tf.get_variable("edge_weights", [input_dim, output_dim], tf.float32,
                            tf.random_normal_initializer(stddev=0.02))
        return n, e


def get_conv_weights_diag(input_dim, output_dim, name='convweight'):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        n = tf.get_variable("nb_weights", [input_dim], tf.float32,
                            tf.random_normal_initializer(stddev=0.02))
        e = tf.get_variable("edge_weights", [input_dim], tf.float32,
                            tf.random_normal_initializer(stddev=0.02))
        return tf.diag(n), tf.diag(e)

def load_data(path, resultmin, resultmax, useS=True, graphconv=False):
    data = h5py.File(path)
    datalist = data.keys()
    # data = sio.loadmat(path)
    # .value.astype('float32')
    logr = np.transpose(data['FLOGRNEW'], (2, 1, 0))
    s = np.transpose(data['FS'], (2, 1, 0))
    # neighbour = data['neighbour']
    neighbour1 = np.transpose(data['neighbour1'])
    cotweight1 = np.transpose(data['cotweight1'])


    # sio.savemat('1.mat',{'logr':logr.astype('float32')})
    # f = np.zeros_like(logr).astype('float32')
    # f = logr
    # sio.savemat('2.mat', {'logr2': f})
    # demapping = np.transpose(data['demapping'])

    # degree = data['degrees']

    pointnum1 = neighbour1.shape[0]
    maxdegree1 = neighbour1.shape[1]
    modelnum = len(logr)

    logrmin = logr.min()
    logrmin = logrmin - 1e-6
    logrmax = logr.max()
    logrmax = logrmax + 1e-6
    smin = s.min()
    smin = smin - 1e-6
    smax = s.max()
    smax = smax + 1e-6

    rnew = (resultmax - resultmin) * (logr - logrmin) / (logrmax - logrmin) + resultmin
    snew = (resultmax - resultmin) * (s - smin) / (smax - smin) + resultmin
    if useS:
        feature = np.concatenate((rnew, snew), axis=2)
    else:
        feature = rnew

    f = np.zeros_like(feature).astype('float32')
    f = feature

    nb1 = np.zeros((pointnum1, maxdegree1)).astype('float32')
    nb1 = neighbour1

    L1 = np.zeros((pointnum1, pointnum1)).astype('float32')


    cotw1 = np.zeros((cotweight1.shape[0], cotweight1.shape[1], 1)).astype('float32')
    for i in range(1):
        cotw1[:, :, i] = cotweight1
    degree1 = np.zeros((neighbour1.shape[0], 1)).astype('float32')
    for i in range(neighbour1.shape[0]):
        degree1[i] = np.count_nonzero(nb1[i])



    return f, nb1, degree1, logrmin, logrmax, smin, smax, modelnum, pointnum1, maxdegree1, L1, cotw1


def bce(o, t):
    o = tf.clip_by_value(o, 1e-7, 1. - 1e-7)
    return tf.reduce_mean(-(t * tf.log(o) + (1. - t) * tf.log(1. - o)))


def loss_abs(a, b):
    return tf.reduce_mean(tf.abs(tf.subtract(a, b)))


def loss_mse(a, b):
    return tf.pow(tf.reduce_mean(tf.square(tf.subtract(a, b))), 0.5)


def load_data_old(path, result_max, result_min):
    data = sio.loadmat(path)
    datalist = data.keys()

    dismat = data['dismat']
    if 'lz_a' in datalist:
        metric_lz_a = data['lz_a']
        metric_lz_a = np.zeros_like(metric_lz_a).astype('float32')
        metric_lz_a = metric_lz_a.astype('float32')
    if 'lz_b' in datalist:
        metric_lz_b = data['lz_b']
        metric_lz_b = np.zeros_like(metric_lz_b).astype('float32')
        metric_lz_b = metric_lz_b.astype('float32')

    x = np.zeros_like(data).astype('float32')
    x = dismat.astype('float32')
    # x_min = x.min(axis = 0)
    x_min = x.min()
    x_min = x_min - 1e-6
    # x_max = x.max(axis = 0)
    x_max = x.max()
    x_max = x_max + 1e-6

    x = (result_max - result_min) * (x - x_min) / (x_max - x_min) + result_min

    if 'lz_a' in datalist and 'lz_b' in datalist:
        a=1
    else:
        metric_lz_a = np.array([])
        metric_lz_b = np.array([])

    return x, x_min, x_max, metric_lz_a, metric_lz_b

def load_lfd(path, std=100.0):
    data = sio.loadmat(path)
    datalist = data.keys()
    dismat = data['dismat']

    if 'lz_a' in datalist:
        metric_lz_a = data['lz_a']
        metric_lz_a = np.zeros_like(metric_lz_a).astype('float32')
        metric_lz_a = metric_lz_a.astype('float32')
    if 'lz_b' in datalist:
        metric_lz_b = data['lz_b']
        metric_lz_b = np.zeros_like(metric_lz_b).astype('float32')
        metric_lz_b = metric_lz_b.astype('float32')

    x = np.zeros_like(data).astype('float32')
    x = dismat.astype('float32')
    # x_min = x.min(axis = 0)
    x_mean = x.mean()
    x = (x-x_mean)/std


    # x = (result_max - result_min) * (x - x_min) / (x_max - x_min) + result_min

    if 'lz_a' in datalist and 'lz_b' in datalist:
        a=1
    else:
        metric_lz_a = np.array([])
        metric_lz_b = np.array([])

    return x, x_min, x_max, metric_lz_a, metric_lz_b

def recover_lfd(dis, mean, std):
    dis = dis * std + mean
    # dis = (dismax - dismin) * (dis - resultmin) / (resultmax - resultmin) + dismin
    return dis


def recover_data_old(dis, dismin, dismax, resultmin, resultmax):
    dis = (dismax - dismin) * (dis - resultmin) / (resultmax - resultmin) + dismin

    return dis


def recover_data(recover_feature, logrmin, logrmax, smin, smax, resultmin, resultmax, useS=True):
    logr = recover_feature[:, :, 0:3]

    logr = (logrmax - logrmin) * (logr - resultmin) / (resultmax - resultmin) + logrmin
    # feature=[]
    if useS:
        s = recover_feature[:, :, 3:9]
        s = (smax - smin) * (s - resultmin) / (resultmax - resultmin) + smin
        logr = np.concatenate((logr, s), axis=2)

    return logr


# -----------------------------------------------------------conv operation
def leaky_relu(input_, alpha=0.02):
    return tf.maximum(input_, alpha * input_)

def softplusplus(input_, alpha=0.02):
    return tf.log(1.0+tf.exp(input_*(1.0-alpha)))+alpha*input_-tf.log(2.0)


def linear_l2(input_, input_size, output_size, name='Linear', stddev=0.02, bias_start=0.0):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        matrix = tf.get_variable("weights", [input_size, output_size], tf.float32,
                                 tf.random_normal_initializer(stddev=stddev))
        bias = tf.get_variable("bias", [output_size], tf.float32,
                               initializer=tf.constant_initializer(bias_start))

        return tf.matmul(input_, matrix) + bias, matrix


def linear(input_, input_size, output_size, name='Linear', stddev=0.02, bias_start=0.0):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        matrix = tf.get_variable("weights", [input_size, output_size], tf.float32,
                                 tf.random_normal_initializer(stddev=stddev))
        bias = tf.get_variable("bias", [output_size], tf.float32,
                               initializer=tf.constant_initializer(bias_start))

        return tf.matmul(input_, matrix) + bias


def linear1(input_, matrix, output_size, name='Linear', stddev=0.02, bias_start=0.0):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        # matrix = tf.get_variable("weights", [input_size, output_size], tf.float32,
        #                          tf.random_normal_initializer(stddev=stddev))
        bias = tf.get_variable("bias", [output_size], tf.float32,
                               initializer=tf.constant_initializer(bias_start))

        return tf.matmul(input_, matrix) + bias


def batch_norm_wrapper(inputs, name='batch_norm', is_training=False, decay=0.9, epsilon=1e-5):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        if is_training == True:
            scale = tf.get_variable('scale', dtype=tf.float32, trainable=True,
                                    initializer=tf.ones([inputs.get_shape()[-1]], dtype=tf.float32))
            beta = tf.get_variable('beta', dtype=tf.float32, trainable=True,
                                   initializer=tf.zeros([inputs.get_shape()[-1]], dtype=tf.float32))
            pop_mean = tf.get_variable('overallmean', dtype=tf.float32, trainable=False,
                                       initializer=tf.zeros([inputs.get_shape()[-1]], dtype=tf.float32))
            pop_var = tf.get_variable('overallvar', dtype=tf.float32, trainable=False,
                                      initializer=tf.ones([inputs.get_shape()[-1]], dtype=tf.float32))
        else:
            scope.reuse_variables()
            scale = tf.get_variable('scale', dtype=tf.float32, trainable=True)
            beta = tf.get_variable('beta', dtype=tf.float32, trainable=True)
            pop_mean = tf.get_variable('overallmean', dtype=tf.float32, trainable=False)
            pop_var = tf.get_variable('overallvar', dtype=tf.float32, trainable=False)

        if is_training == True:
            axis = list(range(len(inputs.get_shape()) - 1))
            batch_mean, batch_var = tf.nn.moments(inputs, axis)
            train_mean = tf.assign(pop_mean, pop_mean * decay + batch_mean * (1 - decay))
            train_var = tf.assign(pop_var, pop_var * decay + batch_var * (1 - decay))
            with tf.control_dependencies([train_mean, train_var]):
                return tf.nn.batch_normalization(inputs, batch_mean, batch_var, beta, scale, epsilon)
        else:
            return tf.nn.batch_normalization(inputs, pop_mean, pop_var, beta, scale, epsilon)


def fclayer(inputs, input_dim, output_dim, name='fclayer', training=True):
    with tf.variable_scope(name) as scope:
        scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
        fcdot = linear(inputs, input_dim, output_dim, 'fclinear')
        fcbn = batch_norm_wrapper(fcdot, name='fcbn', is_training=training, decay=0.9)
        fca = leaky_relu(fcbn)

        return fca


# ------------------------------------------------------prior_factory.py------------------------
def onehot_categorical(batch_size, n_labels):
    y = np.zeros((batch_size, n_labels), dtype=np.float32)
    indices = np.random.randint(0, n_labels, batch_size)
    for b in range(batch_size):
        y[b, indices[b]] = 1
    return y


def uniform(batch_size, n_dim, n_labels=10, minv=-1, maxv=1, label_indices=None):
    if label_indices is not None:
        if n_dim != 2:
            raise Exception("n_dim must be 2.")

        def sample(label, n_labels):
            num = int(np.ceil(np.sqrt(n_labels)))
            size = (maxv - minv) * 1.0 / num
            x, y = np.random.uniform(-size / 2, size / 2, (2,))
            i = label / num
            j = label % num
            x += j * size + minv + 0.5 * size
            y += i * size + minv + 0.5 * size
            return np.array([x, y]).reshape((2,))

        z = np.empty((batch_size, n_dim), dtype=np.float32)
        for batch in range(batch_size):
            for zi in range(int(n_dim / 2)):
                z[batch, zi * 2:zi * 2 + 2] = sample(label_indices[batch], n_labels)
    else:
        z = np.random.uniform(minv, maxv, (batch_size, n_dim)).astype(np.float32)
    return z


def gaussian(batch_size, n_dim, mean=0, var=1, n_labels=10, use_label_info=False):
    if use_label_info:
        if n_dim != 2:
            raise Exception("n_dim must be 2.")

        def sample(n_labels):
            x, y = np.random.normal(mean, var, (2,))
            angle = np.angle((x - mean) + 1j * (y - mean), deg=True)

            label = (int(n_labels * angle)) // 360

            if label < 0:
                label += n_labels

            return np.array([x, y]).reshape((2,)), label

        z = np.empty((batch_size, n_dim), dtype=np.float32)
        z_id = np.empty((batch_size, 1), dtype=np.int32)
        for batch in range(batch_size):
            for zi in range(int(n_dim / 2)):
                a_sample, a_label = sample(n_labels)
                z[batch, zi * 2:zi * 2 + 2] = a_sample
                z_id[batch] = a_label
        return z, z_id
    else:
        z = np.random.normal(mean, var, (batch_size, n_dim)).astype(np.float32)
        return z


def gaussian_mixture(batch_size, n_dim=2, n_labels=10, x_var=0.5, y_var=0.1, label_indices=None):
    if n_dim != 2:
        raise Exception("n_dim must be 2.")

    def sample(x, y, label, n_labels):
        shift = 1.4
        r = 2.0 * np.pi / float(n_labels) * float(label)
        new_x = x * cos(r) - y * sin(r)
        new_y = x * sin(r) + y * cos(r)
        new_x += shift * cos(r)
        new_y += shift * sin(r)
        return np.array([new_x, new_y]).reshape((2,))

    x = np.random.normal(0, x_var, (batch_size, int(n_dim / 2)))
    y = np.random.normal(0, y_var, (batch_size, int(n_dim / 2)))
    z = np.empty((batch_size, n_dim), dtype=np.float32)
    for batch in range(batch_size):
        for zi in range(int(n_dim / 2)):
            if label_indices is not None:
                z[batch, zi * 2:zi * 2 + 2] = sample(x[batch, zi], y[batch, zi], label_indices[batch], n_labels)
            else:
                z[batch, zi * 2:zi * 2 + 2] = sample(x[batch, zi], y[batch, zi], np.random.randint(0, n_labels),
                                                     n_labels)

    return z


def swiss_roll(batch_size, n_dim=2, n_labels=10, label_indices=None):
    if n_dim != 2:
        raise Exception("n_dim must be 2.")

    def sample(label, n_labels):
        uni = np.random.uniform(0.0, 1.0) / float(n_labels) + float(label) / float(n_labels)
        r = sqrt(uni) * 3.0
        rad = np.pi * 4.0 * sqrt(uni)
        x = r * cos(rad)
        y = r * sin(rad)
        return np.array([x, y]).reshape((2,))

    z = np.zeros((batch_size, n_dim), dtype=np.float32)
    for batch in range(batch_size):
        for zi in range(int(n_dim / 2)):
            if label_indices is not None:
                z[batch, zi * 2:zi * 2 + 2] = sample(label_indices[batch], n_labels)
            else:
                z[batch, zi * 2:zi * 2 + 2] = sample(np.random.randint(0, n_labels), n_labels)
    return z
