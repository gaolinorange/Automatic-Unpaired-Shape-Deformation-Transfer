#include "mex.h"
#if (_MSC_VER >= 1600)
#define __STDC_UTF_16__
#endif


#include "MatEngine.h"
#include <ostream>
#include <iostream>

namespace Utility
{
	MatEngine::MatEngine()
	{
		this->eg = NULL;
	}

	MatEngine::MatEngine(Engine* eg)
	{
		this->eg = eg;
	}

	void MatEngine::SetEngine(Engine* eg)
	{
		this->eg = eg;
	}

	void MatEngine::OpenEngine(bool singleuse)
	{
		if (eg == NULL)
		{
			if (singleuse)
			{
				eg = engOpen("\0");
			}
			else
			{
				int statusmat = 0;
				eg = engOpenSingleUse(NULL, NULL, &statusmat);
			}
		}
		if (!eg) {
			std::cerr << "\nCan't start MATLAB engine\n" << std::endl;
		}
	}

	void MatEngine::CloseEngine()
	{
		if (eg != NULL)
		{
			engClose(eg);
		}
	}

	Engine* MatEngine::GetEngine()
	{
		return eg;
	}

	int MatEngine::EvalString(const char* pathStr, const char* name)
	{
		// Change working directory
		mxArray* wkDir = mxCreateString(pathStr);
		engPutVariable(eg, "pathStr__", wkDir);
		engEvalString(eg, "cd(pathStr__)");
		return engEvalString(eg, name);
	}
	int MatEngine::EvalString(const char *string)
	{
		return engEvalString(eg, string);
	}

	mxArray *MatEngine::GetVariable(const char *name)
	{
		return engGetVariable(eg, name);
	}

	int MatEngine::PutVariable(const char *var_name, const mxArray *ap)
	{
		return engPutVariable(eg, var_name, ap);
	}

	int MatEngine::OutputBuffer(char* buffer, int buflen)
	{
		return engOutputBuffer(eg, buffer, buflen);
	}
}
