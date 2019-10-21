# coding: utf-8

import pickle,random,os,time
import model as vcgan
import scipy.io as sio
import tensorflow as tf

from utils import *



def train_VAE(_model):
    Ilf = np.zeros((_model.batch_size, 1))
    rng = np.random.RandomState(23456)

    Ia = _model.Ia
    Ib = _model.Ib
    _model.file.write("VAE start\n")
    for step in xrange(_model.start_step_vae, vcgan.n_epoch_Vae):
        timeserver1 = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(time.time()))

        rng.shuffle(Ia)
        rng.shuffle(Ib)
        feature_a = _model.feature_a[Ia]
        feature_b = _model.feature_b[Ib]
        random_a = gaussian(len(feature_a), _model.hidden_dim)
        random_b = gaussian(len(feature_b), _model.hidden_dim)

        # ------------------------------------VAE a
        _, cost_generation_a, cost_latent_a, l2_loss_a = _model.sess.run(
            [_model.train_op_vae_a, _model.neg_loglikelihood_a, _model.KL_divergence_a, _model.r2_a],
            feed_dict={_model.inputs_a: feature_a, _model.random_a: random_a})
        print("|%s step: [%2d|%d]cost_generation_a: %.8f, cost_latent_a: %.8f, l2_loss_a: %.8f" % (
            timeserver1, step + 1, vcgan.n_epoch_Vae, cost_generation_a, cost_latent_a, l2_loss_a))
        # ------------------------------------VAE b
        _, cost_generation_b, cost_latent_b, l2_loss_b = _model.sess.run(
            [_model.train_op_vae_b, _model.neg_loglikelihood_b, _model.KL_divergence_b, _model.r2_b],
            feed_dict={_model.inputs_b: feature_b, _model.random_b: random_b})
        print("|%s step: [%2d|%d]cost_generation_b: %.8f, cost_latent_b: %.8f, l2_loss_b: %.8f" % (
            timeserver1, step + 1, vcgan.n_epoch_Vae, cost_generation_b, cost_latent_b, l2_loss_b))

        _model.file.write("|%s Epoch: [%5d|%d] cost_generation_a: %.8f, cost_latent_a: %.8f, l2_loss_a: %.8f\n" \
                        % (timeserver1, step + 1, vcgan.n_epoch_Vae, cost_generation_a, cost_latent_a, l2_loss_a))

        _model.file.write("|%s Epoch: [%5d|%d] cost_generation_b: %.8f, cost_latent_b: %.8f, l2_loss_b: %.8f\n" \
                        % (timeserver1, step + 1, vcgan.n_epoch_Vae, cost_generation_b, cost_latent_b, l2_loss_b))

        _model.file_vae.write("A %d %.8f %.8f %.8f\n" % (step + 1, cost_generation_a, cost_latent_a, l2_loss_a))
        _model.file_vae.write("B %d %.8f %.8f %.8f\n" % (step + 1, cost_generation_b, cost_latent_b, l2_loss_b))

        if vcgan.tb and (step + 1) % 20 == 0:
            s = _model.sess.run(_model.merge_summary,
                              feed_dict={_model.inputs_a: feature_a, _model.inputs_b: feature_b,
                                         _model.random_a: random_a,
                                         _model.random_b: random_b, _model.lf_dis: Ilf})
            _model.write.add_summary(s, step)

        if (step + 1) % 1 == 0:
            print(vcgan.logfolder)
            if vcgan.test_vae:
                test_vae(_model, step)

            # self.saver_vae_a.save(self.sess, self.checkpoint_dir_vae_a + '/vae_a.model', global_step=step + 1)
            # self.saver_vae_b.save(self.sess, self.checkpoint_dir_vae_b + '/vae_b.model', global_step=step + 1)
            _model.saver_vae_all.save(_model.sess, _model.checkpoint_dir_vae_all + '/vae_all.model', global_step=step + 1)
    print('---------------------------------train VAE success!!----------------------------------')
    print('------------------------------------train Metric--------------------------------------')
    # self.save(sess, self.checkpoint_dir, n_iteration)

