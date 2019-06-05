#ifndef new_ARAP_DEFORM_H
#define new_ARAP_DEFORM_H


#include "MatEngine.h"
#include "fileSystemUtility.h"
#include "FeatureVector.h"

#define tic long long timetick = clock();
#define tocv clock()-timetick
#define tocp std::cout<<coutcmd::blue<<tocv<<coutcmd::white<<std::endl;

class new_ARAPDeform {
public:
	Utility::MatEngine *eng;
	RefMesh ref;
	std::vector<DTriMesh*> meshs;

	std::vector<FeatureVector> fvs;

	new_ARAPDeform() {}

	new_ARAPDeform(Utility::MatEngine &eng, DTriMesh &refMesh, std::vector<DTriMesh*> ms, bool new_feature);

	~new_ARAPDeform();

	void bfscorrot(FeatureVector & fv);
	void writeIterMesh(DTriMesh &mesh, std::string name, int id);

	void GetFeatureMat2(std::string & currentpath, std::string & inputfolder, std::string outputfolder);

};
std::vector<std::pair<int, std::pair<int, Eigen::Vector3d>>> operator *(const std::vector<Rot> &a, const std::vector<std::pair<int, std::pair<int, Eigen::Vector3d>>> &b);



#endif
