// for using Matlab engine
// Note:  matlab uses column first storage

#pragma once
#include <engine.h>


#pragma comment(lib, "libeng.lib")
#pragma comment(lib, "libmat.lib")
#pragma comment(lib, "libmx.lib")
#pragma comment(lib, "libmex.lib")


namespace Utility
{
	class MatEngine
	{
	public:
		MatEngine();
		MatEngine(Engine* eg);
		//
		void SetEngine(Engine* eg);
		Engine* GetEngine();
		// service functions
		void OpenEngine(bool singleuse = false);
		void CloseEngine();

		int EvalString(const char* pathStr, const char* name);
		int EvalString(const char *string);

		mxArray *GetVariable(const char *name);
		int PutVariable(const char *var_name, const mxArray *ap);
		int OutputBuffer(char* buffer, int buflen);

	private:
		Engine* eg;

	};
}