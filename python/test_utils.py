import model as vcgan
import os

import scipy.interpolate as interpolate

from utils import *


def test_vae(_model, step):
    if not os.path.isdir(vcgan.logfolder + '/test_vae'):
        os.makedirs(vcgan.logfolder + '/test_vae')

    zeros1a = np.zeros((_model.model_num_a, vcgan.hidden_dim)).astype('float32')
    zeros1b = np.zeros((_model.model_num_b, vcgan.hidden_dim)).astype('float32')
    z_meana, z_stddeva, recon_mesha, z_meanb, z_stddevb, recon_meshb = _model.sess.run(
        [_model.z_mean_a, _model.z_stddev_a, _model.generated_mesh_test_a,
         _model.z_mean_b, _model.z_stddev_b, _model.generated_mesh_test_b],
        feed_dict={_model.inputs_a: _model.feature_a, _model.random_a: zeros1a, _model.inputs_b: _model.feature_b,
                   _model.random_b: zeros1b})

    recon_fva = recover_data(recon_mesha, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin,
                             vcgan.resultmax)
    recon_fvb = recover_data(recon_meshb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin,
                             vcgan.resultmax)

    name = vcgan.logfolder + '/test_vae/' + 'recon_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['latent_mean'] = z_meana
    f['latent_std'] = z_stddeva
    f['feature'] = _model.feature_a
    f['test_mesh'] = recon_fva
    f.close()

    name = vcgan.logfolder + '/test_vae/' + 'recon_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['latent_mean'] = z_meanb
    f['latent_std'] = z_stddevb
    f['feature'] = _model.feature_b
    f['test_mesh'] = recon_fvb
    f.close()

    # sio.savemat(logfolder + '/test_vae/' + 'recon_a' + str(step + 1) + '.mat', {'latent_mean': z_meana,
    #                                                                           'latent_std': z_stddeva,
    #                                                                          'feature': _model.feature_a,
    #                                                                         'recon_fv': recon_fva})

    # sio.savemat(logfolder + '/test_vae/' + 'recon_b' + str(step + 1) + '.mat', {'latent_mean': z_meanb,
    #                                                                            'latent_std': z_stddevb,
    #                                                                            'feature': _model.feature_b,
    #                                                                            'recon_fv': recon_fvb})

    random_batch_a = np.random.normal(loc=0.0, scale=1.0, size=(50, _model.hidden_dim))
    random_batch_b = np.random.normal(loc=0.0, scale=1.0, size=(50, _model.hidden_dim))
    testa, testb = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b],
                                 feed_dict={_model.random_a: random_batch_a, _model.random_b: random_batch_b})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    name = vcgan.logfolder + '/test_vae/' + 'random_gen_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1a
    f['latent_z'] = random_batch_a
    f.close()

    name = vcgan.logfolder + '/test_vae/' + 'random_gen_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1b
    f['latent_z'] = random_batch_b
    f.close()


def test_vae_itlp(_model, step):
    if not os.path.isdir(vcgan.logfolder + '/test_vae_itlp'):
        os.makedirs(vcgan.logfolder + '/test_vae_itlp')
    # intepolation in the latent space
    random_two_lz = np.random.normal(loc=0.0, scale=1.0, size=(2, _model.hidden_dim))
    random2_intpl = interpolate.griddata(np.linspace(0, 1, len(random_two_lz)), random_two_lz,
                                         np.linspace(0, 1, len(random_two_lz) * 10), method='linear')
    testa, testb = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b],
                                 feed_dict={_model.random_a: random2_intpl, _model.random_b: random2_intpl})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    name = vcgan.logfolder + '/test_vae_itlp/' + 'random_intpl_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1a
    f['latent_z'] = random2_intpl
    f.close()

    name = vcgan.logfolder + '/test_vae_itlp/' + 'random_intpl_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1b
    f['latent_z'] = random2_intpl
    f.close()

    # intepolate data from the known data
    IDa_start = (np.random.randint(0, _model.modelnum_a - 1))
    IDa_end = (np.random.randint(0, _model.modelnum_a - 1))
    ida = [IDa_start, IDa_end]
    IDb_start = (np.random.randint(0, _model.modelnum_b - 1))
    IDb_end = (np.random.randint(0, _model.modelnum_b - 1))
    idb = [IDb_start, IDb_end]
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
    name = vcgan.logfolder + '/test_vae_itlp/' + 'random_intpl_knowdata_a' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1a
    f['latent_z'] = random2_intpl_a
    f.close()

    name = vcgan.logfolder + '/test_vae_itlp/' + 'random_intpl_knowdata_b' + str(step + 1) + '.h5'
    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1b
    f['latent_z'] = random2_intpl_b
    f.close()