def test_vae(_model, step):
    logfolder = vcgan.logfolder
    if not os.path.isdir(logfolder + '/test_vae'):
        os.makedirs(logfolder + '/test_vae')

    zeros1a = np.zeros((_model.model_num_a, _model.hidden_dim)).astype('float32')
    zeros1b = np.zeros((_model.model_num_b, _model.hidden_dim)).astype('float32')
    #zeros1a = np.zeros((len(_model.C_Ia), hidden_dim)).astype('float32')
    #zeros1b = np.zeros((len(_model.C_Ib), hidden_dim)).astype('float32')
    z_meana, z_stddeva, recon_mesha, z_meanb, z_stddevb, recon_meshb = _model.sess.run(
        [_model.z_mean_a, _model.z_stddev_a, _model.generated_mesh_test_a,
         _model.z_mean_b, _model.z_stddev_b, _model.generated_mesh_test_b],
        feed_dict={_model.inputs_a: _model.feature_a, _model.random_a: zeros1a, _model.inputs_b: _model.feature_b,
                   _model.random_b: zeros1b})

    recon_fva = recover_data(recon_mesha, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin,
                             vcgan.resultmax)
    recon_fvb = recover_data(recon_meshb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin,
                             vcgan.resultmax)

    name = logfolder + '/test_vae/' + 'recon_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['latent_mean'] = z_meana
    f['latent_std'] = z_stddeva
    f['feature'] = _model.feature_a
    f['test_mesh'] = recon_fva
    f['cia']=_model.Ia_C
    f.close()

    name = logfolder + '/test_vae/' + 'recon_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['latent_mean'] = z_meanb
    f['latent_std'] = z_stddevb
    f['feature'] = _model.feature_b
    f['test_mesh'] = recon_fvb
    f['cib']=_model.Ib_C
    f.close()

    random_batch_a = np.random.normal(loc=0.0, scale=1.0, size=(50, _model.hidden_dim))
    random_batch_b = np.random.normal(loc=0.0, scale=1.0, size=(50, _model.hidden_dim))
    testa, testb = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b],
                                 feed_dict={_model.random_a: random_batch_a, _model.random_b: random_batch_b})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    name = logfolder + '/test_vae/' + 'random_gen_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1a
    f['latent_z'] = random_batch_a
    f.close()

    name = logfolder + '/test_vae/' + 'random_gen_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1b
    f['latent_z'] = random_batch_b
    f.close()

def test_vae_itlp(_model, step):
    logfolder = vcgan.logfolder
    if not os.path.isdir(logfolder + '/test_vae_itlp'):
        os.makedirs(logfolder + '/test_vae_itlp')
    # intepolation in the latent space
    random_two_lz = np.random.normal(loc=0.0, scale=1.0, size=(2, _model.hidden_dim))
    random2_intpl = interpolate.griddata(np.linspace(0, 1, len(random_two_lz)), random_two_lz,
                                         np.linspace(0, 1, len(random_two_lz) * 10), method='linear')
    testa, testb = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b],
                                feed_dict={_model.random_a: random2_intpl, _model.random_b: random2_intpl})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    name = logfolder + '/test_vae_itlp/' + 'random_intpl_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1a
    f['latent_z'] = random2_intpl
    f.close()

    name = logfolder + '/test_vae_itlp/' + 'random_intpl_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1b
    f['latent_z'] = random2_intpl
    f.close()

    # intepolate data from the known data
    IDa_start = (np.random.randint(0, _model.modelnum_a - 1))
    IDa_end = (np.random.randint(0, _model.modelnum_a - 1))
    ida=[IDa_start,IDa_end]
    IDb_start = (np.random.randint(0, _model.modelnum_b - 1))
    IDb_end = (np.random.randint(0, _model.modelnum_b - 1))
    idb=[IDb_start,IDb_end]
    dataA = _model.feature_a[ida]
    dataB = _model.feature_b[idb]
    za, zb = _model.sess.run([_model.z_mean_a, _model.z_mean_b],
                                 feed_dict={_model.inputs_a: dataA, _model.inputs_b: dataB})

    random2_intpl_a = interpolate.griddata(np.linspace(0, 1, len(za)), za,
                                           np.linspace(0, 1, len(za) * 10), method='linear')
    random2_intpl_b = interpolate.griddata(np.linspace(0, 1, len(zb)), zb,
                                           np.linspace(0, 1, len(zb) * 10), method='linear')

    testa, testb = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b],
                                feed_dict={_model.random_a: random2_intpl_a, _model.random_b: random2_intpl_b})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    name = logfolder + '/test_vae_itlp/' + 'random_intpl_knowdata_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1a
    f['latent_z'] = random2_intpl_a
    f.close()

    name = logfolder + '/test_vae_itlp/' + 'random_intpl_knowdata_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1b
    f['latent_z'] = random2_intpl_b
    f.close()



