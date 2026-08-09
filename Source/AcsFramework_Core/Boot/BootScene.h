#pragma once

#include <acs.h>
using namespace acs;

class ABootScene : public AScene
{
public:
	/** シーンが top に積まれたときに 1 回呼ばれる。 */
	void OnEnter() noexcept override;
};
