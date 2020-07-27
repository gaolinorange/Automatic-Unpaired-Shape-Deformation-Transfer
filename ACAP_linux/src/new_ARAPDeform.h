#ifndef new_ARAP_DEFORM_H
#define new_ARAP_DEFORM_H


#include "fileSystemUtility.h"
#include "FeatureVector.h"

#define tic long long timetick = clock();
#define tocv clock()-timetick
#define tocp std::cout<<coutcmd::blue<<tocv<<coutcmd::white<<std::endl;

class new_ARAPDeform {
public:
	RefMesh ref;
	std::vector<DTriMesh*> meshs;

	std::vector<FeatureVector> fvs;

	new_ARAPDeform() {}

	new_ARAPDeform(DTriMesh &refMesh, std::vector<DTriMesh*> ms, bool new_feature);

	~new_ARAPDeform();

	void bfscorrot(FeatureVector & fv);
	void writeIterMesh(DTriMesh &mesh, std::string name, int id);

	void GetFeatureMat2(std::string & currentpath, std::string & inputfolder, std::string outputfolder, bool opt = true);

};



#endif
