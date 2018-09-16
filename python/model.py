# coding: utf-8
import os
import time

import tensorflow.contrib.slim as slim

from utils import *

n_epoch_Vae = 10000
n_epoch_Metric_1 = 12000
n_epoch_Metric_2 = 20000
n_epoch_Gan = 10000

hidden_dim = 128
batchsize = 128

dataname_a = 'cat'
dataname_b = 'lion'
featurefile_a = './'+dataname_a+'.mat'
featurefile_b = './'+dataname_b+'.mat'
lightfeildmat = dataname_a+dataname_b+'lfd.mat'

speed = 0.001
resultmax = 0.95
resultmin = -0.95
test_vae = True
test_gan = True
useS = True
tb = False
lambda_2 = 10.0
vae_ablity = 0.0
layer = 2
sp = False
timecurrent = time.strftime('%Y%m%d_%H%M%S', time.localtime(time.time()))
logfolder = './' + timecurrent + 'test'


# print(sp)
# os.environ["CUDA_VISIBLE_DEVICES"] = "1"


class convMESH():
    VAE = 'VAE'
    METRIC = 'METRIC'
    GAN = 'GAN'  # the part of GAN is coming soon

    config = tf.ConfigProto()
    config.gpu_options.allow_growth = True
    # config.log_device_placement = True
    # config.allow_soft_placement = True
    def __init__(self):
        self.sess = None
        if not os.path.isdir(logfolder):
            os.mkdir(logfolder)

        self.feature_a, self.neighbour1_a, self.degree1_a, self.logrmin_a, self.logrmax_a, self.smin_a, self.smax_a, self.modelnum_a, \
        self.pointnum1_a, self.maxdegree1_a, self.L1_a, self.cotw1_a = load_data(featurefile_a, resultmin, resultmax, useS=useS)

        self.feature_b, self.neighbour1_b, self.degree1_b, self.logrmin_b, self.logrmax_b, self.smin_b, self.smax_b, self.modelnum_b, \
        self.pointnum1_b, self.maxdegree1_b, \
        self.L1_b, self.L2_b, self.cotw1_b, self.cotw2_b = load_data(featurefile_b, resultmin, resultmax, useS=useS)

        self.lf_matrix, self.lf_matrix_min, self.lf_matrix_max, self.metric_lz_a, self.matric_lz_b = load_data_old(
            lightfeildmat, 2, 0.05)
        print(self.lf_matrix[0][0])
        print(self.lf_matrix[0][1])
        print(np.shape(self.metric_lz_a))
        # self.lf_matrix, self.lf_matrix_mean, self.lf_matrix_std = load_lfd(lightfeildmat)
        self.start = 'VAE'
        self.start_step_vae = 0
        self.start_step_metric = 0
        self.start_step_gan = 0

        self.batch_size = batchsize
        self.hidden_dim = hidden_dim

        self.dataset_name_a = dataname_a
        self.pointnum1_a = self.pointnum1_a
        self.maxdegree1_a = self.maxdegree1_a
        self.mapping1_col_a = self.mapping1_col_a
        self.model_num_a = self.modelnum_a
        self.lambda1_a = 10.0
        self.lambda2_a = lambda_2

        self.dataset_name_b = dataname_b
        self.pointnum1_b = self.pointnum1_b
        self.maxdegree1_b = self.maxdegree1_b
        self.mapping1_col_b = self.mapping1_col_b
        self.model_num_b = self.modelnum_b
        self.lambda1_b = 10.0
        self.lambda2_b = lambda_2


        self.log_dir = logfolder
        self.checkpoint_dir = logfolder

        if not useS:
            self.vertex_dim = 3
            self.finaldim = 3
        else:
            self.vertex_dim = 9
            self.finaldim = 9
        # ---------------------------------------------------feed_dict
        self.lf_dis = tf.placeholder(tf.float32, [None, 1], name='lf_dis')
        self.inputs_a = tf.placeholder(tf.float32, [None, self.pointnum1_a, self.vertex_dim], name='a/input_mesh')
        self.inputs_b = tf.placeholder(tf.float32, [None, self.pointnum1_b, self.vertex_dim], name='b/input_mesh')
        self.random_a = tf.placeholder(tf.float32, [None, self.hidden_dim], name='a/random_samples')
        self.random_b = tf.placeholder(tf.float32, [None, self.hidden_dim], name='b/random_samples')
        # ---------------------------------------------------

        # >>>>>>>>>>>>>>>>>>>
        # >>>>>>>>>>>>>>>>>>>
        # >>>>>>>>>>>>>>>>>>>
        # >>>>>>>>>>>>>>>>>>>
        # 
        # ---------------------------------------------------a
        self.nb1_a = tf.constant(self.neighbour1_a, dtype='int32', shape=[self.pointnum1_a, self.maxdegree1_a],
                                 name='a/nb_relation1')
        self.degrees1_a = tf.constant(self.degree1_a, dtype='float32', shape=[self.pointnum1_a, 1], name='a/degrees1')

        self.cw1_a = tf.constant(self.cotw1_a, dtype='float32', shape=[self.pointnum1_a, self.maxdegree1_a, 1],
                                 name='a/cw1')

        self.Laplace1_a = tf.constant(self.L1_a, dtype='float32', shape=[self.pointnum1_a, self.pointnum1_a],
                                      name='a/L1')


        # self.embedding_inputs = tf.placeholder(tf.float32, [None, self.hidden_dim], name='embedding_inputs')
        self.object_stddev_a = tf.constant(np.concatenate((np.array([1, 1]).astype('float32'), 1
                                                           * np.ones(self.hidden_dim - 2).astype('float32'))))
        if layer >= 1:
            self.vae_n1_a, self.vae_e1_a = get_conv_weights(self.vertex_dim, self.vertex_dim, name='encoder/a/convw1')
            print("layer|%d" % (layer))
        if layer >= 2:
            self.vae_n2_a, self.vae_e2_a = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/a/convw2')
            print("layer|%d" % (layer))
        if layer >= 3:
            self.vae_n3_a, self.vae_e3_a = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/a/convw3')
            print("layer|%d" % (layer))
        if layer >= 4:
            self.vae_n4_a, self.vae_e4_a = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/a/convw4')
            print("layer|%d" % (layer))
        if layer >= 5:
            self.vae_n5_a, self.vae_e5_a = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/a/convw5')
            print("layer|%d" % (layer))
        self.vae_meanpara_a = tf.get_variable("encoder/a/mean_weights",
                                              [self.pointnum1_a * self.finaldim, self.hidden_dim],
                                              tf.float32, tf.random_normal_initializer(stddev=0.02))
        self.vae_stdpara_a = tf.get_variable("encoder/a/std_weights",
                                             [self.pointnum1_a * self.finaldim, self.hidden_dim],
                                             tf.float32, tf.random_normal_initializer(stddev=0.02))
        # self.vae_mean_dpara_a = tf.get_variable("decoder/a/mean_weights",
        #                                      [self.pointnum2_a * self.finaldim, self.hidden_dim],
        #                                      tf.float32, tf.random_normal_initializer(stddev=0.02))
        # -------------------------------------------
        # self.gen_n1_a, self.gen_e1_a = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='gen/a/convw1')
        # self.gen_n2_a, self.gen_e2_a = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='gen/a/convw2')
        # self.gen_n3_a, self.gen_e3_a = self.get_conv_weights(self.vertex_dim, self.finaldim, name='gen/a/convw3')
        #
        # self.dis_n3_a, self.dis_e3_a = self.get_conv_weights(self.finaldim, self.vertex_dim, name='dis/a/convw1')
        # self.dis_n2_a, self.dis_e2_a = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='dis/a/convw2')
        # self.dis_n1_a, self.dis_e1_a = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='dis/a/convw3')
        #
        # self.gen_dense_a = tf.get_variable("gen/a/dense_weights", [self.hidden_dim, self.pointnum2_b * self.finaldim],
        #                                    tf.float32,
        #                                    tf.random_normal_initializer(stddev=0.02))
        # self.dis_dense_a = tf.get_variable("dis/a/dense_weights", [self.pointnum2_b * self.finaldim, 1], tf.float32,
        #                                    tf.random_normal_initializer(stddev=0.02))

        self.z_mean_a, self.z_stddev_a = self.encoder_a(self.inputs_a)
        self.guessed_z_a = self.z_mean_a + self.z_stddev_a * self.random_a

        self.z_mean_test_a, self.z_stddev_test_a = self.encoder_a(self.inputs_a, train=False)
        self.guessed_z_test_a = self.z_mean_test_a + self.z_stddev_test_a * self.random_a

        self.generated_mesh_train_a = self.decoder_a(self.guessed_z_a, train=True)
        self.generated_mesh_train_a = tf.clip_by_value(self.generated_mesh_train_a, resultmin + 1e-8, resultmax - 1e-8)
        self.generated_mesh_test_a = self.decoder_a(self.guessed_z_test_a, train=False)
        self.test_mesh_a = self.decoder_a(self.random_a, train=False)

        marginal_likelihood_a = tf.reduce_sum(tf.pow(self.inputs_a - self.generated_mesh_train_a, 2.0),
                                              [1, 2])  # + tf.abs(self.z_mean)
        KL_divergence_a = 0.5 * tf.reduce_sum(
            tf.square(self.z_mean_a) + tf.square(self.z_stddev_a / self.object_stddev_a) - tf.log(
                1e-8 + tf.square(self.z_stddev_a / self.object_stddev_a)) - 1, 1)

        self.neg_loglikelihood_a = self.lambda2_a * tf.reduce_mean(marginal_likelihood_a)
        self.KL_divergence_a = self.lambda1_a * tf.reduce_mean(KL_divergence_a)

        ELBO_a = - self.neg_loglikelihood_a - self.KL_divergence_a
        if layer >= 1:
            self.r2_a = tf.nn.l2_loss(self.vae_e1_a) + tf.nn.l2_loss(self.vae_n1_a)
            print("layer|%d" % (layer))
        if layer >= 2:
            self.r2_a = self.r2_a + tf.nn.l2_loss(self.vae_e2_a) + tf.nn.l2_loss(self.vae_n2_a)
            print("layer|%d" % (layer))
        if layer >= 3:
            self.r2_a = self.r2_a + tf.nn.l2_loss(self.vae_e3_a) + tf.nn.l2_loss(self.vae_n3_a)
            print("layer|%d" % (layer))
        if layer >= 4:
            self.r2_a = self.r2_a + tf.nn.l2_loss(self.vae_e4_a) + tf.nn.l2_loss(self.vae_n4_a)
            print("layer|%d" % (layer))
        if layer >= 5:
            self.r2_a = self.r2_a + tf.nn.l2_loss(self.vae_e5_a) + tf.nn.l2_loss(self.vae_n5_a)
            print("layer|%d" % (layer))

        # self.r2_a = tf.nn.l2_loss(self.vae_e1_a) + tf.nn.l2_loss(self.vae_n1_a) + \
        #            tf.nn.l2_loss(self.vae_e2_a) + tf.nn.l2_loss(self.vae_n2_a) + \
        #            tf.nn.l2_loss(self.vae_e3_a) + tf.nn.l2_loss(self.vae_n3_a) + \
        self.r2_a = self.r2_a + tf.nn.l2_loss(self.vae_stdpara_a) + tf.nn.l2_loss(
            self.vae_meanpara_a)  # + tf.nn.l2_loss(self.vae_mean_dpara_a)
        # self.r2_a = tf.losses.get_regularization_loss(scope='encoder/a') + tf.losses.get_regularization_loss(scope='decoder/a')
        reg_losses = tf.get_collection(tf.GraphKeys.REGULARIZATION_LOSSES, scope='encoder/a') + tf.get_collection(
            tf.GraphKeys.REGULARIZATION_LOSSES, scope='decoder/a')
        self.r2_a += sum(reg_losses)

        # ---------------------------------------------------bbbbbbbbb------------------------------
        self.nb1_b = tf.constant(self.neighbour1_b, dtype='int32', shape=[self.pointnum1_b, self.maxdegree1_b],
                                 name='b/nb_relation1')
        self.degrees1_b = tf.constant(self.degree1_b, dtype='float32', shape=[self.pointnum1_b, 1], name='b/degrees1')
        self.cw1_b = tf.constant(self.cotw1_b, dtype='float32', shape=[self.pointnum1_b, self.maxdegree1_b, 1],
                                 name='b/cw1')

        # self.embedding_inputs = tf.placeholder(tf.float32, [None, self.hidden_dim], name='embedding_inputs')
        self.object_stddev_b = tf.constant(
            np.concatenate((np.array([1, 1]).astype('float32'), 1 * np.ones(self.hidden_dim - 2).astype('float32'))))
        # ----------------------------------------
        # self.vae_n1_b, self.vae_e1_b = get_conv_weights(self.vertex_dim, self.vertex_dim, name='encoder/b/convw1')
        # self.vae_n2_b, self.vae_e2_b = get_conv_weights(self.vertex_dim, self.vertex_dim, name='encoder/b/convw2')
        # self.vae_n3_b, self.vae_e3_b = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/b/convw3')
        if layer >= 1:
            self.vae_n1_b, self.vae_e1_b = get_conv_weights(self.vertex_dim, self.vertex_dim, name='encoder/b/convw1')
            print("layer|%d" % (layer))
        if layer >= 2:
            self.vae_n2_b, self.vae_e2_b = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/b/convw2')
            print("layer|%d" % (layer))
        if layer >= 3:
            self.vae_n3_b, self.vae_e3_b = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/b/convw3')
            print("layer|%d" % (layer))
        if layer >= 4:
            self.vae_n4_b, self.vae_e4_b = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/b/convw4')
            print("layer|%d" % (layer))
        if layer >= 5:
            self.vae_n5_b, self.vae_e5_b = get_conv_weights(self.vertex_dim, self.finaldim, name='encoder/b/convw5')
            print("layer|%d" % (layer))
        self.vae_meanpara_b = tf.get_variable("encoder/b/mean_weights",
                                              [self.pointnum1_b * self.finaldim, self.hidden_dim],
                                              tf.float32,
                                              tf.random_normal_initializer(stddev=0.02))
        self.vae_stdpara_b = tf.get_variable("encoder/b/std_weights",
                                             [self.pointnum1_b * self.finaldim, self.hidden_dim],
                                             tf.float32,
                                             tf.random_normal_initializer(stddev=0.02))
        # self.vae_mean_dpara_b = tf.get_variable("decoder/b/mean_weights",
        # [self.pointnum2_b * self.finaldim, self.hidden_dim],
        # tf.float32,
        # tf.random_normal_initializer(stddev=0.02))
        # --
        # self.gen_n1_b, self.gen_e1_b = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='gen/b/convw1')
        # self.gen_n2_b, self.gen_e2_b = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='gen/b/convw2')
        # self.gen_n3_b, self.gen_e3_b = self.get_conv_weights(self.vertex_dim, self.finaldim, name='gen/b/convw3')
        #
        # self.dis_n3_b, self.dis_e3_b = self.get_conv_weights(self.finaldim, self.vertex_dim, name='dis/b/convw1')
        # self.dis_n2_b, self.dis_e2_b = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='dis/b/convw2')
        # self.dis_n1_b, self.dis_e1_b = self.get_conv_weights(self.vertex_dim, self.vertex_dim, name='dis/b/convw3')
        #
        # self.gen_dense_b = tf.get_variable("gen/b/dense_weights", [self.hidden_dim, self.pointnum2_a * self.finaldim],
        #                                    tf.float32,
        #                                    tf.random_normal_initializer(stddev=0.02))
        # self.dis_dense_b = tf.get_variable("dis/b/dense_weights", [self.pointnum2_a * self.finaldim, 1], tf.float32,
        #                                    tf.random_normal_initializer(stddev=0.02))

        self.z_mean_b, self.z_stddev_b = self.encoder_b(self.inputs_b)
        self.guessed_z_b = self.z_mean_b + self.z_stddev_b * self.random_b

        self.z_mean_test_b, self.z_stddev_test_b = self.encoder_b(self.inputs_b, train=False)
        self.guessed_z_test_b = self.z_mean_test_b + self.z_stddev_test_b * self.random_b

        self.generated_mesh_train_b = self.decoder_b(self.guessed_z_b, train=True)
        self.generated_mesh_train_b = tf.clip_by_value(self.generated_mesh_train_b, resultmin + 1e-8, resultmax - 1e-8)
        self.generated_mesh_test_b = self.decoder_b(self.guessed_z_test_b, train=False)
        self.test_mesh_b = self.decoder_b(self.random_b, train=False)

        marginal_likelihood_b = tf.reduce_sum(tf.pow(self.inputs_b - self.generated_mesh_train_b, 2.0),
                                              [1, 2])  # + tf.abs(self.z_mean)
        KL_divergence_b = 0.5 * tf.reduce_sum(
            tf.square(self.z_mean_b) + tf.square(self.z_stddev_b / self.object_stddev_b) - tf.log(
                1e-8 + tf.square(self.z_stddev_b / self.object_stddev_b)) - 1, 1)

        self.neg_loglikelihood_b = self.lambda2_b * tf.reduce_mean(marginal_likelihood_b)
        self.KL_divergence_b = self.lambda1_b * tf.reduce_mean(KL_divergence_b)

        ELBO_b = - self.neg_loglikelihood_b - self.KL_divergence_b
        if layer >= 1:
            self.r2_b = tf.nn.l2_loss(self.vae_e1_b) + tf.nn.l2_loss(self.vae_n1_b)
            print("layer|%d" % (layer))
        if layer >= 2:
            self.r2_b = self.r2_b + tf.nn.l2_loss(self.vae_e2_b) + tf.nn.l2_loss(self.vae_n2_b)
            print("layer|%d" % (layer))
        if layer >= 3:
            self.r2_b = self.r2_b + tf.nn.l2_loss(self.vae_e3_b) + tf.nn.l2_loss(self.vae_n3_b)
            print("layer|%d" % (layer))
        if layer >= 4:
            self.r2_b = self.r2_b + tf.nn.l2_loss(self.vae_e4_b) + tf.nn.l2_loss(self.vae_n4_b)
            print("layer|%d" % (layer))
        if layer >= 5:
            self.r2_b = self.r2_b + tf.nn.l2_loss(self.vae_e5_b) + tf.nn.l2_loss(self.vae_n5_b)
            print("layer|%d" % (layer))

        self.r2_b = self.r2_b + tf.nn.l2_loss(self.vae_stdpara_b) + tf.nn.l2_loss(
            self.vae_meanpara_b)  # + tf.nn.l2_loss(self.vae_mean_dpara_b)
        reg_losses = tf.get_collection(tf.GraphKeys.REGULARIZATION_LOSSES, scope='encoder/b') + tf.get_collection(
            tf.GraphKeys.REGULARIZATION_LOSSES, scope='decoder/b')

        self.r2_b += sum(reg_losses)

        # <<<<<<<<<
        # <<<<<<<<<
        # <<<<<<<<<
        # <<<<<<<<<

        # >>>>>>>>>
        # >>>>>>>>>
        # >>>>>>>>>
        # >>>>>>>>>
        self.distance = self.metric_net(self.z_mean_test_a, self.z_mean_test_b)
        self.loss_metric_l2 = sum(tf.get_collection(tf.GraphKeys.REGULARIZATION_LOSSES, scope='metric'))


        # <<<<<<<<<
        # <<<<<<<<<
        # <<<<<<<<<
        # <<<<<<<<<

        # ---------------------------------------------------test
        # ---------------------------------------------------Metric
        self.distance_test = self.metric_net(self.random_a, self.random_b, reuse=True)

        # ---------------------------------------------------loss

        self.loss_metric_l2 = self.loss_metric_l2 * 0.1
        self.loss_metric = loss_mse(self.lf_dis, self.distance) * 1000 + self.loss_metric_l2

        # self.loss_vae = -ELBO_a - ELBO_b + 0.1 * self.r2_a + 0.1 * self.r2_b
        # -------------------------------------------------
        self.loss_vae_a = -ELBO_a + 0.15 * self.r2_a
        self.loss_vae_b = -ELBO_b + 0.15 * self.r2_b
        # -----------------------------------------

        # ----------------------------------------------------train opts
        tf.summary.scalar('loss_metric', self.loss_metric)
        tf.summary.scalar('loss_metric_l2', self.loss_metric_l2)
        # tf.summary.scalar('loss_vae', self.loss_vae)
        # ---------------------------------------------
        tf.summary.scalar('loss_vae_a', self.loss_vae_a)
        tf.summary.scalar('loss_vae_b', self.loss_vae_b)
        # ------------------------------------------

        tf.summary.scalar("nll_a", self.neg_loglikelihood_a)
        tf.summary.scalar("kl_a", self.KL_divergence_a)
        tf.summary.scalar("L2_loss_a", self.r2_a)
        tf.summary.scalar("nll_b", self.neg_loglikelihood_b)
        tf.summary.scalar("kl_b", self.KL_divergence_b)
        tf.summary.scalar("L2_loss_b", self.r2_b)

        # self.optimizer_vae = tf.train.AdamOptimizer(speed, beta1=0.5, name='encoder')
        # ----------------------------------------------------
        self.optimizer_vae_a = tf.train.AdamOptimizer(speed, name='encoder/a')
        self.optimizer_vae_b = tf.train.AdamOptimizer(speed, name='encoder/b')
        # ------------------------------------------

        self.optimizer_metric_1 = tf.train.AdamOptimizer(speed, name='metric1')
        self.optimizer_metric_2 = tf.train.AdamOptimizer(speed / 20.0, name='metric2')

        # variables_encoder = slim.get_variables(scope="encoder")
        # ------------------------------------------------------
        variables_encoder_a = slim.get_variables(scope="encoder/a")
        variables_encoder_b = slim.get_variables(scope="encoder/b")
        variables_decoder_a = slim.get_variables(scope="decoder/a")
        variables_decoder_b = slim.get_variables(scope="decoder/b")
        # -----------------------------------------
        # variables_decoder = slim.get_variables(scope="decoder")
        variables_metric = slim.get_variables(scope="metric")

        # train_variables_vae = []
        # --------------------------------------------
        train_variables_vae_a = []
        train_variables_vae_b = []
        train_variables_vae_all = []
        variables_vae_all = []
        variables_vae_a = []
        variables_vae_b = []
        # ------------------------------------------
        train_variables_metric = []
        # ---------------------------------------------test para
        variables_vae_metric = []

        # ----------------select trainable variables for every par
        # for v in variables_encoder:
        #     if v in tf.trainable_variables():
        #         train_variables_vae.append(v)
        # ------------------------------------------
        for v in variables_encoder_a:
            variables_vae_metric.append(v)
            variables_vae_a.append(v)
            variables_vae_all.append(v)
            if v in tf.trainable_variables():
                train_variables_vae_a.append(v)
                train_variables_vae_all.append(v)
        for v in variables_decoder_a:
            variables_vae_metric.append(v)
            variables_vae_a.append(v)
            variables_vae_all.append(v)
            if v in tf.trainable_variables():
                train_variables_vae_a.append(v)
                train_variables_vae_all.append(v)
        for v in variables_encoder_b:
            variables_vae_metric.append(v)
            variables_vae_b.append(v)
            variables_vae_all.append(v)
            if v in tf.trainable_variables():
                train_variables_vae_b.append(v)
                train_variables_vae_all.append(v)
        for v in variables_decoder_b:
            variables_vae_metric.append(v)
            variables_vae_b.append(v)
            variables_vae_all.append(v)
            if v in tf.trainable_variables():
                train_variables_vae_b.append(v)
                train_variables_vae_all.append(v)
        # -----------------------------------------
        for v in variables_metric:
            variables_vae_metric.append(v)
            if v in tf.trainable_variables():
                train_variables_metric.append(v)


        # -----------------------------------------------------
        self.saver_vae_a = tf.train.Saver(variables_vae_a, max_to_keep=None)
        self.saver_vae_b = tf.train.Saver(variables_vae_b, max_to_keep=None)
        self.saver_vae_all = tf.train.Saver(variables_vae_all, max_to_keep=None)
        # -----------------------------------------
        self.saver_metric = tf.train.Saver(variables_metric, max_to_keep=None)

        # -------------------------------------------
        self.train_op_vae_a = tf.contrib.training.create_train_op(self.loss_vae_a, self.optimizer_vae_a,
                                                                  variables_to_train=train_variables_vae_a,
                                                                  summarize_gradients=True)
        self.train_op_vae_b = tf.contrib.training.create_train_op(self.loss_vae_b, self.optimizer_vae_b,
                                                                  variables_to_train=train_variables_vae_b,
                                                                  summarize_gradients=True)
        # --------------------------------------------
        self.train_op_metric_1 = tf.contrib.training.create_train_op(self.loss_metric, self.optimizer_metric_1,
                                                                     variables_to_train=train_variables_metric,
                                                                     summarize_gradients=True)
        self.train_op_metric_2 = tf.contrib.training.create_train_op(self.loss_metric, self.optimizer_metric_2,
                                                                     variables_to_train=train_variables_metric,
                                                                     summarize_gradients=True)

        self.checkpoint_dir_vae_a = os.path.join(logfolder, self.model_dir(self.VAE, dataname_a))
        self.checkpoint_dir_vae_b = os.path.join(logfolder, self.model_dir(self.VAE, dataname_b))
        self.checkpoint_dir_vae_all = os.path.join(logfolder, self.model_dir(self.VAE, dataname_a + dataname_b))
        self.checkpoint_dir_metric = os.path.join(logfolder, self.model_dir(self.METRIC, dataname_a + dataname_b))
        self.checkpoint_dir_gan = os.path.join(logfolder, self.model_dir(self.GAN, dataname_a + dataname_b))

        if os.path.exists(logfolder + '/log.txt'):
            self.file = open(logfolder + '/log.txt', 'a')
        else:
            self.file = open(logfolder + '/log.txt', 'w')

        if os.path.exists(logfolder + '/log.vae'):
            self.file_vae = open(logfolder + '/log.vae' + timecurrent, 'w')
        else:
            self.file_vae = open(logfolder + '/log.vae', 'w')

        if os.path.exists(logfolder + '/log.metric'):

            self.file_metric = open(logfolder + '/log.metric' + timecurrent, 'w')
        else:
            self.file_metric = open(logfolder + '/log.metric', 'w')

        if os.path.exists(logfolder + '/log.gan'):
            self.file_gan = open(logfolder + '/log.gan' + timecurrent, 'w')
        else:
            self.file_gan = open(logfolder + '/log.gan', 'w')

        self.merge_summary = tf.summary.merge_all()

    # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>metric>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    def metric_net(self, A, B, training=True, reuse=False):
        with tf.variable_scope("metric") as scope:
            scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
            if not training or reuse:
                training = False
                reuse = True
                scope.reuse_variables()

            h1, matrix = linear_l2(tf.concat([A, B], axis=1), self.hidden_dim * 2, 2048, 'h1')
            # h1, matrix = linear_l2(tf.abs(tf.subtract(A, B)), self.hidden_dim, 1024, 'h1')
            # self.loss_metric_l2 = tf.nn.l2_loss(matrix)
            h1bn = batch_norm_wrapper(h1, name='h1bn', is_training=training, decay=0.9)
            h1a = leaky_relu(h1bn)

            h2, matrix = linear_l2(h1a, 2048, 1024, 'h2')
            # self.loss_metric_l2 += tf.nn.l2_loss(matrix)
            h2bn = batch_norm_wrapper(h2, name='h2bn', is_training=training, decay=0.9)
            h2a = leaky_relu(h2bn)

            h3, matrix = linear_l2(h2a, 1024, 512, 'h3')
            # self.loss_metric_l2 += tf.nn.l2_loss(matrix)
            h3bn = batch_norm_wrapper(h3, name='h3bn', is_training=training, decay=0.9)
            h3a = leaky_relu(h3bn)

            h4, matrix = linear_l2(h3a, 512, 256, 'h4')
            # self.loss_metric_l2 += tf.nn.l2_loss(matrix)
            h4bn = batch_norm_wrapper(h4, name='h4bn', is_training=training, decay=0.9)
            h4a = leaky_relu(h4bn)

            distance, matrix = linear_l2(h4a, 256, 1, 'distance')
            # self.loss_metric_l2 += tf.nn.l2_loss(matrix)
            print(tf.concat([A, B], axis=1).shape)
            print(h1.shape)

            return tf.square(distance)
            # return tf.abs(distance)

    # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<metric<<<<<<<<<<<<<<<<<<<<<<<<<<

    # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> A  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    def encoder_a(self, input_feature, train=True, reuse=False):
        global conv1
        with tf.variable_scope("encoder/a") as scope:
            scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
            if not train or reuse:
                train = False
                reuse = True
                scope.reuse_variables()
            bnbn = True
            if layer >= 1:
                print("layer|%d" % (layer))
                if layer == 1:
                    bnbn = False
                conv1 = newconvlayer_pooling(input_feature, self.vertex_dim, self.vertex_dim, self.vae_n1_a,
                                             self.vae_e1_a, self.nb1_a,
                                             self.cw1_a, name='conv1', training=train, bn=bnbn, special_activation=sp)
            if layer >= 2:
                print("layer|%d" % (layer))
                if layer == 2:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n2_a, self.vae_e2_a,
                                             self.nb1_a, self.cw1_a,
                                             name='conv2', training=train, bn=bnbn, special_activation=sp)
                print('layer')
            if layer >= 3:
                print("layer|%d" % (layer))
                if layer == 3:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n3_a, self.vae_e3_a,
                                             self.nb1_a, self.cw1_a,
                                             name='conv3', training=train, bn=bnbn, special_activation=sp)
            if layer >= 4:
                print("layer|%d" % (layer))
                if layer == 4:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n4_a, self.vae_e4_a,
                                             self.nb1_a, self.cw1_a,
                                             name='conv4', training=train, bn=bnbn, special_activation=sp)
            if layer >= 5:
                print("layer|%d" % (layer))
                if layer == 5:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n5_a, self.vae_e5_a,
                                             self.nb1_a, self.cw1_a,
                                             name='conv5', training=train, bn=bnbn, special_activation=sp)

            conv1 = conv1

            # conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.finaldim, self.vae_n3_a, self.vae_e3_a,
            #                             self.nb2_a, self.cw2_a,
            #                             name='conv3', training=train, bn=False)
            l0 = tf.reshape(conv1, [tf.shape(conv1)[0], self.pointnum1_a * self.finaldim])
            mean = linear1(l0, self.vae_meanpara_a, self.hidden_dim, 'mean')
            stddev = linear1(l0, self.vae_stdpara_a, self.hidden_dim, 'stddev')
            stddev = tf.sqrt(2 * tf.nn.sigmoid(stddev))

            return mean, stddev

    def decoder_a(self, latent_tensor, train=True, reuse=False):
        with tf.variable_scope("decoder/a") as scope:
            scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
            if not train or reuse:
                train = False
                reuse = True
                scope.reuse_variables()

            l1 = linear1(latent_tensor, tf.transpose(self.vae_meanpara_a), self.pointnum1_a * self.finaldim, 'mean')
            conv1 = tf.reshape(l1, [tf.shape(l1)[0], self.pointnum1_a, self.finaldim])
            # conv1 = newconvlayer_pooling(l2, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n3_a),
            #                             tf.transpose(self.vae_e3_a),
            #                             self.nb2_a, self.cw2_a, name='conv4', training=train)
            if layer == 1:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n1_a),
                                             tf.transpose(self.vae_e1_a),
                                             self.nb1_a, self.cw1_a, name='conv2', training=train, bn=False,
                                             special_activation=sp)
            if layer == 2:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n2_a),
                                             tf.transpose(self.vae_e2_a),
                                             self.nb1_a, self.cw1_a, name='conv3', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n1_a),
                                             tf.transpose(self.vae_e1_a),
                                             self.nb1_a, self.cw1_a, name='conv4', training=train, bn=False,
                                             special_activation=sp)
            if layer == 3:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n3_a),
                                             tf.transpose(self.vae_e3_a),
                                             self.nb1_a, self.cw1_a, name='conv4', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n2_a),
                                             tf.transpose(self.vae_e2_a),
                                             self.nb1_a, self.cw1_a, name='conv5', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n1_a),
                                             tf.transpose(self.vae_e1_a),
                                             self.nb1_a, self.cw1_a, name='conv6', training=train, bn=False,
                                             special_activation=sp)
            if layer == 4:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n4_a),
                                             tf.transpose(self.vae_e4_a),
                                             self.nb1_a, self.cw1_a, name='conv5', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n3_a),
                                             tf.transpose(self.vae_e3_a),
                                             self.nb1_a, self.cw1_a, name='conv6', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n2_a),
                                             tf.transpose(self.vae_e2_a),
                                             self.nb1_a, self.cw1_a, name='conv7', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n1_a),
                                             tf.transpose(self.vae_e1_a),
                                             self.nb1_a, self.cw1_a, name='conv8', training=train, bn=False,
                                             special_activation=sp)
            if layer == 5:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n5_a),
                                             tf.transpose(self.vae_e5_a),
                                             self.nb1_a, self.cw1_a, name='conv6', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n4_a),
                                             tf.transpose(self.vae_e4_a),
                                             self.nb1_a, self.cw1_a, name='conv7', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n3_a),
                                             tf.transpose(self.vae_e3_a),
                                             self.nb1_a, self.cw1_a, name='conv8', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n2_a),
                                             tf.transpose(self.vae_e2_a),
                                             self.nb1_a, self.cw1_a, name='conv9', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n1_a),
                                             tf.transpose(self.vae_e1_a),
                                             self.nb1_a, self.cw1_a, name='conv10', training=train, bn=False,
                                             special_activation=sp)

            conv1 = conv1



        return conv1


    # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< A  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    # >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> B  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

    def encoder_b(self, input_feature, train=True, reuse=False):
        global conv1
        with tf.variable_scope("encoder/b") as scope:
            scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
            if not train or reuse:
                train = False
                reuse = True
                scope.reuse_variables()
            bnbn = True
            if layer >= 1:
                print("layer|%d" % (layer))
                if layer == 1:
                    bnbn = False
                conv1 = newconvlayer_pooling(input_feature, self.vertex_dim, self.vertex_dim, self.vae_n1_b,
                                             self.vae_e1_b, self.nb1_b,
                                             self.cw1_b, name='conv1', training=train, bn=bnbn, special_activation=sp)
            if layer >= 2:
                print("layer|%d" % (layer))
                if layer == 2:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n2_b, self.vae_e2_b,
                                             self.nb1_b, self.cw1_b,
                                             name='conv2', training=train, bn=bnbn, special_activation=sp)
            if layer >= 3:
                print("layer|%d" % (layer))
                if layer == 3:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n3_b, self.vae_e3_b,
                                             self.nb1_b, self.cw1_b,
                                             name='conv3', training=train, bn=bnbn, special_activation=sp)
            if layer >= 4:
                print("layer|%d" % (layer))
                if layer == 4:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n4_b, self.vae_e4_b,
                                             self.nb1_b, self.cw1_b,
                                             name='conv4', training=train, bn=bnbn, special_activation=sp)
            if layer >= 5:
                print("layer|%d" % (layer))
                if layer == 5:
                    bnbn = False
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, self.vae_n5_b, self.vae_e5_b,
                                             self.nb1_b, self.cw1_b,
                                             name='conv5', training=train, bn=bnbn, special_activation=sp)

            conv1 = conv1

            l0 = tf.reshape(conv1, [tf.shape(conv1)[0], self.pointnum1_b * self.finaldim])
            mean = linear1(l0, self.vae_meanpara_b, self.hidden_dim, 'mean')
            stddev = linear1(l0, self.vae_stdpara_b, self.hidden_dim, 'stddev')
            stddev = tf.sqrt(2 * tf.nn.sigmoid(stddev))

            return mean, stddev

    def decoder_b(self, latent_tensor, train=True, reuse=False):
        with tf.variable_scope("decoder/b") as scope:
            scope.set_regularizer(tf.contrib.layers.l2_regularizer(scale=1.0))
            if not train or reuse:
                train = False
                reuse = True
                scope.reuse_variables()

            l1 = linear1(latent_tensor, tf.transpose(self.vae_meanpara_b), self.pointnum1_b * self.finaldim, 'mean')

            conv1 = tf.reshape(l1, [tf.shape(l1)[0], self.pointnum1_b, self.finaldim])
            if layer == 1:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n1_b),
                                             tf.transpose(self.vae_e1_b),
                                             self.nb1_b, self.cw1_b, name='conv2', training=train, bn=False,
                                             special_activation=sp)
            if layer == 2:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n2_b),
                                             tf.transpose(self.vae_e2_b),
                                             self.nb1_b, self.cw1_b, name='conv3', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n1_b),
                                             tf.transpose(self.vae_e1_b),
                                             self.nb1_b, self.cw1_b, name='conv4', training=train, bn=False,
                                             special_activation=sp)
            if layer == 3:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n3_b),
                                             tf.transpose(self.vae_e3_b),
                                             self.nb1_b, self.cw1_b, name='conv4', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n2_b),
                                             tf.transpose(self.vae_e2_b),
                                             self.nb1_b, self.cw1_b, name='conv5', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n1_b),
                                             tf.transpose(self.vae_e1_b),
                                             self.nb1_b, self.cw1_b, name='conv6', training=train, bn=False,
                                             special_activation=sp)
            if layer == 4:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n4_b),
                                             tf.transpose(self.vae_e4_b),
                                             self.nb1_b, self.cw1_b, name='conv5', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n3_b),
                                             tf.transpose(self.vae_e3_b),
                                             self.nb1_b, self.cw1_b, name='conv6', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n2_b),
                                             tf.transpose(self.vae_e2_b),
                                             self.nb1_b, self.cw1_b, name='conv7', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n1_b),
                                             tf.transpose(self.vae_e1_b),
                                             self.nb1_b, self.cw1_b, name='conv8', training=train, bn=False,
                                             special_activation=sp)
            if layer == 5:
                print("layer|%d" % (layer))
                conv1 = newconvlayer_pooling(conv1, self.finaldim, self.vertex_dim, tf.transpose(self.vae_n5_b),
                                             tf.transpose(self.vae_e5_b),
                                             self.nb1_b, self.cw1_b, name='conv6', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n4_b),
                                             tf.transpose(self.vae_e4_b),
                                             self.nb1_b, self.cw1_b, name='conv7', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n3_b),
                                             tf.transpose(self.vae_e3_b),
                                             self.nb1_b, self.cw1_b, name='conv8', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n2_b),
                                             tf.transpose(self.vae_e2_b),
                                             self.nb1_b, self.cw1_b, name='conv9', training=train,
                                             special_activation=sp)
                conv1 = newconvlayer_pooling(conv1, self.vertex_dim, self.vertex_dim, tf.transpose(self.vae_n1_b),
                                             tf.transpose(self.vae_e1_b),
                                             self.nb1_b, self.cw1_b, name='conv10', training=train, bn=False,
                                             special_activation=sp)
            conv1 = conv1

        return conv1

     # <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< B  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    def train_pre(self):
        tf.global_variables_initializer().run()
        self.write = tf.summary.FileWriter(logfolder + '/logs/', self.sess.graph)

        if not os.path.exists(self.checkpoint_dir_vae_all):
            os.makedirs(self.checkpoint_dir_vae_all)

        if not os.path.exists(self.checkpoint_dir_vae_a):
            os.makedirs(self.checkpoint_dir_vae_a)

        if not os.path.exists(self.checkpoint_dir_vae_b):
            os.makedirs(self.checkpoint_dir_vae_b)

        if not os.path.exists(self.checkpoint_dir_metric):
            os.makedirs(self.checkpoint_dir_metric)

        if not os.path.exists(self.checkpoint_dir_gan):
            os.makedirs(self.checkpoint_dir_gan)

        could_load_vae, checkpoint_counter_vae = self.load(self.checkpoint_dir_vae_all)
        could_load_metric, checkpoint_counter_metric = self.load(self.checkpoint_dir_metric)
        could_load_gan, checkpoint_counter_gan = self.load(self.checkpoint_dir_gan)

        if not could_load_vae or (could_load_vae and checkpoint_counter_vae < n_epoch_Vae):
            self.start = 'VAE'
            self.start_step_vae = checkpoint_counter_vae
            self.start_step_metric = 0
            self.start_step_gan = 0
        elif not could_load_metric or (
                    could_load_metric and checkpoint_counter_metric < n_epoch_Metric_1 + n_epoch_Metric_2):
            self.start = 'Mat'
            # self.strat_step_vae=checkpoint_counter_metric
            self.start_step_metric = checkpoint_counter_metric
            self.start_step_gan = 0
        elif not could_load_gan or (could_load_gan and checkpoint_counter_gan < n_epoch_Gan):
            self.start = 'GAN'
            self.start_step_gan = checkpoint_counter_gan
        else:
            print('we start from VAE...')

    def model_dir(self, model_name, dataset_name):
        return "{}_{}_{}_{}".format(model_name, dataset_name,
                                    self.batch_size, self.hidden_dim)

    def load(self, checkpoint_dir):
        import re
        print(" [*] Reading checkpoints...")
        # checkpoint_dir = os.path.join(checkpoint_dir, self.model_dir, self.model_name)
        if not checkpoint_dir.find(self.VAE) == -1:
            saver = self.saver_vae_all
        elif not checkpoint_dir.find(self.METRIC) == -1:
            saver = self.saver_metric
        else:
            saver = self.saver_gan

        ckpt = tf.train.get_checkpoint_state(checkpoint_dir)
        if ckpt and ckpt.model_checkpoint_path:
            ckpt_name = os.path.basename(ckpt.model_checkpoint_path)

            saver.restore(self.sess, os.path.join(checkpoint_dir, ckpt_name))
            counter = int(next(re.finditer("(\d+)(?!.*\d)", ckpt_name)).group(0))
            print(" [*] Success to read {}".format(ckpt_name))
            return True, counter
        else:
            print(" [*] Failed to find a checkpoint")
            return False, 0  # model = convMESH()

