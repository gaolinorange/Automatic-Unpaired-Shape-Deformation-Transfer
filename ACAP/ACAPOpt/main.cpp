#include <string>
#include <direct.h>
#include "new_ARAPDeform.h"
#include <random>
#include <gurobi_c++.h>
#include "fileSystemUtility.h"
#include "MatEngine.h"



void PreprocessObj2Mat(std::string & inputfolder)
{
	Utility::MatEngine eng;// change to matlab script directory:
	eng.OpenEngine(false);
	FileSystemUtility fileSystem;
	char* buffer;
	if ((buffer = _getcwd(nullptr, 0)) == nullptr)
	{
		perror("_getcwd error");
	}
	std::string currentFolder("cd ");
	currentFolder += buffer;
	std::cout << "Current Folder: " << buffer << std::endl;
	//currentFolder += "\\..\\..\\script";
	currentFolder = FileSystemUtility::yj_replace_all_distinct(currentFolder, "\\", "/");
	eng.EvalString(currentFolder.c_str());
	//std::cout <<"Current Folder: "<< buffer << std::endl;

	//std::string inputfolder = "F:\\yangjiee\\sig17\\angleaxis\\1111";
	//std::string outputfolder = inputfolder + "\\feature";
	std::string outputfolder = inputfolder;
	//if (fileSystem.folderExist(outputfolder))
	//mkdir(outputfolder.c_str());
	std::vector<std::string> meshlist;
	fileSystem.getAllFileRec(inputfolder + "\\", ".obj", meshlist);
	//yj_getAllFile(inputfolder + "\\", meshlist);
	fileSystem.sortFileName(meshlist);
	// meshlist.erase();

	std::vector<DTriMesh*> meshes;
	meshes.resize(meshlist.size());
	//openmp
	for (int i = 0; i < meshlist.size(); i++)
	{
		DTriMesh *Meshori = new DTriMesh();
		if (!OpenMesh::IO::read_mesh(*Meshori, (inputfolder + "\\" + meshlist[i]).c_str()))
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
	arapDeform = new new_ARAPDeform(eng, meshes[0][0], meshes, true);
	arapDeform->GetFeatureMat2(currentFolder, inputfolder, outputfolder);
	eng.CloseEngine();

}

int main(int argc, char *argv[])
{
	std::string inputfolder = argv[1];
	PreprocessObj2Mat(inputfolder);

	return 0;
}