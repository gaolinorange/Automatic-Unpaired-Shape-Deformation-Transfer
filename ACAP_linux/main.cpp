#include <string>
#include <unistd.h>
#include "new_ARAPDeform.h"
#include <random>
#include <gurobi_c++.h>
#include "fileSystemUtility.h"


void PreprocessObj2Mat(std::string & inputfolder, bool opt)
{
    // check_gurobi_available
	GRBEnv *env = new GRBEnv();

	GRBModel model = GRBModel(*env);
	int time_gurobi = model.get(GRB_IntAttr_LicenseExpiration);
	std::cout<<"GUROBI Expiration DATE: ";
	env->message(std::to_string(time_gurobi).c_str());
	std::cout<<std::endl;
	//delete env;

	//main
	OpenMesh::IO::_OBJReader_();
    OpenMesh::IO::_OBJWriter_();
	FileSystemUtility fileSystem;
	char* buffer;
	if ((buffer = getcwd(nullptr, 0)) == nullptr)
	{
		perror("_getcwd error");
	}
	std::string currentFolder("cd ");
	currentFolder += buffer;
	std::cout << "Current Folder: " << buffer << std::endl;
	//currentFolder += "\\..\\..\\script";
	currentFolder = FileSystemUtility::yj_replace_all_distinct(currentFolder, "\\", "/");
	//std::cout <<"Current Folder: "<< buffer << std::endl;

	std::string outputfolder = inputfolder;
	//if (fileSystem.folderExist(outputfolder))
	//mkdir(outputfolder.c_str());
	std::vector<std::string> meshlist;
	fileSystem.getAllFileRec(inputfolder + "/", std::string("obj"), meshlist);
	fileSystem.sortFileName(meshlist);
	// meshlist.erase();
	std::vector<DTriMesh*> meshes;
	meshes.resize(meshlist.size());
	//openmp
	for (int i = 0; i < meshlist.size(); i++)
	{
		DTriMesh *Meshori = new DTriMesh();
		if (!OpenMesh::IO::read_mesh(*Meshori, (meshlist[i]).c_str()))
		{
			std::cout << "Load reference Mesh Error" << std::endl;
			return;
		}
		if (!Meshori[0].has_vertex_normals())
		{
			Meshori[0].request_vertex_normals();
			Meshori[0].request_face_normals();
			Meshori[0].update_normals();
			Meshori[0].release_face_normals(); // let the mesh update the normalss
		}
		meshes[i] = Meshori;
		std::cout << "load " << meshlist[i] << " success!!" << std::endl;
	}
	new_ARAPDeform *arapDeform;
	arapDeform = new new_ARAPDeform(meshes[0][0], meshes, true);
	arapDeform->GetFeatureMat2(currentFolder, inputfolder, outputfolder, opt);

}

int main(int argc, char *argv[])
{
	int opt = 0;
	if (argc <= 3)
    {
        std::cout << "./main path_to_folder -opt 1or0 \n";
        return 0;
    }

	if (argc > 3 && strcmp(argv[2], "-opt") == 0) {
		sscanf(argv[3], "%d", &opt);
	}

    printf("./main %s -opt %s\n", argv[1], argv[3]);

	std::string inputfolder = argv[1];
	// bool opt = bool(atoi(argv[2]));
	PreprocessObj2Mat(inputfolder, bool(opt));

	return 0;
}