def test_metric(_model, step):
    if not os.path.isdir(vcgan.logfolder + '/test_metric'):
        os.mkdir(vcgan.logfolder + '/test_metric')
    random_batch_a = np.random.normal(loc=0.0, scale=1.0, size=(50, _model.hidden_dim))
    random_batch_b = np.random.normal(loc=0.0, scale=1.0, size=(50, _model.hidden_dim))
    testa, testb, test_dis = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b, _model.distance_test],
                                           feed_dict={_model.random_a: random_batch_a, _model.random_b: random_batch_b})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    test_dis = recover_data_old(test_dis, _model.lf_matrix_min, _model.lf_matrix_max, 0.05, 2.0)
    # test_dis = recover_lfd(test_dis, _model.lf_matrix_mean,_model.lf_matrix_std)

    name = vcgan.logfolder + '/test_metric/testmetric_random' + str(step) + '.h5'
    f = h5py.File(name, 'w')
    f['test_dis'] = test_dis
    f['test_mesh_a'] = fv1a
    f['test_mesh_b'] = fv1b
    f.close()

    za, zb = _model.sess.run([_model.z_mean_a, _model.z_mean_b],
                           feed_dict={_model.inputs_a: _model.feature_a, _model.inputs_b: _model.feature_b})
    id = np.min([np.shape(za)[0], np.shape(zb)[0]])
    testa, testb, test_dis = _model.sess.run([_model.test_mesh_a, _model.test_mesh_b, _model.distance_test],
                                           feed_dict={_model.random_a: za[0:id], _model.random_b: zb[0:id]})
    fv1b = recover_data(testb, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax)
    fv1a = recover_data(testa, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax)
    test_dis = recover_data_old(test_dis, _model.lf_matrix_min, _model.lf_matrix_max, 0.05, 2.0)
    # test_dis = recover_lfd(test_dis, _model.lf_matrix_mean,_model.lf_matrix_std)
    name = vcgan.logfolder + '/test_metric/testmetric_recon' + str(step) + '.h5'
    f = h5py.File(name, 'w')
    f['test_dis'] = test_dis
    f['test_mesh_a'] = fv1a
    f['test_mesh_b'] = fv1b
    f.close()


def recons_error_a(_model):
    zeros1 = np.zeros((_model.model_num_a, vcgan.hidden_dim)).astype('float32')
    mean, recover_mesh = _model.sess.run([_model.z_mean_test_a, _model.generated_mesh_test_a],
                                       feed_dict={_model.inputs_a: _model.feature_a, _model.random_a: zeros1})

    feature1 = np.concatenate((_model.feature_a, recover_mesh), axis=0)
    fv1 = recover_data(feature1, _model.logrmin_a, _model.logrmax_a, _model.smin_a, _model.smax_a, vcgan.resultmin, vcgan.resultmax,
                       useS=vcgan.useS)

    if not os.path.isdir(vcgan.logfolder + '/_recon'):
        os.mkdir(vcgan.logfolder + '/_recon')
    name = vcgan.logfolder + '/_recon/' + 'recover' + '_a' + '.h5'

    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1
    f['latent_z'] = mean
    f['input_mesh'] = _model.feature_a
    f.close()


def recons_error_b(_model):
    zeros1 = np.zeros((_model.model_num_b, vcgan.hidden_dim)).astype('float32')
    mean, recover_mesh = _model.sess.run([_model.z_mean_test_b, _model.generated_mesh_test_b],
                                       feed_dict={_model.inputs_b: _model.feature_b, _model.random_b: zeros1})

    feature1 = np.concatenate((_model.feature_b, recover_mesh), axis=0)
    fv1 = recover_data(feature1, _model.logrmin_b, _model.logrmax_b, _model.smin_b, _model.smax_b, vcgan.resultmin, vcgan.resultmax,
                       useS=vcgan.useS)

    if not os.path.isdir(vcgan.logfolder + '/_recon'):
        os.mkdir(vcgan.logfolder + '/_recon')
    name = vcgan.logfolder + '/_recon/' + 'recover' + '_b' + '.h5'

    f = h5py.File(name, 'w')
    f['test_mesh'] = fv1
    f['latent_z'] = mean
    f['input_mesh'] = _model.feature_b
    f.close()

    