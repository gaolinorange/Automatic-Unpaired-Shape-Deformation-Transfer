# coding: utf-8

import tensorflow as tf
import model as vcgan
import random, pickle, time, model

from utils import *
from test_utils import *


def train_metric_1(_model):
    _model.batch_size = 512
    IA = np.zeros((_model.batch_size, _model.pointnum1_a, _model.vertex_dim))
    IB = np.zeros((_model.batch_size, _model.pointnum1_b, _model.vertex_dim))
    Ilf = np.zeros((_model.batch_size, 1))
    _model.file.write("Metric start step one\n")

    for step in xrange(_model.start_step_metric, model.n_epoch_Metric_1):
        timeserver1 = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

        random_a = gaussian(_model.batch_size, _model.hidden_dim)
        random_b = gaussian(_model.batch_size, _model.hidden_dim)
        i = 0
        while i < _model.batch_size:
            IDa = (np.random.randint(0, _model.modelnum_a - 1))
            IDb = (np.random.randint(0, _model.modelnum_b - 1))
            IA[i] = _model.feature_a[IDa]
            IB[i] = _model.feature_b[IDb]
            if _model.metric_lz_a.any() and _model.metric_lz_b.any():
                random_a[i] = _model.metric_lz_a[IDa]
                random_b[i] = _model.metric_lz_b[IDb]
            Ilf[i][0] = _model.lf_matrix[int(IDa)][int(IDb)]
            i = i + 1

        # ------------------------------------metric
        _, cost_metric, cost_metric_l2, t_lf, t_dis = _model.sess.run(
            [_model.train_op_metric_1, _model.loss_metric, _model.loss_metric_l2, _model.lf_dis, _model.distance],
            feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a,
                       _model.random_b: random_b, _model.lf_dis: Ilf})
        print("|%s step: [%2d]cost_metric: %.8f,cost_metric_l2: %.8f" % (
            timeserver1, step + 1, cost_metric, cost_metric_l2))
        print("gt:%.8f test:%.8f error:%.8f " % (t_lf[0], t_dis[0], np.max(abs(t_lf - t_dis))))

        _model.file.write("|%s step: [%2d|%5d]cost_metric: %.8f cost_metric_l2: %.8f\n" \
                        % (timeserver1, step + 1, vcgan.n_epoch_Metric_1, cost_metric, cost_metric_l2))
        _model.file_metric.write("M %d %.8f %.8f\n" % (step + 1, cost_metric, cost_metric_l2))

        if vcgan.tb and (step + 1) % 20 == 0:
            s = _model.sess.run(_model.merge_summary,
                              feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a,
                                         _model.random_b: random_b, _model.lf_dis: Ilf})
            _model.write.add_summary(s, step)

        if (step + 1) % 2000 == 0:
            print(vcgan.logfolder)
            test_metric(_model, step + 1)
            _model.saver_metric.save(_model.sess, _model.checkpoint_dir_metric + '/metric.model', global_step=step + 1)
            # latent_z_a ,latent_z_b =sess.run([_model.guessed_z_a,_model.guessed_z_b], feed_dict = { _model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a, _model.random_b: random_b})
            # sio.savemat('./latent_z/latent_z_a'+str(step)+'.mat', {'latent_z': latent_z_a})
            # sio.savemat('./latent_z/latent_z_b'+str(step)+'.mat', {'latent_z': latent_z_b})

    # _model.save(sess, _model.checkpoint_dir, n_iteration)
    print('---------------------------------train Metric_1 success!!----------------------------------')
    print('------------------------------------train Metric_2---------------------------------------')


def train_metric_2(_model):
    _model.batch_size = 64
    IA = np.zeros((_model.batch_size, _model.pointnum1_a, _model.vertex_dim))
    IB = np.zeros((_model.batch_size, _model.pointnum1_b, _model.vertex_dim))
    Ilf = np.zeros((_model.batch_size, 1))
    _model.file.write("Metric start step two\n")
    for step in xrange(_model.start_step_metric, vcgan.n_epoch_Metric_1 + vcgan.n_epoch_Metric_2):
        timeserver1 = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

        ib = random.sample(list(np.arange(len(_model.feature_b))), _model.batch_size)
        ia = random.sample(list(np.arange(len(_model.feature_a))), _model.batch_size)
        random_a = gaussian(_model.batch_size, _model.hidden_dim)
        random_b = gaussian(_model.batch_size, _model.hidden_dim)
        i = 0

        while i < _model.batch_size:
            IA[i] = _model.feature_a[ia[i]]
            IB[i] = _model.feature_b[ib[i]]
            if _model.metric_lz_a.any() and _model.metric_lz_b.any():
                random_a[i] = _model.metric_lz_a[ia[i]]
                random_b[i] = _model.metric_lz_b[ib[i]]
            Ilf[i][0] = _model.lf_matrix[ia[i]][ib[i]]
            i = i + 1

        # ------------------------------------metric
        _, cost_metric, cost_metric_l2, t_lf, t_dis = _model.sess.run(
            [_model.train_op_metric_2, _model.loss_metric, _model.loss_metric_l2, _model.lf_dis, _model.distance],
            feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a,
                       _model.random_b: random_b, _model.lf_dis: Ilf})
        print("|%s step: [%2d]cost_metric: %.8f,cost_metric_l2: %.8f" % (
            timeserver1, step + 1, cost_metric, cost_metric_l2))
        print("gt:%.8f test:%.8f error:%.8f " % (t_lf[0], t_dis[0], np.max(abs(t_lf - t_dis))))

        _model.file.write("|%s step: [%2d|%5d]cost_metric: %.8f cost_metric_l2: %.8f\n" \
                        % (timeserver1, step + 1, vcgan.n_epoch_Metric_2 + vcgan.n_epoch_Metric_1, cost_metric, cost_metric_l2))
        _model.file_metric.write("M %d %.8f %.8f\n" % (step + 1, cost_metric, cost_metric_l2))

        if vcgan.tb and (step + 1) % 20 == 0:
            s = _model.sess.run(_model.merge_summary,
                              feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a,
                                         _model.random_b: random_b, _model.lf_dis: Ilf})
            _model.write.add_summary(s, step)

        if (step + 1) % 2000 == 0:
            print(vcgan.logfolder)
            test_metric(_model, step + 1)
            _model.saver_metric.save(_model.sess, _model.checkpoint_dir_metric + '/metric.model', global_step=step + 1)
            # latent_z_a ,latent_z_b =sess.run([_model.guessed_z_a,_model.guessed_z_b], feed_dict = { _model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a, _model.random_b: random_b})
            # sio.savemat('./latent_z/latent_z_a'+str(step)+'.mat', {'latent_z': latent_z_a})
            # sio.savemat('./latent_z/latent_z_b'+str(step)+'.mat', {'latent_z': latent_z_b})

    # _model.save(sess, _model.checkpoint_dir, n_iteration)
    print('---------------------------------train Metric_2 success!!----------------------------------')
    print('------------------------------------train cycleGAN-----------------------------------------')


def train_metric(_model):
    if _model.start_step_metric < vcgan.n_epoch_Metric_1:
        train_metric_1(_model)
        train_metric_2(_model)
    else:
        train_metric_2(_model)



