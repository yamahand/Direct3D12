#include "CommandList.h"

CommandList::CommandList() {

}

CommandList::~CommandList() {
	Term();
}

bool CommandList::Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t count)
{
	return false;
}

void CommandList::Term()
{
}

ID3D12GraphicsCommandList* CommandList::Reset()
{
	return nullptr;
}

