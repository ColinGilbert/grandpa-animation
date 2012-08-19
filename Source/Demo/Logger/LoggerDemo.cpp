#include <tchar.h>
#include "Grandpa.h"
#include "ConsoleLogger.h"
#include <conio.h>
#include "Performance.h"

int _tmain(int argc, _TCHAR* argv[])
{
	ConsoleLogger logger;

	grp::initialize(&logger);

#ifdef _PERFORMANCE
	PerfManager::createTheOne();
	grp::setProfiler(PerfManager::getTheOne());
#endif

	grp::IResource* modelRes = grp::grabResource(L"Test/warrior.gmd", grp::RES_TYPE_MODEL);
	if (modelRes == NULL || modelRes->getResourceState() != grp::RES_STATE_COMPLETE)
	{
		goto end;
	}

	grp::IModel* model = grp::createModel(modelRes);
	grp::dropResource(modelRes);
	if (model == NULL)
	{
		goto end;
	}

	model->playAnimation(L"stand", grp::ANIMATION_LOOP);

	//model->update(1.0, 0.1f);

	grp::destroyModel(model);

end:
	grp::destroy();

	wprintf(L"Press any key.");
	_getch();

	return 0;
}
