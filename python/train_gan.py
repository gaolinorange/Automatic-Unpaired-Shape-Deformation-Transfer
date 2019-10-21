# coding: utf-8

import pickle,random,os,time
import model as vcgan
import scipy.io as sio
import tensorflow as tf

from utils import *

def train_GAN(_model):
    _model.batch_size = 64
    rng = np.random.RandomState(23456)
    header = '     Time    Epoch     Iteration    Progress(%)   cost_g   cost_mapping   cost_cycle  cost_d_all  cost_g_all'
    log_template = ' '.join('{:>9s},{:>5.0f}/{:<5.0f},{:>5.0f}/{:<5.0f},{:>9.1f}%,{:>11.2f},{:>10.2f},{:>11.2f},{:>10.2f},{:>10.2f}'.split(','))
    start = time.time()
    # print(header)
    global cost_KL
    # IA = np.zeros((_model.batch_size, _model.pointnum1_a, _model.vertex_dim))
    # IB = np.zeros((_model.batch_size, _model.pointnum1_b, _model.vertex_dim))
    # Ilf = np.zeros((_model.batch_size, 1))
    _model.file.write("GAN start\n")
    Gstep = 5
    Dstep = 1

    Ia = _model.Ia
    Ib = _model.Ib
    Ia_C = _model.Ia_C
    Ib_C = _model.Ib_C
    Ia = np.array(Ia)
    Ib = np.array(Ib)

    Ia1 = Ia.repeat(len(Ib))
    Ib1 = np.tile(Ib, len(Ia))
    assert(len(Ia1)==len(Ib1))

    for step in xrange(_model.start_step_gan, vcgan.n_epoch_Gan):
        print(header)
        timeserver1 = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

        # random_a = gaussian(_model.batch_size, _model.hidden_dim)
        # random_b = gaussian(_model.batch_size, _model.hidden_dim)
        random_a_train = gaussian(len(Ia1), _model.hidden_dim)
        random_b_train = gaussian(len(Ib1), _model.hidden_dim)
        rng.shuffle(Ia1)
        rng.shuffle(Ib1)
        # i = 0

        # while i < _model.batch_size:
        #     IDa = (np.random.randint(0, _model.modelnum_a - 1))
        #     IDb = (np.random.randint(0, _model.modelnum_b - 1))
        #     IA[i] = _model.feature_a[IDa]
        #     IB[i] = _model.feature_b[IDb]
        #     Ilf[i][0] = _model.lf_matrix[IDa][IDb]
        #     i = i + 1
        print(len(Ia1))
        for bidx in xrange(0, len(Ia1)//_model.batch_size + 1):

            train_id_a = [i for i in Ia1[(bidx*_model.batch_size)%len(Ia1):min(len(Ia1), (bidx*_model.batch_size)%len(Ia1)+_model.batch_size)]]
            train_id_b = [i for i in Ib1[(bidx*_model.batch_size)%len(Ib1):min(len(Ib1), (bidx*_model.batch_size)%len(Ib1)+_model.batch_size)]]
            IA = _model.feature_a[train_id_a]
            IB = _model.feature_b[train_id_b]
            random_a = random_a_train[train_id_a]
            random_b = random_b_train[train_id_b]
            Ilf = _model.lf_matrix[train_id_a, train_id_b]
            Ilf = np.expand_dims(Ilf, 1)
            if len(Ilf) == 0:
                continue

        # ------------------GAN
            for G in range(Gstep):
                _, cost_mapping, cost_cycle, cost_g, cost_g_all = _model.sess.run(
                    [_model.train_op_g, _model.loss_mapping, _model.loss_cycle, _model.loss_g, _model.loss_g_all],
                    feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a, _model.random_b: random_b})
            # printout(_model.file, "Gen|step: [%2d]cost_g: %.8f, cost_mapping: %.8f, cost_KL: %.8f, cost_cycle: %.8f" % (
                # step + 1, cost_g, cost_mapping, cost_KL, cost_cycle))
            for D in range(Dstep):
                # _model.sess.run(_model.clip_D)
                _, cost_d_all, cost_mapping, cost_cycle, cost_g = _model.sess.run(
                    [_model.train_op_d, _model.loss_d_all, _model.loss_mapping, _model.loss_cycle, _model.loss_g],
                    feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a, _model.random_b: random_b})
            print(log_template.format(time.strftime("%H:%M:%S",time.gmtime(time.time()-start)),
            step + 1, vcgan.n_epoch_Gan, bidx, len(Ia1),
            100. * (bidx+len(Ia1)*step) / (len(Ia1)*vcgan.n_epoch_Gan),
            cost_g, cost_mapping, cost_cycle, cost_d_all, cost_g_all))

        _model.file_gan.write("G %d %.8f %.8f %.8f %.8f %.8f" % (step + 1, cost_mapping, cost_cycle, cost_g, cost_d_all, cost_g_all))

        if vcgan.tb and (step + 1) % 20 == 0:
            s = _model.sess.run(_model.merge_summary,
                              feed_dict={_model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a,
                                         _model.random_b: random_b, _model.lf_dis: Ilf})
            _model.write.add_summary(s, step)

        if (step + 1) % 1 == 0:
            print(vcgan.logfolder)
            if test_gan:
                test_gan(_model, step)
            _model.saver_gan.save(_model.sess, _model.checkpoint_dir + '/GAn.model', global_step=step + 1)
            # latent_z_a ,latent_z_b =sess.run([_model.guessed_z_a,_model.guessed_z_b], feed_dict = { _model.inputs_a: IA, _model.inputs_b: IB, _model.random_a: random_a, _model.random_b: random_b})
            # sio.savemat('./latent_z/latent_z_a'+str(step)+'.mat', {'latent_z': latent_z_a})
            # sio.savemat('./latent_z/latent_z_b'+str(step)+'.mat', {'latent_z': latent_z_b})

    print('---------------------------------train cycleGAN success!!----------------------------------\nFinished!')


def test_gan(_model, step):
    logfolder = vcgan.logfolder
    if not os.path.isdir(logfolder + '/test_gan'):
        os.mkdir(logfolder + '/test_gan')

    random_a = gaussian(_model.batch_size, _model.hidden_dim)
    random_b = gaussian(_model.batch_size, _model.hidden_dim)
    a_gen_b, b_gen_a = _model.sess.run([_model.gen_feature_a, _model.gen_feature_b],
                                     feed_dict={_model.inputs_a: _model.feature_a, _model.inputs_b: _model.feature_b,
                                                _model.random_a: random_a, _model.random_b: random_b})
    IA = recover_data(_model.feature_a, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin,
                      vcgan.resultmax)
    IB = recover_data(_model.feature_b, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin,
                      vcgan.resultmax)
    a_gen_b = recover_data(a_gen_b, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    b_gen_a = recover_data(b_gen_a, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)

    name = logfolder + '/test_gan/' + 'a_gen_b' + str(step) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = a_gen_b
    f.close()
    name = logfolder + '/test_gan/' + 'b_gen_a' + str(step) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = b_gen_a
    f.close()

    name = logfolder + '/test_gan/' + 'IA' + str(step) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = IA
    f.close()
    name = logfolder + '/test_gan/' + 'IB' + str(step) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = IB
    f.close()
